#pragma once
#include "Node.hpp"
#include <WS2tcpip.h>
#include <winsock.h>
#include <iostream>
#include <string>
#include <unordered_map>

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable:4996) 

class ClientTCP : public NodeTCP {
private:
	//Id sesji klienta
	unsigned sessionId = 0;

	//Kontstruktory
public:
	ClientTCP(const std::string &address, const unsigned int& port) : NodeTCP(address, port) {}

	//Metody prywatne
	bool connectServer() {
		if (connect(nodeSocket, reinterpret_cast<SOCKADDR*>(&address), sizeof(address)) != SOCKET_ERROR) {
			BinProtocol tempProt;
			receiveBinProtocol(nodeSocket, tempProt);
			//Sprawdzanie czy odebrany protokół dotyczy przydzielenia Id
			if (!tempProt.compare(OP_DATA, DATA_ID, NULL)) {
				return false;
			}
			else { sessionId = tempProt.getData_Int(); }

			mutex.lock();
			std::cout << GetCurrentTimeAndDate() << " : " << "Session id: " << sessionId << "\n";
			mutex.unlock();
			return true;
		}
		else { return false; }
	}

	//Metody publiczne
	void startGame() {
		BinProtocol tempProt;


		//Czekanie na rozpocz�cie rozgrywki
		while (!tempProt.compare(OP_GAME, GAME_BEGIN, sessionId)) { receiveBinProtocol(nodeSocket, tempProt); }

		mutex.lock();
		std::cout << '\n' << GetCurrentTimeAndDate() << " : " << "Game start.\n";
		mutex.unlock();
		while (true) {
			this->receiveBinProtocol(nodeSocket, tempProt);
			//if (tempProt.getOperation().to_string() == OP_MESSAGE) { //Przerobić później
			//	mutex.lock();
			//	std::cout << GetCurrentTimeAndDate() << " : " << "ERROR\n";
			//	mutex.unlock();
			//	break;
			//}
			if (tempProt.compare(OP_GAME, GAME_END, sessionId)) {
				mutex.lock();
				std::cout << GetCurrentTimeAndDate() << " : " << "Game end.\n";
				mutex.unlock();
				break;
			}
			else if (tempProt.compare(OP_DATA, DATA_TIME, sessionId)) {
				mutex.lock();
				std::cout << GetCurrentTimeAndDate() << " : " << "Time left: " << tempProt.getData_Int() << "s\n\n";
				mutex.unlock();
			}
		}
	}
};
