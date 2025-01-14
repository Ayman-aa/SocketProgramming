#pragma once
#include <sys/epoll.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <stdexcept>
#include <cstring>
#include <map>
#include "Server.hpp"

class ClientHandler
{
	public:
		ClientHandler(const std::string &ip, const std::string &port, const std::string &rootDir);
		void run();

	private:
		static const int MAX_EVENTS = 10;
		static const int BUFFER_SIZE = 4096;

		Server server;
		int epollFd;
		std::string rootDir;
		std::map<std::string, std::string> mimeTypes;

		void initEpoll();
		void handleNewConnection();
		void handleClientRequest(int clientSocket);
		void sendResponse(int clientSocket, const std::string &filePath);
		void sendErrorPage(int clientSocket, int errorCode);
		std::string getMimeType(const std::string &filePath);
		void initMimeTypes();
};
