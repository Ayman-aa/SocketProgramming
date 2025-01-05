#include "Client.hpp"

Client::Client(const std::string &server, const std::string &port): server(server), port(port), clientSocket(0){}

Client::~Client()
{
	if (clientSocket > 0)
		close(clientSocket);
}

void* get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)sa;
		return &(ipv4->sin_addr);                 
	} else {
		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)sa;
		return &(ipv6->sin6_addr);
			                   
	}
	
}

bool Client::connectToServer()
{
	struct addrinfo hints, *serverInfo, *res;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(server.c_str(), port.c_str(), &hints, &serverInfo)) != 0)
	{
		std::cerr << "getaddrinfo error: " << gai_strerror(rv)<<std::endl;
		return false;
	}

	for (res = serverInfo; res != NULL; res->ai_next)
	{
		if((clientSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
		{
			std::cerr << "client socket creation failed" << std::endl;
			continue;
		}
		if(connect(clientSocket, res->ai_addr, res->ai_addrlen) == -1)
		{
			close(clientSocket);
			std::cerr << "client connection failed" << std::endl;
			continue;
		}
		break;
	}
	freeaddrinfo(serverInfo);

	if(res == NULL)
	{
		std::cerr << "client failed to connect" << std::endl;
		return false;
	}

	char s[INET6_ADDRSTRLEN];
	inet_ntop(res->ai_family, get_in_addr((struct sockaddr*)res->ai_addr), s, sizeof s);
	std::cout << "connected to " << s << std::endl;

	return true;
}

void Client::communicate()
{
	char buffer[BUFFER_SIZE];

	memset(buffer, 0, BUFFER_SIZE);
	recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
	std::cout << "Server: " << std::endl;

	while(true)
	{
		std::string msg;
		std::cout << "Enter a message or quit to exit: ";
		std::getline(std::cin,msg);

		if(msg == "quit")
			break;

		send(clientSocket, msg.c_str(), msg.length(), 0);

		memset(buffer, 0, BUFFER_SIZE);
		int readBytes = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);

		if(readBytes <= 0)
		{
			std::cout << "Server disconnected" << std::endl;
			break;
		}
		std::cout << "Server response: " << buffer << std::endl;
	}
}
