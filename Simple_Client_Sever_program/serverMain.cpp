#include "Server.hpp"

int main()
{
	Server server("127.0.0.1","8080");

	if(!server.init())
		return 1;

	server.start();
	return 0;
}
