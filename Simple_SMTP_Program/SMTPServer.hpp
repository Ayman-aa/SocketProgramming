#pragma once
#include <map>
#include <thread>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <errno.h>
#include <algorithm>
#include <string.h>
#include "Mailbox.hpp"
#include "MailQueue.hpp"

class SMTPServer
{
	public:
		SMTPServer(const char *port);
		~SMTPServer();

		void start();
		void stop();
		bool createMailbox(const std::string &username, const std::string &password);
		bool authenticateUser(const std::string &username, const std::string &password);

	private:
		int serverSocket;
		const char *port;
		std::map<std::string, Mailbox> mailboxes;
		MailQueue mailQueue;
		bool running;
		std::thread queueProcessor;
		std::mutex mailboxMutex;
		struct addrinfo *serverInfo;
		std::vector<std::thread> clientThreads;
		std::mutex clientThreadsMutex;
		
		void cleanupThreads();
		void setupServerSocket();
		void processQueueLoop();
		void handleClient(int clientSocket);
		bool processCommand(int clientSocket, const std::string &command);
		bool isSpam(const MailMessage &msg);
		void deliverMail(const MailMessage &msg);
		void checkMailbox(int clientSocket, const std::string &username);
};
