// Clients.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Client.hpp"

int main() {
	ClientTCP client;
	system("pause");

	if (client.connectServer()) { std::cout << "Connected!\n"; }
	else { std::cout << "Not Connected!\n"; }
	system("pause");
}