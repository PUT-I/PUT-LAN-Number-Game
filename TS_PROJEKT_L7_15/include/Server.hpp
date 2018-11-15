#pragma once
#include <Protocol.hpp>
#include <WS2tcpip.h>
#include <winsock.h>
#include <iostream>
#include <string>
#include <unordered_map>

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable:4996) 

class ServerTCP {
private:
	static const unsigned int bufLen = 2;

	WSADATA wsaData;
	SOCKET serverSocket;
	sockaddr_in serverAddress;
	std::unordered_map<unsigned int, SOCKET>clients;
	unsigned int freeSessionId = 0;


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

		if (listen(serverSocket, 1) == SOCKET_ERROR) { std::cout << "Error listening on socket.\n"; }
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

		if (clientSocket == SOCKET_ERROR) { return false; }

		for (unsigned int i = 0; i < pow(2, 32) - 1; i++) {
			if (clients.find(freeSessionId) == clients.end()) { clients[freeSessionId] = clientSocket; return true; }

			if (freeSessionId < pow(2, 32) - 1) { freeSessionId++; }
			else { freeSessionId = 0; }
		}
		return false;
	}

	void addressInit(const std::string& ip, const unsigned int& port) {
		memset(&serverAddress, 0, sizeof(serverAddress));
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_addr.s_addr = inet_addr(ip.c_str());
		serverAddress.sin_port = htons(port);
	}

	void sendBinProtocol(const BinProtocol& data, SOCKET paramSocket) const {
		const unsigned int bytesSent = send(paramSocket, data.to_string().c_str(), 2, 0);
		std::cout << "Bytes sent: " << bytesSent << "\n";
	}

	BinProtocol receiveBinProtocol() {
		int bytesRecv = SOCKET_ERROR;
		char* recvbuf = new char[2];

		while (bytesRecv == SOCKET_ERROR) {
			bytesRecv = recv(clients[0], recvbuf, bufLen, 0);

			if (bytesRecv <= 0 || bytesRecv == WSAECONNRESET) {
				std::cout << "Connection closed.\n";
				break;
			}

			std::cout << "Bytes received: " << bytesRecv << "\n";
		}

		std::string str(recvbuf);
		str.resize(2);
		std::cout << "Received text: " << str << "\n";
		return BinProtocol(std::string(str));
	}
};

