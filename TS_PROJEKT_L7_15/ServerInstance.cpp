#include "Server.hpp"

int main() {
	const unsigned int clientNumber = 2;
	std::cerr << "(err)" << GetCurrentTimeTm() << " : " << "---- Server ----\n";
	std::cout << GetCurrentTimeTm() << " : " << "Client accepting.\n";

	ServerTCP server("127.0.0.1", 7000);
	for (unsigned int i = 0; i < clientNumber; i++) {
		std::cout << '\n' << GetCurrentTimeTm() << " : " << "Waiting for client " << i + 1 << ".\n";
		if (server.acceptClient()) { std::cout << GetCurrentTimeTm() << " : " << "Client " << i + 1 << " connected.\n"; }
	}

	server.startGame();
	std::cout << '\n' << GetCurrentTimeTm() << " : " << "Program end.\n";
}
