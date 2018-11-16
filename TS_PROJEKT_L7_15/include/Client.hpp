#pragma once
#include "Node.hpp"
#include <WS2tcpip.h>
#include <winsock.h>
#include <iostream>
#include <string>

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
				std::cout << GetCurrentTimeTm() << " : " << "Session id not received.\n";
				return false;
			}
			else { sessionId = tempProt.getData_Int(); }

			std::cout << GetCurrentTimeTm() << " : " << "Session id: " << sessionId << "\n";
			return true;
		}
		else { return false; }
	}

	//Metody publiczne
	void startGame() {
		BinProtocol tempProt;


		//Czekanie na uzyskanie informacji o czasie rozgrywki
		while (!tempProt.compare(OP_TIME, TIME_DURATION, sessionId)) {
			receiveBinProtocol(nodeSocket, tempProt);
			if (tempProt.compare(OP_MESSAGE, MESSAGE_SENDER_DISCONNECTED, NULL)) {
				std::cout << GetCurrentTimeTm() << " : " << "Server disconnected.\n";
				return;
			}
		}
		std::cout << '\n' << GetCurrentTimeTm() << " : " << "Game duration: " << tempProt.getData_Int() << "s\n";
		std::cerr << "(err)" << GetCurrentTimeTm() << " : " << "Game duration: " << tempProt.getData_Int() << "s\n";

		//Czekanie na rozpoczęcie rozgrywki
		while (!tempProt.compare(OP_GAME, GAME_BEGIN, sessionId)) {
			receiveBinProtocol(nodeSocket, tempProt);
			if (tempProt.compare(OP_TIME, TIME_TO_START, sessionId)) {
				std::cout << GetCurrentTimeTm() << " : " << "Time to start: " << tempProt.getData_Int() << "s\n";
				std::cerr << "(err)" << GetCurrentTimeTm() << " : " << "Time to start: " << tempProt.getData_Int() << "s\n";
			}
			else if (tempProt.compare(OP_MESSAGE, MESSAGE_SENDER_DISCONNECTED, NULL)) {
				std::cout << GetCurrentTimeTm() << " : " << "Server disconnected.\n";
				return;
			}
			else if (tempProt.compare(OP_MESSAGE, MESSAGE_OPPONENT_DISCONNECTED, sessionId)) {
				std::cout << GetCurrentTimeTm() << " : " << "Opponent disconnected.\n";
				return;
			}
		}

		std::cout << GetCurrentTimeTm() << " : " << "Game start.\n\n";
		while (true) {
			this->receiveBinProtocol(nodeSocket, tempProt);
			if (tempProt.compare(OP_MESSAGE, MESSAGE_SENDER_DISCONNECTED, NULL)) {
				std::cout << GetCurrentTimeTm() << " : " << "Server disconnected.\n";
				return;
			}
			else if (tempProt.compare(OP_MESSAGE, MESSAGE_OPPONENT_DISCONNECTED, sessionId)) {
				std::cout << GetCurrentTimeTm() << " : " << "Opponent disconnected.\n";
				return;
			}
			else if (tempProt.compare(OP_GAME, GAME_END, sessionId)) {
				std::cout << GetCurrentTimeTm() << " : " << "Game end.\n";
				break;
			}
			else if (tempProt.compare(OP_TIME, TIME_LEFT, sessionId)) {
				std::cout << GetCurrentTimeTm() << " : " << "Time left: " << tempProt.getData_Int() << "s\n\n";
				std::cerr << "(err)" << GetCurrentTimeTm() << " : " << "Time left: " << tempProt.getData_Int() << "s\n\n";
			}
		}
	}
};
