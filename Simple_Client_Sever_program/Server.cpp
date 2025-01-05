#include "Server.hpp"

Server::Server(const std::string &ip, const std::string &port) : ip(ip), port(port), serverFd(0){}

Server::~Server()
{
	if (serverFd > 0)
		close(serverFd);
}

bool Server::init()
{
	struct addrinfo hints, *serverInfo, *results;
	int rv;
	int opt = 1;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if((rv = getaddrinfo(NULL, port.c_str(), &hints, &serverInfo)) != 0)
	{
		std::cerr << "getaddrinfo error: " << gai_strerror(rv) << std::endl;
		return false;
	}

	for(results = serverInfo; results != NULL; results = results->ai_next)
	{
		if((serverFd = socket(results->ai_family, results->ai_socktype, results->ai_protocol)) == -1)
		{
			std::cerr << "Socket creation failed" << std::endl;
			continue;
		}

		if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1)
		{
			std::cerr << "Setsockopt failed" << std::endl;
			close(serverFd);
			freeaddrinfo(serverInfo);
			return false;
		}

		if(bind(serverFd, results->ai_addr, results->ai_addrlen) == -1)
		{
			close(serverFd);
			std::cerr << "Bind failed" << std::endl;
			continue;
		}
		break;
	}
	freeaddrinfo(serverInfo);
	if(results == NULL)
	{
		std::cerr << "Failed to bind" << std::endl;
		return false;
	}
	return true;
}

void* get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        // IPv4
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)sa;
        return &(ipv4->sin_addr);
    } else {
        // IPv6
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)sa;
        return &(ipv6->sin6_addr);
    }
}

void Server::start()
{
	if(listen(serverFd, 5) == -1)
	{
		std::cerr << "Listen failed" << std::endl;
		return;
	}

	std::cout << "Server listening on port: " << port << std::endl;

	while (true)
	{
		struct sockaddr_storage client_addr;
		socklen_t addr_size = sizeof client_addr;
		char clientIp[INET6_ADDRSTRLEN];

		int clientSocket = accept(serverFd, (struct sockaddr*)&client_addr, &addr_size);
		if (clientSocket == -1)
		{
			std::cerr << "Accept failed" << std::endl;
			continue;
		}

		void *addr = get_in_addr((struct sockaddr *)&client_addr);
		inet_ntop(client_addr.ss_family, addr, clientIp, sizeof clientIp);
		std::cout << "Got connection from :" << clientIp << std::endl;

		handleClient(clientSocket);
	}
}

void Server::handleClient(int clientSocket)
{
	char buffer[BUFFER_SIZE];

	std::string welcome = "Welcome to server ;)";
	send(clientSocket, welcome.c_str(), welcome.length(), 0);

	while(true)
	{
		memset(buffer, 0, BUFFER_SIZE);
		int readBytes = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);

		if (readBytes <= 0)
		{
			std::cout << "Client disconnected" << std::endl;
			break;
		}

		std::cout << "Recieved :" << buffer << std::endl;

		std::string response = "Server recieved : ";
		response += buffer;
		send(clientSocket, response.c_str(), response.length(), 0);
	}
	close(clientSocket);
}

