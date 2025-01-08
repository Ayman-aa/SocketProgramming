#include "SMTPServer.hpp"

void* get_in_addr(struct sockaddr* sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

SMTPServer::SMTPServer(const char *port): port(port), running(false), serverInfo(NULL)
{
	setupServerSocket();
}


void SMTPServer::setupServerSocket()
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int status = getaddrinfo(NULL, port, &hints, &serverInfo);
	if(status != 0)
		throw std::runtime_error("getaddrinfo error: " + std::string(gai_strerror(status)));

	struct addrinfo *res;
	for (res = serverInfo; res != NULL; res = res->ai_next)
	{
		serverSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if(serverSocket == -1)
			continue;

		int opt = 1;
		if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1)
		{
			close(serverSocket);
			continue;
		}
		if(bind(serverSocket, res->ai_addr, res->ai_addrlen) == -1)
		{
			close(serverSocket);
			continue;
		}
		break;
	}
	if(res = NULL)
	{
		freeaddrinfo(serverInfo);
		throw std::runtime_error("Failed to bind socket: " + std::string(strerror(errno)));
	}
}

SMTPServer::~SMTPServer()
{
    stop();
    if (serverInfo) {
        freeaddrinfo(serverInfo);
    }
    close(serverSocket);
}

void SMTPServer::stop()
{
	if(!running)
		return;
	std::cout << "Shutting down the server..." << std::endl;
	running = false;

	if(serverSocket != -1)
	{
		shutdown(serverSocket, SHUT_RDWR);
		close(serverSocket);
		serverSocket = -1;
	}

	if(queueProcessor.joinable())
		queueProcessor.join();

	cleanupThreads();

	if(serverInfo)
	{
		freeaddrinfo(serverInfo);
		serverInfo = NULL;
	}
	std::cout << "Server shutdown complete" << std::endl;
}

void SMTPServer::cleanupThreads()
{
	std::lock_guard<std::mutex> lock(clientThreadsMutex);
	for(std::thread &thread : clientThreads)
	{
		if (thread.joinable())
			thread.join();
	}
	clientThreads.clear();
}

