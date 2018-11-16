#pragma once

#include "Node.hpp"
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


class ServerTCP : public NodeTCP {
private:
	//Tablica gniazdek po³¹czonych klientów (klucz to id sesji klienta)
	std::unordered_map<unsigned int, SOCKET>clientSockets;
	//Tablica id sesji poszczególnych klientów liczonych od 0
	std::vector<unsigned int> sessionIds;

	//Metody prywatne
	bool socketBind() {
		if (bind(nodeSocket, reinterpret_cast<SOCKADDR*>(&address), sizeof(address)) == SOCKET_ERROR) {
			closesocket(nodeSocket);
			return false;
		}
		return true;
	}

	//Konstruktory
public:
	ServerTCP(const std::string &address, const unsigned int& port) : NodeTCP(address, port) {
		if (!socketBind()) { std::cout << GetCurrentTimeAndDate() << " : " << "bind() failed.\n"; }

		if (listen(nodeSocket, 1) == SOCKET_ERROR) { std::cout << GetCurrentTimeAndDate() << " : " << "Error listening on socket.\n"; }
	}
	virtual ~ServerTCP() {
		//Zamykanie gniazdek klientów
		for (const auto& elem : clientSockets) {
			closesocket(elem.second);
		}
		clientSockets.clear();
	}

	//Metody publiczne
	bool acceptClient() {
		SOCKET clientSocket = accept(this->nodeSocket, nullptr, nullptr);

		if (clientSocket == SOCKET_ERROR) { return false; }

		for (unsigned int i = 0; i < pow(2, 5) - 1; i++) {
			const unsigned int sessionId = randInt(1, int(pow(2, 5) - 1));
			if (clientSockets.find(sessionId) == clientSockets.end()) {
				clientSockets[sessionId] = clientSocket;
				sessionIds.push_back(sessionId);
				sendBinProtocol(BinProtocol(OP_DATA, DATA_ID, NULL, sessionId), clientSocket);
				return true;
			}
		}
		return false;
	}

	void receiveBinProtocols(BinProtocol& output, SOCKET& clientSocket, bool& stop) {
		char* recvbuf = new char[BUF_LENGTH];

		while (!stop) {
			if (stop) { return; }
			const int bytesRecv = recv(clientSocket, recvbuf, BUF_LENGTH, 0);

			if (bytesRecv <= 0 || bytesRecv == WSAECONNRESET) {
				std::cout << GetCurrentTimeAndDate() << " : " << "Connection closed.\n";
				output = BinProtocol();
				return;
			}

			mutex.lock();
			std::cout << '\n' << GetCurrentTimeAndDate() << " : " << "Bytes received: " << bytesRecv << "\n";
			mutex.unlock();
		}

		std::string str(recvbuf);
		str.resize(2);
		mutex.lock();
		std::cout << GetCurrentTimeAndDate() << " : " << "Received protocol: " << BinProtocol(str) << '\n';
		mutex.unlock();

		output.from_string(str);
	}

