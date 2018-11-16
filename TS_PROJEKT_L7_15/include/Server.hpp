#pragma once

#include <Protocol.hpp>
#include <WS2tcpip.h>
#include <winsock.h>

#include <iostream>
#include <string>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <random>
#include <chrono>

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable:4996) 

inline int randInt(const int &min, const int &max) {
	if (max <= min) return min;
	std::random_device rd;
	std::mt19937 gen(rd());
	const std::uniform_int_distribution<int > d(min, max);
	return d(gen);
}


class ServerTCP {
private:
	static const unsigned int BUF_LENGTH = BinProtocol::length;

	WSADATA wsaData;
	SOCKET serverSocket;
	sockaddr_in serverAddress;
	std::unordered_map<unsigned int, SOCKET>clients;
	std::vector<unsigned int> sessionIds;
	std::mutex mutex;

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
	~ServerTCP() {
		//Zamykanie gniazdek klientów
		for (const auto& elem : clients) {
			closesocket(elem.second);
		}
		clients.clear();

		//Zamkniêcie gniazdka servera
		closesocket(serverSocket);
		WSACleanup();
	}


private:
	void addressInit(const std::string& ip, const unsigned int& port) {
		memset(&serverAddress, 0, sizeof(serverAddress));
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_addr.s_addr = inet_addr(ip.c_str());
		serverAddress.sin_port = htons(port);
	}
	bool socketBind() {
		if (bind(serverSocket, reinterpret_cast<SOCKADDR*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR) {
			closesocket(serverSocket);
			return false;
		}
		return true;
	}


public:
	bool acceptClient() {
		SOCKET clientSocket = accept(this->serverSocket, nullptr, nullptr);

		if (clientSocket == SOCKET_ERROR) { return false; }

		for (unsigned int i = 0; i < pow(2, 5) - 1; i++) {
			const unsigned int sessionId = randInt(1, int(pow(2, 5) - 1));
			if (clients.find(sessionId) == clients.end()) {
				clients[sessionId] = clientSocket;
				sessionIds.push_back(sessionId);
				sendBinProtocol(BinProtocol("100", "000", std::bitset<5>(sessionId).to_string(), NULL), clientSocket);
				return true;
			}
		}
		return false;
	}

	void sendBinProtocol(const BinProtocol& data, SOCKET& clientSocket) {
		const unsigned int bytesSent = send(clientSocket, data.to_string().c_str(), BUF_LENGTH, 0);
		mutex.lock();
		std::cout << "Bytes sent: " << bytesSent << "\n";
		std::cout << "Sent protocol: "; data.display();
		mutex.unlock();
	}

	void receiveBinProtocol(BinProtocol& output) {
		int bytesRecv = SOCKET_ERROR;
		char* recvbuf = new char[BUF_LENGTH];

		while (bytesRecv == SOCKET_ERROR) {
			bytesRecv = recv(clients[0], recvbuf, BUF_LENGTH, 0);

			if (bytesRecv <= 0 || bytesRecv == WSAECONNRESET) {
				std::cout << "Connection closed.\n";
				output = BinProtocol();
				break;
			}

			std::cout << "Bytes received: " << bytesRecv << "\n";
		}

		std::string str(recvbuf);
		str.resize(BUF_LENGTH);
		output.from_string(str);


		mutex.lock();
		std::cout << "Received protocol: "; BinProtocol(str).display();
		mutex.unlock();
	}

	void receiveBinProtocols(BinProtocol& output, SOCKET& clientSocket, bool& stop) {
		int bytesRecv;
		char* recvbuf = new char[BUF_LENGTH];

		while (!stop) {
			if (stop) { return; }
			bytesRecv = recv(clientSocket, recvbuf, BUF_LENGTH, 0);

			if (bytesRecv <= 0 || bytesRecv == WSAECONNRESET) {
				std::cout << "Connection closed.\n";
				output = BinProtocol();
				return;
			}

			mutex.lock();
			std::cout << "\nBytes received: " << bytesRecv << "\n";
			mutex.unlock();
		}

		std::string str(recvbuf);
		str.resize(2);
		mutex.lock();
		std::cout << "Received protocol: ";
		mutex.unlock();
		BinProtocol(str).display();

		output.from_string(str);
	}

	void startGame() {
		//Obliczenie czasu rozgrywki
		//const unsigned int gameDuration = (abs(int(sessionIds[0] - sessionIds[1])) * 74) % 90 + 25;
		const unsigned int gameDuration = 20;

		const auto timeEnd = std::chrono::duration<double>(gameDuration);
		std::cout << "\nGame duration: " << gameDuration << "s\n";

		//Zmienne do zarz¹dzania w¹tkami
		bool stop = false; //Zmiena gdy prawda przerywa odbieranie danych
		std::vector<BinProtocol> outputs(clients.size());
		std::vector<std::thread> threads(clients.size());

		//Uruchamianie w¹tków odbierania
		threads[0] = std::thread([this, &outputs, &stop] {this->receiveBinProtocols(outputs[0], clients[sessionIds[0]], stop); });
		threads[1] = std::thread([this, &outputs, &stop] {this->receiveBinProtocols(outputs[1], clients[sessionIds[1]], stop); });

		//Wygenerowanie losowej liczby
		unsigned int secretNumber = randInt(0, int(pow(2, 5)) - 1);

		std::cout << '\n';
		for (int i = 5; i > 0; i--) {
			std::cout << "Time to start: " << i << "s\n";
			std::cerr << "(err)Time to start: " << i << "s\n";
			Sleep(1000);
		}
		std::cout << '\n';

		//Rozpoczêcie rozgrywki
		{
			std::cout << "Game start.\n";
			std::cout << "\nSend game start info to players.\n";
			std::thread sendStartThread1([this] {this->sendBinProtocol(BinProtocol(gameStart, "000", "00000", NULL), clients[sessionIds[0]]); });
			std::thread sendStartThread2([this] {this->sendBinProtocol(BinProtocol(gameStart, "000", "00000", NULL), clients[sessionIds[1]]); });
			sendStartThread1.join();
			sendStartThread2.join();

			//Wys³anie wiadomoœci o czasie
			std::cout << "\nSend time to players.\n";
			std::cout << "Time left: " << gameDuration << "s\n";;
			std::cerr << "(err)Time left: " << gameDuration << "s\n";;
			this->sendBinProtocol(BinProtocol(sendTime, "000", std::bitset<5>(sessionIds[0]).to_string(), gameDuration), clients[sessionIds[0]]);
			this->sendBinProtocol(BinProtocol(sendTime, "000", std::bitset<5>(sessionIds[1]).to_string(), gameDuration), clients[sessionIds[1]]);
		}

		//Rozgrywka
		{
			//Informowanie o pozosta³ym czasie i ewentualnym wygraniu rozgrywki
			const auto timeStart = std::chrono::system_clock::now();		  //Czas rozpoczêcia
			auto timeMessageStart = std::chrono::system_clock::now();		  //Czas rozpoczêcia od wiadomoœci
			auto timeMessage = std::chrono::duration<double>(0);			  //Czas u¿ywany przy wiadomoœci
			const auto timeMessagePeriod = std::chrono::duration<double>(15); //Okres w jakim maj¹ pojawiaæ siê wiadomoœci

			for (auto time = std::chrono::duration<double>(0); time < timeEnd; time = std::chrono::system_clock::now() - timeStart) {
				timeMessage = std::chrono::system_clock::now() - timeMessageStart;

				if (timeMessage >= timeMessagePeriod) {
					std::cout << "\nSend time to players.";
					std::cout << "\nTime left: " << unsigned int(ceil((timeEnd - time).count())) << "s\n";
					std::cerr << "(err)Time left: " << unsigned int(ceil((timeEnd - time).count())) << "s\n";
					//Wys³anie wiadomoœci o czasie
					this->sendBinProtocol(BinProtocol(sendTime, "000", std::bitset<5>(sessionIds[0]).to_string(), unsigned int(ceil((timeEnd - time).count()))), clients[sessionIds[0]]);
					this->sendBinProtocol(BinProtocol(sendTime, "000", std::bitset<5>(sessionIds[1]).to_string(), unsigned int(ceil((timeEnd - time).count()))), clients[sessionIds[1]]);
					//Koniec wysy³ania wiadomoœci o czasie
					timeMessage = std::chrono::duration<double>(0);
					timeMessageStart = std::chrono::system_clock::now();
				}
			}
		}

		//Zakoñczenie rozgrywki
		{
			stop = true;
			std::thread sendEndThread1([this] {this->sendBinProtocol(BinProtocol(gameEnd, "000", std::bitset<5>(sessionIds[0]).to_string(), NULL), clients[sessionIds[0]]); });
			std::thread sendEndThread2([this] {this->sendBinProtocol(BinProtocol(gameEnd, "000", std::bitset<5>(sessionIds[1]).to_string(), NULL), clients[sessionIds[1]]); });

			std::cout << "\nGame end.\n";

			//£¹czenie w¹tków odbierania
			for (unsigned int i = 0; i < threads.size(); i++) { mutex.lock(); std::cout << "\nThread " << i + 1 << " joined.\n"; threads[i].join(); mutex.unlock(); }
			sendEndThread1.join();
			sendEndThread2.join();
		}
	}
};

