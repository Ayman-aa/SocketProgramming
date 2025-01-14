#include "ClientHandler.hpp"
#include <iostream>
#include <csignal>
#include <ctime>
#include <iomanip>
#include <thread>
#include <atomic>

std::atomic<bool> running(true);

void signalHandler(int signum)
{
	std::cout << "\nReceived signal " << signum << ". Shutting down server..." << std::endl;
	running = false;
}

std::string getCurrentTimestamp()
{
	auto now = std::chrono::system_clock::now();
	auto now_c = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
	return ss.str();
}

void printServerInfo(const std::string& ip, const std::string& port, const std::string& rootDir)
{
	std::cout << "\n=== HTTP Server Started ===" << std::endl;
	std::cout << "Time: " << getCurrentTimestamp() << std::endl;
	std::cout << "IP: " << (ip.empty() ? "0.0.0.0 (all interfaces)" : ip) << std::endl;
	std::cout << "Port: " << port << std::endl;
	std::cout << "Root Directory: " << rootDir << std::endl;
	std::cout << "Server is ready to accept connections" << std::endl;
	std::cout << "Use telnet " << (ip.empty() ? "localhost" : ip) << " " << port 
		<< " or wget http://" << (ip.empty() ? "localhost" : ip) << ":" << port << "/file.html" << std::endl;
	std::cout << "Or our browser in the same ip::port for cool visual ;) " << std::endl;
	std::cout << "Press Ctrl+C to shutdown the server" << std::endl;
	std::cout << "========================\n" << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc < 3 || argc > 4)
	{
		std::cerr << "Usage: " << argv[0] << " <port> <root_directory> [ip_address]" << std::endl;
		std::cerr << "Examples:" << std::endl;
		std::cerr << "  " << argv[0] << " 8080 ./www              # Listen on all interfaces" << std::endl;
		std::cerr << "  " << argv[0] << " 8080 ./www 127.0.0.1    # Listen only on localhost" << std::endl;
		std::cerr << "  " << argv[0] << " 8080 ./www 192.168.1.10 # Listen on specific interface" << std::endl;
		return 1;
	}

	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);

	std::string port(argv[1]);
	std::string rootDir(argv[2]);
	std::string ip = (argc == 4) ? argv[3] : "";

	try
	{
		ClientHandler handler(ip, port, rootDir);
		printServerInfo(ip, port, rootDir);

		while (running)
		{
			try
			{
				handler.run();
			}
			catch (const std::exception& e)
			{
				std::cerr << getCurrentTimestamp() << " - Error: " << e.what() << std::endl;
				std::cerr << "Server attempting to continue..." << std::endl;
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
		}
	std::cout << "\nServer shutdown completed." << std::endl;
    }
	catch (const std::exception& e)
	{
		std::cerr << "Fatal Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}