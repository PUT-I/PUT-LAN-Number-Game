
#include "Client.hpp"

int main() {
	ClientTCP client("127.0.0.1", 7000);
	sync_cout << GetCurrentTimeTm() << " : " << "---- Client ----\n";
	sync_cout << GetCurrentTimeTm() << " : " << "Connecting server.\n";

	bool connected = false;
	while (!connected) {
		connected = client.connectServer();

		if (connected) {
			sync_cout << GetCurrentTimeTm() << " : " << "Server connected.\n\n";

			client.startGame();
		}
		else {
			sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Client not connected.\n";

			sync_cout << GetCurrentTimeTm() << " : " << "Do you want to stop? (Y): ";
			std::string input;
			std::cin >> input;
			if (input == "Y" || input == "y") { break; }
		}
	}

	sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "Program end.\n";
}