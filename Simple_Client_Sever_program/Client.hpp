#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#define BUFFER_SIZE 1024

void* get_in_addr(struct sockaddr *sa);

class Client
{
	private:
		int clientSocket;
		std::string server;
		std::string port;
	
	public:
		Client(const std::string &server, const std::string &port);
		~Client();

		bool connectToServer();
		void communicate();
};

#endif
