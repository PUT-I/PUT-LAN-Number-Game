#pragma once
#include <Protocol.hpp>
#include <WS2tcpip.h>
#include <winsock.h>
#include <iostream>
#include <string>
#include <unordered_map>

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable:4996) 

class ClientTCP {
private:
	static const unsigned int BUF_LENGTH = 2;

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

	void sendBinProtocol(const BinProtocol& data) const {
		const unsigned int bytesSent = send(clientSocket, data.to_string().c_str(), BUF_LENGTH, 0);

		std::cout << "Bytes sent: " << bytesSent << "\n";
		std::cout << "Text sent: " << data.to_string().c_str() << "\n";
	}

	BinProtocol receiveBinProtocol() {
		int bytesRecv = SOCKET_ERROR;
		char* recvbuf = new char[2];

		while (bytesRecv == SOCKET_ERROR) {
			bytesRecv = recv(clientSocket, recvbuf, BUF_LENGTH, 0);

			if (bytesRecv <= 0 || bytesRecv == WSAECONNRESET) {
				std::cout << "Connection closed.\n";
				break;
			}

			std::cout << "Bytes received: " << bytesRecv << "\n";
		}

		std::string str(recvbuf);
		str.resize(2);
		std::cout << "Received text: " << str << "\n";

		return BinProtocol(str);
	}
};
