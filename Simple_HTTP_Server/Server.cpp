#include "Server.hpp"

Server::Server(const std::string &ip, const std::string &port)
{
	createSocket(ip, port);
	if(listen(serverSocket, SOMAXCONN) < 0)
		throw std::runtime_error("Failed to listen on socket");
}

Server::~Server()
{
	if(serverSocket != -1)
		close(serverSocket);
}

void Server::createSocket(const std::string &ip, const std::string &port)
{
	struct addrinfo hints, *serverInfo;
	memset(&hints, 0 ,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	const char *host = (ip.empty() || ip == "0.0.0.0") ? NULL : ip.c_str();

	int status = getaddrinfo(host, port.c_str(), &hints, &serverInfo);
	if(status != 0)
		throw std::runtime_error("Failed to get address info: " + std::string(gai_strerror(status)));

	struct addrinfo *p;
	for(p = serverInfo; p != NULL; p = p->ai_next)
	{
		serverSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(serverSocket == -1)
			continue;
		try
		{
			setSocketOptions();
			bindSocket(p);
			break;
		}
		catch(const std::exception &e)
		{
			close(serverSocket);
			continue;
		}
	}
	
	freeaddrinfo(serverInfo);
	
	if(p == NULL)
		throw std::runtime_error("Failed to bind to address" + ip + ":" + port);
}

void Server::setSocketOptions()
{
	int opt = 1;
	if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("Failed to set socket options");

	int flags = fcntl(serverSocket, F_GETFL, 0);
	if(flags == -1)
		throw std::runtime_error("Failed to set flags for the socket");

	if(fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error("Failed to set non-blocking socket");
}

void Server::bindSocket(struct addrinfo *serverInfo)
{
	if(bind(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen) < 0)
		throw std::runtime_error("Failed to bind socket");
}

int Server::getServerSocket() const { return serverSocket; }
