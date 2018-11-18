#pragma once

#include "ThreadSafe.hpp"
#include "Protocol.hpp"
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable:4996) 

class NodeTCP {
protected:
	//D³ugoœæ bufora wysy³ania i odbioru
	static const unsigned int BUF_LENGTH = BinProtocol::length;

	WSADATA wsaData;
	SOCKET nodeSocket;
	sockaddr_in nodeInfo;

	std::mutex mutex;

public:
	//Metody inicjalizacyjne
	void info_init(const unsigned long& address, const unsigned int& port) {
		memset(&nodeInfo, 0, sizeof(nodeInfo));
		nodeInfo.sin_family = AF_INET;
		nodeInfo.sin_addr.s_addr = address;
		nodeInfo.sin_port = htons(port);
	}

	//Konstruktory
	NodeTCP(): nodeInfo() {
		if (WSAStartup(0x0101, &wsaData) != NO_ERROR)
		{
			sync_cerr << GET_CURRENT_TIME() << " : " << "Error creating socket: " << "Initialization error.\n";
		}

		nodeSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (nodeSocket == INVALID_SOCKET)
		{
			sync_cerr << GET_CURRENT_TIME() << " : " << "Error creating socket: " << WSAGetLastError() << "\n";
			WSACleanup();
		}
	}

	NodeTCP(const unsigned long& address, const unsigned int& port) : NodeTCP() {
		this->info_init(address, port);
	}
	virtual ~NodeTCP() {
		closesocket(nodeSocket);
		WSACleanup();
	}

	//Metody publiczne
	static void send_bin_protocol(const BinProtocol &data, SOCKET& clientSocket) {
		const std::string sendStr = data.to_string();
		char sendBuf[BUF_LENGTH];
		for (unsigned int i = 0; i < BUF_LENGTH; i++) {
			sendBuf[i] = sendStr[i];
		}

		const unsigned int bytesSent = send(clientSocket, sendBuf, BUF_LENGTH, 0);
		sync_cerr << GET_CURRENT_TIME() << " : " << "Bytes sent: " << bytesSent << "\n";
		sync_cerr << GET_CURRENT_TIME() << " : " << "Sent protocol: " << data << '\n';
		sync_cerr << GET_CURRENT_TIME() << " : " << "Sent bits: ";
		for (unsigned int i = 0; i < BUF_LENGTH; i++) { sync_cerr << std::bitset<8>(sendBuf[i]) << (i < BUF_LENGTH - 1 ? " " : ""); }
		sync_cerr << '\n';
	}

	static void receive_bin_protocol(const SOCKET &paramSocket, BinProtocol& output) {
		int bytesRecv = SOCKET_ERROR;
		char recvBuf[BUF_LENGTH];

		while (bytesRecv == SOCKET_ERROR) {
			bytesRecv = recv(paramSocket, recvBuf, BUF_LENGTH, 0);

			if (bytesRecv <= 0 || bytesRecv == WSAECONNRESET) {
				sync_cerr << GET_CURRENT_TIME() << " : " << "Connection closed.\n";
				output = BinProtocol();
				return;
			}
			sync_cerr << GET_CURRENT_TIME() << " : " << "Bytes received: " << bytesRecv << "\n";
		}


		output.from_char_a(recvBuf);
		sync_cerr << GET_CURRENT_TIME() << " : " << "Received protocol: " << output << '\n';
		sync_cerr << GET_CURRENT_TIME() << " : " << "Received bits: ";
		for (unsigned int i = 0; i < BUF_LENGTH; i++) { sync_cerr << std::bitset<8>(recvBuf[i]) << (i < BUF_LENGTH - 1 ? " " : ""); }
		sync_cerr << '\n';
	}
};
