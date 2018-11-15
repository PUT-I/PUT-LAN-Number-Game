
#include "Protocol.hpp"
#include "Server.hpp"

int main() {
	//BinProtocol protocol;
	//protocol.setOperation("010");
	//protocol.setAnswer("111");
	//protocol.setId("00100");
	//protocol.display();
	//std::cout << protocol.to_string() << "\n\n";

	ServerTCP server;

	unsigned int clientNumber;
	std::cout << "Enter the desired number of clients: ";
	std::cin >> clientNumber;

	for (unsigned int i = 0; i < clientNumber; i++) {
		if (server.connectClient()) { std::cout << "Client " << i + 1 << " connected\n"; }

	}

	BinProtocol binProt = server.receiveBinProtocol();
	std::cout << "Received binary protocol: ";
	binProt.display();
	std::cout << '\n';

	system("pause");
}
