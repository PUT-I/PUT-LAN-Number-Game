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
	static const unsigned int BUF_LENGTH = BinProtocol::length;

	WSADATA wsaData;
	SOCKET clientSocket;
	sockaddr_in clientAddress;

	unsigned sessionId = 0;


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
	~ClientTCP() {
		closesocket(clientSocket);
		WSACleanup();
	}


private:
	void addressInit(const std::string& ip, const unsigned int& port) {
		memset(&clientAddress, 0, sizeof(clientAddress));
		clientAddress.sin_family = AF_INET;
		clientAddress.sin_addr.s_addr = inet_addr(ip.c_str());
		clientAddress.sin_port = htons(port);
	}


public:
	bool connectServer() {
		if (connect(clientSocket, reinterpret_cast<SOCKADDR*>(&clientAddress), sizeof(clientAddress)) != SOCKET_ERROR) {
			BinProtocol protocol = receiveBinProtocol();
			sessionId = protocol.getId_Int();
			std::cout << "Session id: " << sessionId << "\n";
			return true;
		}
		else { return false; }
	}

	void sendBinProtocol(BinProtocol& data) const {
		data.setId(sessionId);
		const unsigned int bytesSent = send(clientSocket, data.to_string().c_str(), BUF_LENGTH, 0);

		std::cout << "Bytes sent: " << bytesSent << "\n";
		std::cout << "Sent protocol: "; data.display();
	}

	const BinProtocol receiveBinProtocol() const {
		int bytesRecv = SOCKET_ERROR;
		char* recvbuf = new char[BUF_LENGTH];

		while (bytesRecv == SOCKET_ERROR) {
			bytesRecv = recv(clientSocket, recvbuf, BUF_LENGTH, 0);

			if (bytesRecv <= 0 || bytesRecv == WSAECONNRESET) {
				std::cout << "Connection closed.\n";
				return BinProtocol();
			}

			std::cout << "Bytes received: " << bytesRecv << "\n";
		}

		std::string str(recvbuf);
		str.resize(BUF_LENGTH);
		std::cout << "Received protocol: ";
		BinProtocol(str).display();

		return BinProtocol(str);
	}

	void startGame() {
		BinProtocol tempProt = this->receiveBinProtocol();

		//Czekanie na rozpoczêcie rozgrywki
		while (tempProt.getId_Int() != sessionId && tempProt.getOperation().to_string() != "001") {}

		std::cout << "\nGame start.\n";
		while (true) {
			tempProt = this->receiveBinProtocol();
			if (tempProt.getOperation().to_string() == "000") { std::cout << "ERROR\n"; break; }
			if (tempProt.getId_Int() == sessionId && tempProt.getOperation().to_string() == "111") {
				std::cout << "Game end.\n";
				break;
			}
			if (tempProt.getId_Int() == sessionId && tempProt.getOperation().to_string() == "110") {
				std::cout << "Time left: " << tempProt.getData_Int() << "s\n\n";
			}

		}
	}
};
