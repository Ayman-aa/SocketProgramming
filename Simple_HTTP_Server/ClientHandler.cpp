#include "ClientHandler.hpp"

ClientHandler::ClientHandler(const std::string &ip, const std::string &port, const std::string &rootDir):server(ip,port), rootDir(rootDir)
{
	initEpoll();
	initMimeTypes();
}

void ClientHandler::initMimeTypes()
{
	mimeTypes = {
		{".html", "text/html"},
		{".css", "text/css"},
		{".js", "application/javascript"},
		{".jpg", "image/jpeg"},
		{".png", "image/png"},
		{".txt", "text/plain"}
	};
}

void ClientHandler::initEpoll()
{
	epollFd = epoll_create1(0);
	if(epollFd == -1)
		throw std::runtime_error("Failed to create epoll instance");

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = server.getServerSocket();

	if(epoll_ctl(epollFd, EPOLL_CTL_ADD, server.getServerSocket(), &event) == -1)
		throw std::runtime_error("Failed to add server socket to epoll");
}

void ClientHandler::run()
{
	struct epoll_event events[MAX_EVENTS];

	while(true)
	{
		int ndfs = epoll_wait(epollFd, events, MAX_EVENTS, -1);
		if(ndfs == -1)
			throw std::runtime_error("Epoll wait failed");

		for (int i = 0; i < ndfs; i++)
		{
			if(events[i].data.fd == server.getServerSocket())
				handleNewConnection();
			else
				handleClientRequest(events[i].data.fd);
		}
	}
}

void ClientHandler::handleNewConnection()
{
	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);

	int clientSocket = accept(server.getServerSocket(), (struct sockaddr*)&clientAddr, &clientLen);
	if(clientSocket == -1)
		return;

	int flags = fcntl(clientSocket, F_GETFL, 0);
	if (flags != -1)
		fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK);

	struct epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = clientSocket;

	if(epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &event) == -1)
	{
		close(clientSocket);
		return;
	}
}

void ClientHandler::handleClientRequest(int clientSocket)
{
	char buffer[BUFFER_SIZE];
	ssize_t readBytes = read(clientSocket, buffer, BUFFER_SIZE - 1);

	if(readBytes <= 0)
	{
		close(clientSocket);
		epoll_ctl(epollFd, EPOLL_CTL_DEL, clientSocket, NULL);
		return;
	}

	buffer[readBytes] = '\0';
	std::string request(buffer);

	std::istringstream iss(request);
	std::string method, path, protocol;
	iss >> method >> path >> protocol;

	if(method != "GET")
	{
		sendErrorPage(clientSocket, 405);
		return;
	}
	
	if(path[0] == '/')
		path = path.substr(1);

	size_t queryPos = path.find('?');
	if(queryPos != std::string::npos)
		path = path.substr(0, queryPos);
	
	if(path.empty())
		path = "index.html";

	std::string fullPath = rootDir + "/" + path;
	if(std::filesystem::exists(fullPath))
		sendResponse(clientSocket, fullPath);
	else
		sendErrorPage(clientSocket, 404);
}

std::string ClientHandler::getMimeType(const std::string &filePath)
{
	size_t dotPos = filePath.find_last_of('.');
	if(dotPos != std::string::npos)
	{
		std::string ext = filePath.substr(dotPos);
		std::map<std::string, std::string>::iterator it = mimeTypes.find(ext);
		if(it != mimeTypes.end())
			return it->second;
	}
	return "application/octet-stream";
}

void ClientHandler::sendResponse(int clientSocket, const std::string& filePath)
{
	std::ifstream file(filePath, std::ios::binary);
	if(!file)
	{
		sendErrorPage(clientSocket, 500);
		return;
	}

	file.seekg(0, std::ios::end);
	size_t fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	std::stringstream headers;
	headers << "HTTP/1.1 200 OK\r\n"
		<< "Content-Type: " << getMimeType(filePath) << "\r\n"
		<< "Content-Length: " << fileSize << "\r\n"
		<< "Connection: close\r\n"
		<< "\r\n";
	
	std::string headerStr = headers.str();
	send(clientSocket, headerStr.c_str(), headerStr.length(), 0);

	char buffer[BUFFER_SIZE];
	while(file.read(buffer, sizeof(buffer)))
		send(clientSocket, buffer, file.gcount(), 0);
	if(file.gcount() > 0)
		send(clientSocket, buffer, file.gcount(), 0);
	
	close(clientSocket);
	epoll_ctl(epollFd, EPOLL_CTL_DEL, clientSocket, NULL);
}

void ClientHandler::sendErrorPage(int clientSocket, int errorCode)
{
	std::string message;
	switch(errorCode)
	{
		case 404:
			message = "404 Not Found";
			break;
		case 405:
			message = "405 Method Not Allowed";
			break;
		case 500:
			message = "500 Internal Server Error";
			break;
		default:
			message = "Unknown Error";
	}

	std::string content = "<!DOCTYPE html><html><body><h1>" + message + "</h1></body></html>";
	std::stringstream response;
	response << "HTTP/1.1 " << errorCode << " " << message << "\r\n"
		<< "Content-Type: text/html\r\n"
		<< "Content-Length: " << content.length() << "\r\n"
		<< "Connection: close\r\n"
		<< "\r\n"
		<< content;
	std::string responseStr = response.str();
	send(clientSocket, responseStr.c_str(), responseStr.length(), 0);

	close(clientSocket);
	epoll_ctl(epollFd, EPOLL_CTL_DEL, clientSocket, nullptr);
}
