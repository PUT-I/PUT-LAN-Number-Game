// Clients.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Client.hpp"

void setProtocol(BinProtocol& protocol) {
	std::string operation;
	std::string answer;
	std::string id;

	std::cout << "Enter protocol operation (3-bit): ";
	std::cin >> operation;

	std::cout << "Enter protocol answer (3-bit): ";
	std::cin >> answer;

	protocol.set(operation, answer, "00000", NULL);
}

int main() {
	fflush(stdout);
	ClientTCP client;
	std::cout << "Server connection.\n";

	bool connected = false;
	while (!connected) {
		connected = client.connectServer();

		if (connected) {
			std::cout << "\nClient connected.\n";

			client.startGame();
		}
		else {
			std::cout << "\nClient not connected.\n";

			std::cout << "Do you want to stop? (Y)\n";
			std::string input;
			std::cin >> input;
			if (input == "Y" || input == "y") { break; }
		}
	}

	std::cout << "\nProgram end.\n";
}