// Clients.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Client.hpp"

int main() {
	ClientTCP client;
	system("pause");

	if (client.connectServer())
	{
		std::cout << "Client Connected!\n";

		BinProtocol bProt("010", "111", "00000");
		bProt.display();
		client.sendBinProtocol(bProt);

	}
	else { std::cout << "Client1 Not Connected!\n"; }

	system("pause");
}