void SMTPServer::processQueueLoop()
{
	while(running)
	{
		MailMessage msg("", "", "", "");
		if(mailQueue.dequeue(msg))
			deliverMail(msg);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void SMTPServer::deliverMail(const MailMessage &msg)
{
	if (isSpam(msg))
	{
		std::cout << "Message filtered as spam" << std::endl;
		return;
	}

	std::lock_guard<std::mutex> lock(mailboxMutex);
	std::map<std::string, Mailbox>::iterator it = mailboxes.find(msg.getTo());
	if(it != mailboxes.end())
			it->second.addMessage(msg);
}

bool SMTPServer::isSpam(const MailMessage &msg)
{
	std::vector<std::string> spamKeywords = {"viagra", "lottery", "winner", "million dollars"};
	
	for(const std::string &keyword : spamKeywords)
	{
		if(msg.getContent().find(keyword) != std::string::npos)
			return true;
	}
	return false;
}

void SMTPServer::start()
{
	running = true;

	queueProcessor = std::thread(&SMTPServer::processQueueLoop, this);

	if(listen(serverSocket, SOMAXCONN) == -1)
		throw std::runtime_error("Listen failed: " + std::string(strerror(errno)));

	std::cout << "Server is listening on port " << port << std::endl;

	while(running)
	{
		struct sockaddr_storage clientAddr;
		socklen_t clientLen = sizeof(clientAddr);

		int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
		
		if(!running)
			break;
		
		if (clientSocket == -1)
		{
			std::cout << "Accept failed: " << strerror(errno) << std::endl;
			continue;
		}

		char clientIp[INET6_ADDRSTRLEN];
		inet_ntop(clientAddr.ss_family, get_in_addr((struct sockaddr *)&clientAddr), clientIp, sizeof(clientIp));

		std::cout << "Got connection from: " << clientIp << std::endl;

		{
			std::lock_guard<std::mutex> lock(clientThreadsMutex);
			clientThreads.push_back(std::thread(&SMTPServer::handleClient, this, clientSocket));
		}
	}
}

void SMTPServer::checkMailbox(int clientSocket, const std::string &username) {
	std::lock_guard<std::mutex> lock(mailboxMutex);
	auto it = mailboxes.find(username);
	if (it == mailboxes.end())
	{
		std::string response = "550 Mailbox not found\r\n";
		send(clientSocket, response.c_str(), response.length(), 0);
		return;
	}

	std::vector<MailMessage> messages = it->second.getMessages();
	std::string response = "250 Mailbox contents:\r\n";
	for (const auto &msg : messages) {
		response += "From: " + msg.getFrom() + "\r\n";
		response += "To: " + msg.getTo() + "\r\n";
		response += "Subject: " + msg.getSubject() + "\r\n";
		response += "Content: " + msg.getContent() + "\r\n";
		response += "Timestamp: " + std::to_string(msg.getTimestamp()) + "\r\n";
		response += "\r\n";
	}
	send(clientSocket, response.c_str(), response.length(), 0);
}

void SMTPServer::handleClient(int clientSocket)
{
	std::string response = "200 Server ready\r\n";
	response += "Available commands:\r\n";
	response += "AUTH <username> <password>\r\n";
	response += "MAIL FROM:<address>\r\n";
	response += "RCPT TO:<address>\r\n";
	response += "DATA\r\n";
	response += "QUIT\r\n";
	response += "CHECKMAIL\r\n";
	send(clientSocket, response.c_str(), response.length(), 0);

	char buffer[1024];
	bool clientConnected = true;

	while(running && clientConnected)
	{
		memset(buffer, 0, sizeof(buffer));
		ssize_t readBytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

		if(readBytes <= 0)
			break;

		std::string command(buffer);
		clientConnected = processCommand(clientSocket, command);
	}
	std::cout << "Client "<< clientSocket << " disconnected" << std::endl;
	close(clientSocket);

	{
		std::lock_guard<std::mutex> lock(clientThreadsMutex);
		std::vector<std::thread>::iterator it = std::find_if(
				clientThreads.begin(),
				clientThreads.end(),
				[](std::thread& t) {return !t.joinable();}
				);
		if(it != clientThreads.end())
			clientThreads.erase(it);
	}
}

bool SMTPServer::processCommand(int clientSocket, const std::string &command)
{
	std::string response;

	std::istringstream iss(command);
	std::string cmd;
	iss >> cmd;

	static std::string currentMailFrom;
	static std::string currentRcptTo;
	static bool isAuthenticated = false;
	static bool inDataMode = false;
	static std::string messageData;

	if (inDataMode)
	{
		if (command == ".\r\n" || command == ".\n" || command == ".")
		{
			inDataMode = false;

			MailMessage msg(currentMailFrom, currentRcptTo, "Subject", messageData);
			mailQueue.enqueue(msg);
			response = "250 Message accepted for delivery\r\n";
			messageData.clear();
			return true;
        }
		else
		{
			messageData += command + "\r\n";
			return true;
		}
	}
	if(cmd == "AUTH")
	{
		std::string username, password;
		iss >> username >> password;

		if(authenticateUser(username, password))
		{
			isAuthenticated = true;
			response = "235  Authentication successful\r\n";
		}
		else
			response = "535 Authentication failed\r\n";
	}
	else if (cmd == "QUIT")
	{
		response = "221 Goodbye\r\n";
		send(clientSocket, response.c_str(), response.length(), 0);
		return false;
	}
	else if(!isAuthenticated && cmd != "HELO" && cmd != "EHLO")
		response = "530 Authentication required\r\n";
	else if(cmd == "HELO" || cmd == "EHLO")
	{
		std::string domain;
		iss >> domain;
		response += "250 Server ready\r\n";
		response += "250-AUTH PLAIN LOGIN\r\n";
		response += "250-MAIL FROM\r\n";
		response += "250 RCPT TO\r\n";
	}
	else if (cmd == "MAIL")
	{
		std::string from;
		iss >> from;
		if(from.substr(0,5) == "FROM:")
		{
			currentMailFrom = from.substr(5);
			response = "250 Sender OK\r\n";
		}
		else
			response = "501 Syntax error in parameters\r\n";
	}
	else if (cmd == "RCPT")
	{
		std::string to;
		iss >> to;

		if(to.substr(0,3) == "TO:")
		{
			currentRcptTo = to.substr(3);
			response = "250 Recipient OK\r\n";
		}
		else
			response = "501 Syntax error in parameters\r\n";
	}
	else if (cmd == "DATA")
	{
		if(currentMailFrom.empty() || currentRcptTo.empty())
			response = "503 Need MAIL FROM and RCPT TO\r\n";
		else
		{
			response = "354 Start mail input; end with <CRLF>.<CRLF>\r\n";
			inDataMode = true;
			messageData.clear();
		}
	}
	else if(cmd == "CHECKMAIL")
	{
		checkMailbox(clientSocket, currentRcptTo);
		return true;
	}
	else
		response = "500 Unknown command\r\n";
	
	send(clientSocket, response.c_str(), response.length(), 0);
	return true;
}

bool SMTPServer::authenticateUser(const std::string &username, const std::string &password)
{
	std::lock_guard<std::mutex> lock(mailboxMutex);
	std::map<std::string, Mailbox>::iterator it = mailboxes.find(username);
	if(it == mailboxes.end())
		return false;
	return it->second.authenticate(password);
}

bool SMTPServer::createMailbox(const std::string& username, const std::string& password) {
	std::lock_guard<std::mutex> lock(mailboxMutex);
	auto result = mailboxes.try_emplace(username, username, password);
	return result.second;
}
