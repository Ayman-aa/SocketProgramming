// main.cpp
#include <iostream>
#include <signal.h>
#include "SMTPServer.hpp"

SMTPServer* globalServer = nullptr;

void signalHandler(int signum) {
    if (globalServer) {
        std::cout << "\nShutting down server..." << std::endl;
        globalServer->stop();
    }
    exit(signum);
}

int main() {
    try {
        SMTPServer server("2525");
        globalServer = &server;

        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);

        server.createMailbox("test@local.com", "test123");
        
        std::cout << "Server started on port 2525" << std::endl;
        std::cout << "Test account: test@local.com (password: test123)" << std::endl;
        
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}