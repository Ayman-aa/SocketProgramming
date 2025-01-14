#pragma once
#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

class Server
{
	public:
		Server(const std::string &ip, const std::string &port);
		~Server();
		int getServerSocket() const;
	private:
		int serverSocket;
		void createSocket(const std::string &ip, const std::string &port);
		void setSocketOptions();
		void bindSocket(struct addrinfo *serverInfo);
};
