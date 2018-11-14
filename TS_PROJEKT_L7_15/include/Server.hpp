#pragma once
#include <WS2tcpip.h>
#include <winsock.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <chrono>

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable:4996) 

class ServerTCP {
	WSADATA wsaData;
	SOCKET serverSocket;
	sockaddr_in serverAddress;
	std::unordered_map<unsigned int, SOCKET>clients;

public:
	ServerTCP() {
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) { std::cout << "Initialization error.\n"; }

		serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (serverSocket == INVALID_SOCKET) {
			std::cout << "Error creating socket: " << WSAGetLastError() << "\n";
			WSACleanup();
		}

		this->addressInit("127.0.0.1", 7000);

		if (!socketBind()) { std::cout << "bind() failed.\n"; }
	}

	bool socketBind() {
		if (bind(serverSocket, reinterpret_cast<SOCKADDR*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR) {
			closesocket(serverSocket);
			return false;
		}
		return true;
	}

	bool connectClient() {
		const SOCKET clientSocket = accept(this->serverSocket, nullptr, nullptr);

		if (clientSocket == SOCKET_ERROR) {return false; }

		for (unsigned int i = 0; i < ~unsigned int(0); i++) {
			if (clients.find(i) == clients.end()) { clients[i] = clientSocket; return true; }
		}
		return false;
	}

	void addressInit(const std::string& ip, const unsigned int& port) {
		memset(&serverAddress, 0, sizeof(serverAddress));
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_addr.s_addr = inet_addr(ip.c_str());
		serverAddress.sin_port = htons(port);
	}
};
