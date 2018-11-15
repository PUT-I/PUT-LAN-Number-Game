// Clients.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Client.hpp"

int main() {
	ClientTCP client1;
	ClientTCP client2;
	system("pause");

	if (client1.connectServer()) { std::cout << "Client1 Connected!\n"; }
	else { std::cout << "Client1 Not Connected!\n"; }

	if (client2.connectServer()) { std::cout << "Client2 Connected!\n"; }
	else { std::cout << "Client2 Not Connected!\n"; }

	BinProtocol bProt("010", "111", "00000");
	bProt.display();
	client1.sendBinProtocol(bProt);

	system("pause");

}