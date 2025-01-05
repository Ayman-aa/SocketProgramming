#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <netdb.h>

#define BUFFER_SIZE 1024

void* get_in_addr(struct sockaddr *sa);

class Server
{
	private:
		int serverFd;
		std::string port;
		std::string ip;
		
	public:
		Server(const std::string &ip, const std::string &port);
		~Server();

		bool init();
		void start();
		void handleClient(int clientFd);
};

#endif
