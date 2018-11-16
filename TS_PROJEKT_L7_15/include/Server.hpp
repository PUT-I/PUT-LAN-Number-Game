#pragma once

#include "Node.hpp"
#include <WS2tcpip.h>
#include <winsock.h>

#include <iostream>
#include <string>
#include <unordered_map>
#include <thread>
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
		if (!socketBind()) { std::cout << GetCurrentTimeTm() << " : " << "bind() failed.\n"; }

		if (listen(nodeSocket, 1) == SOCKET_ERROR) { std::cout << GetCurrentTimeTm() << " : " << "Error listening on socket.\n"; }
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

	void sendBinProtocolToAll(const BinProtocol &data) {
		std::vector<std::thread>threads;
		std::vector<BinProtocol>dataVec;
		for (const unsigned int id : sessionIds) {
			dataVec.push_back(data);
			(dataVec.end() - 1)->setId(id);
			threads.push_back(std::thread(&ServerTCP::sendBinProtocol, this, std::ref(*(dataVec.end() - 1)), std::ref(clientSockets[id])));
		}
		for (std::thread& thread : threads) { thread.join(); }
		dataVec.clear();
	}

	void receiveBinProtocols(BinProtocol& output, SOCKET& clientSocket, bool& stop) {
		char* recvbuf = new char[BUF_LENGTH];

		while (!stop) {

			const int bytesRecv = recv(clientSocket, recvbuf, BUF_LENGTH, 0);
			if (bytesRecv <= 0 || bytesRecv == WSAECONNRESET) {
				mutex.lock();
				for (unsigned int i = 0; i < sessionIds.size(); i++) {
					if (clientSockets[sessionIds[i]] == clientSocket) {
						std::cout << GetCurrentTimeTm() << " : " << "Client " << i + 1 << " (with id " << sessionIds[i] << ") disconnected.\n";
						std::cerr << "(err)" << GetCurrentTimeTm() << " : " << "Client " << i + 1 << " (with id " << sessionIds[i] << ") disconnected.\n";
						sessionIds.erase(sessionIds.begin() + i);
					}
				}
				stop = true;
				output = BinProtocol();
				mutex.unlock();
				return;
			}

			mutex.lock();
			std::cout << '\n' << GetCurrentTimeTm() << " : " << "Bytes received: " << bytesRecv << "\n";
			mutex.unlock();
		}

		std::string str(recvbuf);
		str.resize(2);
		mutex.lock();
		std::cout << GetCurrentTimeTm() << " : " << "Received protocol: " << BinProtocol(str) << '\n';
		mutex.unlock();

		output.from_string(str);
	}

	//Rozgrywka
	void startGame() {
		//Obliczenie czasu rozgrywki
		//const unsigned int gameDuration = (abs(int(sessionIds[0] - sessionIds[1])) * 74) % 90 + 25;
		const unsigned int gameDuration = 20;

		const auto timeEnd = std::chrono::duration<double>(gameDuration);
		std::cout << '\n' << GetCurrentTimeTm() << " : " << "Game duration: " << gameDuration << "s\n";
		std::cerr << "(err)" << GetCurrentTimeTm() << " : " << "Game duration: " << gameDuration << "s\n";
		this->sendBinProtocolToAll(BinProtocol(OP_TIME, TIME_DURATION, NULL, gameDuration));

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
			if (stop) { break; }
			std::cout << GetCurrentTimeTm() << " : " << "Time to start: " << i << "s\n";
			std::cerr << "(err)" << GetCurrentTimeTm() << " : " << "Time to start: " << i << "s\n";
			this->sendBinProtocolToAll(BinProtocol(OP_TIME, TIME_TO_START, NULL, i));
			Sleep(1000);
		}
		std::cout << '\n';

		//Rozpoczêcie rozgrywki
		if (!stop) {
			//Wys³anie wiadomoœci o starcie rozgrywki
			std::cout << GetCurrentTimeTm() << " : " << "Game start.\n";
			std::cout << '\n' << GetCurrentTimeTm() << " : " << "Send game start info to players.\n";
			this->sendBinProtocolToAll(BinProtocol(OP_GAME, GAME_BEGIN, NULL, NULL));

			//Wys³anie wiadomoœci o czasie
			std::cout << '\n' << GetCurrentTimeTm() << " : " << "Send time to players.\n";
			std::cout << GetCurrentTimeTm() << " : " << "Time left: " << gameDuration << "s\n";
			std::cerr << "(err)" << GetCurrentTimeTm() << " : " << "Time left: " << gameDuration << "s\n";
			this->sendBinProtocolToAll(BinProtocol(OP_TIME, TIME_LEFT, NULL, gameDuration));
		}

		//Rozgrywka
		if (!stop) {
			//Zmienne do informowania o pozosta³ym czasie
			const auto timeStart = std::chrono::system_clock::now();		  //Czas rozpoczêcia
			auto timeMessageStart = std::chrono::system_clock::now();		  //Czas rozpoczêcia od wiadomoœci
			auto timeMessage = std::chrono::duration<double>(0);			  //Czas u¿ywany przy wiadomoœci

			for (auto time = std::chrono::duration<double>(0); time < timeEnd; time = std::chrono::system_clock::now() - timeStart) {
				timeMessage = std::chrono::system_clock::now() - timeMessageStart;

				if (stop) {
					break;
				}
				if (timeMessage >= std::chrono::duration<double>(15)) {
					std::cout << '\n' << GetCurrentTimeTm() << " : " << "Send time to players.\n";
					std::cout << GetCurrentTimeTm() << " : " << "Time left: " << unsigned int(ceil((timeEnd - time).count())) << "s\n";
					std::cerr << "(err)" << GetCurrentTimeTm() << " : " << "Time left: " << unsigned int(ceil((timeEnd - time).count())) << "s\n";
					//Wys³anie wiadomoœci o czasie
					this->sendBinProtocolToAll(BinProtocol(OP_TIME, TIME_LEFT, NULL, unsigned int(ceil((timeEnd - time).count()))));
					//Koniec wysy³ania wiadomoœci o czasie
					timeMessage = std::chrono::duration<double>(0);
					timeMessageStart = std::chrono::system_clock::now();
				}
			}
		}

		//Zakoñczenie rozgrywki
		if (!stop) {
			stop = true;
			this->sendBinProtocolToAll(BinProtocol(OP_GAME, GAME_END, NULL, NULL));
			mutex.lock();
			std::cout << '\n' << GetCurrentTimeTm() << " : " << "Game end.\n";
			mutex.unlock();
		}
		else{
			this->sendBinProtocolToAll(BinProtocol(OP_MESSAGE, MESSAGE_OPPONENT_DISCONNECTED, NULL, NULL));
		}

		//£¹czenie w¹tków odbierania
		for (std::thread& thread : threads) {
			thread.join();
			mutex.lock();
			std::cout << '\n' << GetCurrentTimeTm() << " : " << "Client  receive end.\n";
			mutex.unlock();
		}
	}
};
