
#include <WS2tcpip.h>
#include <winsock.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <chrono>

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable:4996) 

class ClientTCP {
	WSADATA wsaData;
	SOCKET clientSocket;
	sockaddr_in clientAddress;

public:
	ClientTCP() {
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) { std::cout << "Initialization error.\n"; }

		clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (clientSocket == INVALID_SOCKET) {
			std::cout << "Error creating socket: " << WSAGetLastError() << "\n";
			WSACleanup();
		}

		this->addressInit("127.0.0.1", 7000);
	}

	bool connectServer() {
		if (connect(clientSocket, reinterpret_cast<SOCKADDR*>(&clientAddress), sizeof(clientAddress)) != SOCKET_ERROR) { return true; }
		else { return false; }
	}

	void addressInit(const std::string& ip, const unsigned int& port) {
		memset(&clientAddress, 0, sizeof(clientAddress));
		clientAddress.sin_family = AF_INET;
		clientAddress.sin_addr.s_addr = inet_addr(ip.c_str());
		clientAddress.sin_port = htons(port);
	}
};
