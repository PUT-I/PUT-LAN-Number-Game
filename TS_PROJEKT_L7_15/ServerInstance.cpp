
#include "Protocol.hpp"
#include "Server.hpp"

int main() {
	fflush(stdout);

	const unsigned int clientNumber = 2;
	std::cout << "Client accepting.\n";

	ServerTCP server;
	for (unsigned int i = 0; i < clientNumber; i++) {
		std::cout << "\nWaiting for client " << i + 1 << ".\n";
		if (server.acceptClient()) { std::cout << "Client " << i + 1 << " connected.\n"; }
	}

	server.startGame();

	std::cout << "\nProgram end.\n";
}
