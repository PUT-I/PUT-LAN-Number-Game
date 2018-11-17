#pragma once

#include "ThreadSafe.hpp"
#include "Protocol.hpp"
#include <WS2tcpip.h>
#include <winsock.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable:4996) 

class NodeTCP {
protected:
	//D³ugoœæ bufora wysy³ania i odbioru
	static const unsigned int BUF_LENGTH = BinProtocol::length;

	WSADATA wsaData;
	SOCKET nodeSocket;
	sockaddr_in address;

	std::mutex mutex;

	//Metody prywatne
	void addressInit(const std::string& ip, const unsigned int& port) {
		memset(&address, 0, sizeof(address));
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = inet_addr(ip.c_str());
		address.sin_port = htons(port);
	}

	//Konstruktory
public:
	NodeTCP(const std::string &address, const unsigned int& port) {
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
			sync_cerr << GetCurrentTimeTm() << " : " << "Error creating socket: " << "Initialization error.\n";
		}

		nodeSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (nodeSocket == INVALID_SOCKET) {
			sync_cerr << GetCurrentTimeTm() << " : " << "Error creating socket: " << WSAGetLastError() << "\n";
			WSACleanup();
		}

		this->addressInit(address, port);
	}
	virtual ~NodeTCP() {
		closesocket(nodeSocket);
		WSACleanup();
	}

	//Metody publiczne
	void sendBinProtocol(const BinProtocol &data, SOCKET& clientSocket) const {
		const std::string sendStr = data.to_string();
		char sendBuf[BUF_LENGTH];
		for (unsigned int i = 0; i < BUF_LENGTH; i++) {
			sendBuf[i] = sendStr[i];
		}

		const unsigned int bytesSent = send(clientSocket, sendBuf, BUF_LENGTH, 0);
		sync_cerr << GetCurrentTimeTm() << " : " << "Bytes sent: " << bytesSent << "\n";
		sync_cerr << GetCurrentTimeTm() << " : " << "Sent protocol: " << data << '\n';
		sync_cerr << GetCurrentTimeTm() << " : " << "Sent bits: ";
		for (unsigned int i = 0; i < BUF_LENGTH; i++) { sync_cerr << std::bitset<8>(sendBuf[i]) << (i < BUF_LENGTH - 1 ? " " : ""); }
		sync_cerr << '\n';
	}

	void receiveBinProtocol(const SOCKET &paramSocket, BinProtocol& output) const {
		int bytesRecv = SOCKET_ERROR;
		char recvBuf [BUF_LENGTH];

		while (bytesRecv == SOCKET_ERROR) {
			bytesRecv = recv(paramSocket, recvBuf, BUF_LENGTH, 0);

			if (bytesRecv <= 0 || bytesRecv == WSAECONNRESET) {
				sync_cerr << GetCurrentTimeTm() << " : " << "Connection closed.\n";
				output = BinProtocol();
				return;
			}
			sync_cerr << GetCurrentTimeTm() << " : " << "Bytes received: " << bytesRecv << "\n";
		}


		output.from_char_a(recvBuf);
		sync_cerr << GetCurrentTimeTm() << " : " << "Received protocol: " << output << '\n';
		sync_cerr << GetCurrentTimeTm() << " : " << "Received bits: ";
		for (unsigned int i = 0; i < BUF_LENGTH; i++) { sync_cerr << std::bitset<8>(recvBuf[i]) << (i < BUF_LENGTH - 1 ? " " : ""); }
		sync_cerr << '\n';
	}
};
