
#include "Client.hpp"

int main() {
	ClientTCP client("127.0.0.1", 7000);
	std::cerr << "(err)" << GetCurrentTimeTm() << " : " << "---- Client ----\n";
	std::cout << GetCurrentTimeTm() << " : " << "Connecting server.\n";

	bool connected = false;
	while (!connected) {
		connected = client.connectServer();

		if (connected) {
			std::cout << GetCurrentTimeTm() << " : " << "Server connected.\n\n";

			client.startGame();
		}
		else {
			std::cout << '\n' << GetCurrentTimeTm() << " : " << "Client not connected.\n";

			std::cout << GetCurrentTimeTm() << " : " << "Do you want to stop? (Y)\n";
			std::string input;
			std::cin >> input;
			if (input == "Y" || input == "y") { break; }
		}
	}

	std::cout << '\n' << GetCurrentTimeTm() << " : " << "Program end.\n";
}