	void startGame() {
		//Obliczenie czasu rozgrywki
		//const unsigned int gameDuration = (abs(int(sessionIds[0] - sessionIds[1])) * 74) % 90 + 25;
		const unsigned int gameDuration = 20;

		const auto timeEnd = std::chrono::duration<double>(gameDuration);
		std::cout << '\n' << GetCurrentTimeAndDate() << " : " << "Game duration: " << gameDuration << "s\n";

		//Zmienne do zarz¹dzania w¹tkami
		bool stop = false; //Zmiena gdy prawda przerywa odbieranie danych
		std::vector<BinProtocol> outputs(clientSockets.size());
		std::vector<std::thread> threads(clientSockets.size());

		//Uruchamianie w¹tków odbierania
		threads[0] = std::thread([this, &outputs, &stop] {this->receiveBinProtocols(outputs[0], clientSockets[sessionIds[0]], stop); });
		threads[1] = std::thread([this, &outputs, &stop] {this->receiveBinProtocols(outputs[1], clientSockets[sessionIds[1]], stop); });

		//Wygenerowanie losowej liczby
		unsigned int secretNumber = randInt(0, int(pow(2, 8)) - 1);

		std::cout << '\n';
		for (unsigned int i = 5; i > 0; i--) {
			std::cout << GetCurrentTimeAndDate() << " : " << "Time to start: " << i << "s\n";
			std::cerr << "(err)" << GetCurrentTimeAndDate() << " : " << "Time to start: " << i << "s\n";
			Sleep(1000);
		}
		std::cout << '\n';

		//Rozpoczêcie rozgrywki
		{
			//Wys³anie wiadomoœci o starcie rozgrywki
			std::cout << GetCurrentTimeAndDate() << " : " << "Game start.\n";
			std::cout << '\n' << GetCurrentTimeAndDate() << " : " << "Send game start info to players.\n";
			std::thread sendStartThread1([this] {this->sendBinProtocol(BinProtocol(OP_GAME, GAME_BEGIN, sessionIds[0], NULL), clientSockets[sessionIds[0]]); });
			std::thread sendStartThread2([this] {this->sendBinProtocol(BinProtocol(OP_GAME, GAME_BEGIN, sessionIds[1], NULL), clientSockets[sessionIds[1]]); });
			sendStartThread1.join();
			sendStartThread2.join();

			//Wys³anie wiadomoœci o czasie
			std::cout << '\n' << GetCurrentTimeAndDate() << " : " << "Send time to players.\n";
			std::cout << GetCurrentTimeAndDate() << " : " << "Time left: " << gameDuration << "s\n";;
			std::cerr << "(err)" << GetCurrentTimeAndDate() << " : " << "Time left: " << gameDuration << "s\n";;
			this->sendBinProtocol(BinProtocol(OP_DATA, DATA_TIME, sessionIds[0], gameDuration), clientSockets[sessionIds[0]]);
			this->sendBinProtocol(BinProtocol(OP_DATA, DATA_TIME, sessionIds[1], gameDuration), clientSockets[sessionIds[1]]);
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
					std::cout << '\n' << GetCurrentTimeAndDate() << " : " << "Send time to players.\n";
					std::cout << GetCurrentTimeAndDate() << " : " << "Time left: " << unsigned int(ceil((timeEnd - time).count())) << "s\n";
					std::cerr << "(err)" << GetCurrentTimeAndDate() << " : " << "Time left: " << unsigned int(ceil((timeEnd - time).count())) << "s\n";
					//Wys³anie wiadomoœci o czasie
					this->sendBinProtocol(BinProtocol(OP_DATA, DATA_TIME, sessionIds[0], unsigned int(ceil((timeEnd - time).count()))), clientSockets[sessionIds[0]]);
					this->sendBinProtocol(BinProtocol(OP_DATA, DATA_TIME, sessionIds[1], unsigned int(ceil((timeEnd - time).count()))), clientSockets[sessionIds[1]]);
					//Koniec wysy³ania wiadomoœci o czasie
					timeMessage = std::chrono::duration<double>(0);
					timeMessageStart = std::chrono::system_clock::now();
				}
			}
		}

		//Zakoñczenie rozgrywki
		{
			stop = true;
			std::thread sendEndThread1([this] {this->sendBinProtocol(BinProtocol(OP_GAME, GAME_END, sessionIds[0], NULL), clientSockets[sessionIds[0]]); });
			std::thread sendEndThread2([this] {this->sendBinProtocol(BinProtocol(OP_GAME, GAME_END, sessionIds[1], NULL), clientSockets[sessionIds[1]]); });
			mutex.lock();
			std::cout << '\n' << GetCurrentTimeAndDate() << " : " << "Game end.\n";
			mutex.unlock();

			//£¹czenie w¹tków odbierania
			for (unsigned int i = 0; i < threads.size(); i++) { mutex.lock(); std::cout << '\n' << GetCurrentTimeAndDate() << " : " << "Thread " << i + 1 << " joined.\n"; threads[i].join(); mutex.unlock(); }
			sendEndThread1.join();
			sendEndThread2.join();
		}
	}
};
