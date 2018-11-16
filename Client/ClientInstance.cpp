
#include "Client.hpp"

int main() {
	ClientTCP client("127.0.0.1", 7000);
	std::cout << GetCurrentTimeAndDate() << " : " << "Server connection.\n";

	bool connected = false;
	while (!connected) {
		connected = client.connectServer();

		if (connected) {
			std::cout << '\n' << GetCurrentTimeAndDate() << " : " << "Client connected.\n";

			client.startGame();
		}
		else {
			std::cout << '\n' << GetCurrentTimeAndDate() << " : " << "Client not connected.\n";

			std::cout << GetCurrentTimeAndDate() << " : " << "Do you want to stop? (Y)\n";
			std::string input;
			std::cin >> input;
			if (input == "Y" || input == "y") { break; }
		}
	}

	std::cout << '\n' << GetCurrentTimeAndDate() << " : " << "Program end.\n";
}