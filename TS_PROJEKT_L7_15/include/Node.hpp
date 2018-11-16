#pragma once

#include "Protocol.hpp"
#include <WS2tcpip.h>
#include <winsock.h>
#include <mutex>

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
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR){
			mutex.lock();
			std::cout << GetCurrentTimeAndDate() << " : " << "Error creating socket: " << "Initialization error.\n";
			mutex.unlock();
		}

		nodeSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (nodeSocket == INVALID_SOCKET) {
			mutex.lock();
			std::cout << GetCurrentTimeAndDate() << " : " << "Error creating socket: " << WSAGetLastError() << "\n";
			mutex.unlock();
			WSACleanup();
		}

		this->addressInit(address, port);
	}
	virtual ~NodeTCP(){
		closesocket(nodeSocket);
		WSACleanup();
	}

	//Metody publiczne
	void sendBinProtocol(const BinProtocol& data, SOCKET& clientSocket) {
		const std::string sendStr = data.to_string();
		char* sendBuf = new char[BUF_LENGTH];
		memcpy(sendBuf, sendStr.c_str(), BUF_LENGTH);


		const unsigned int bytesSent = send(clientSocket, sendBuf, BUF_LENGTH, 0);
		mutex.lock();
		std::cout << GetCurrentTimeAndDate() << " : " << "Bytes sent: " << bytesSent << "\n";
		std::cout << GetCurrentTimeAndDate() << " : " << "Sent protocol: " << data << '\n';
		mutex.unlock();
	}

	void receiveBinProtocol(const SOCKET &paramSocket, BinProtocol& output) {
		int bytesRecv = SOCKET_ERROR;
		char* recvbuf = new char[BUF_LENGTH];

		while (bytesRecv == SOCKET_ERROR) {
			bytesRecv = recv(paramSocket, recvbuf, BUF_LENGTH, 0);

			if (bytesRecv <= 0 || bytesRecv == WSAECONNRESET) {
				mutex.lock();
				std::cout << GetCurrentTimeAndDate() << " : " << "Connection closed.\n";
				mutex.unlock();
				output = BinProtocol();
				break;
			}
			mutex.lock();
			std::cout << GetCurrentTimeAndDate() << " : " << "Bytes received: " << bytesRecv << "\n";
			mutex.unlock();
		}

		std::string str(recvbuf);
		str.resize(BUF_LENGTH);
		output.from_string(str);


		mutex.lock();
		std::cout << GetCurrentTimeAndDate() << " : " << "Received protocol: " << BinProtocol(str) << '\n';
		mutex.unlock();
	}
};
