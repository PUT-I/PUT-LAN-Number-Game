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

	//Metody prywatne
	bool inputCheck(const std::string& input) const
	{
		if (input.find_first_not_of("0123456789") != std::string::npos) { return false; }
		else {
			const int numberCheck = std::stoi(input);
			//Czy dodatnia i mniejsza niż zakres
			if (numberCheck < 0 || numberCheck > int(pow(2, 8) - 1)) { return false; }
		}
		return true;
	}

	void listenForServerMessages(BinProtocol& tempProt, bool& stop) {
		while (!stop) {
			this->receiveBinProtocol(nodeSocket, tempProt);
			if (tempProt.compare(OP_MESSAGE, MESSAGE_SENDER_DISCONNECTED, NULL)) {
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Server disconnected.\n";
				sync_cerr << GetCurrentTimeTm() << " : " << "Server disconnected.\n";
				sync_cout << GetCurrentTimeTm() << " : " << "Enter anything to exit: ";
				stop = true;
			}
			else if (tempProt.compare(OP_MESSAGE, MESSAGE_OPPONENT_DISCONNECTED, sessionId)) {
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Opponent disconnected.\n";
				sync_cerr << GetCurrentTimeTm() << " : " << "Opponent disconnected.\n";
				sync_cout << GetCurrentTimeTm() << " : " << "Enter anything to exit: ";
				stop = true;
			}
			else if (tempProt.compare(OP_GAME, GAME_END, sessionId)) {
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Game end.\n";
				sync_cerr << GetCurrentTimeTm() << " : " << "Game end.\n";
				sync_cout << GetCurrentTimeTm() << " : " << "Enter anything to exit: ";
				stop = true;
			}
			else if (tempProt.compare(OP_TIME, TIME_LEFT, sessionId)) {
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Time left: " << tempProt.getData_Int() << "s\n\n";
				sync_cerr << GetCurrentTimeTm() << " : " << "Time left: " << tempProt.getData_Int() << "s\n\n";
				sync_cout << GetCurrentTimeTm() << " : " << "Enter number: ";
			}
			else if (tempProt.compare(OP_NUMBER, NUMBER_TOO_BIG, sessionId)) {
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Number too big. Try again.\n";
				sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "Number too big. Try again.\n";
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Enter number: ";
			}
			else if (tempProt.compare(OP_NUMBER, NUMBER_TOO_SMALL, sessionId)) {
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Number too small. Try again.\n";
				sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "Number too small. Try again.\n";
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Enter number: ";
			}
			else if (tempProt.compare(OP_GAME, GAME_DRAW, sessionId)){
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "No one won the game.\n";
				sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "No one won the game.\n";
				sync_cout << GetCurrentTimeTm() << " : " << "Enter anything to exit: ";
				stop = true;
			}
			else if (tempProt.compare(OP_GAME, GAME_WON, sessionId)) {
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "You won the game.\n";
				sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "You won the game.\n";
				sync_cout << GetCurrentTimeTm() << " : " << "Enter anything to exit: ";
				stop = true;
			}
			else if (tempProt.compare(OP_GAME, GAME_LOST, sessionId)) {
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "You lost the game.\n";
				sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "You lost the game.\n";
				sync_cout << GetCurrentTimeTm() << " : " << "Enter anything to exit: ";
				stop = true;
			}
		}
	}

	//Kontstruktory
public:
	ClientTCP(const std::string &address, const unsigned int& port) : NodeTCP(address, port) {}

	//Metody publiczne
	bool connectServer() {
		if (connect(nodeSocket, reinterpret_cast<SOCKADDR*>(&address), sizeof(address)) != SOCKET_ERROR) {
			BinProtocol tempProt;
			receiveBinProtocol(nodeSocket, tempProt);
			//Sprawdzanie czy odebrany protokół dotyczy przydzielenia Id
			if (!tempProt.compare(OP_DATA, DATA_ID, NULL)) {
				sync_cout << GetCurrentTimeTm() << " : " << "Session id not received.\n";
				sync_cerr << GetCurrentTimeTm() << " : " << "Session id not received.\n";
				return false;
			}
			else { sessionId = tempProt.getData_Int(); }

			sync_cout << GetCurrentTimeTm() << " : " << "Session id: " << sessionId << "\n";
			sync_cerr << GetCurrentTimeTm() << " : " << "Session id: " << sessionId << "\n";
			return true;
		}
		else { return false; }
	}

	void startGame() {
		BinProtocol tempProt;

		//Czekanie na uzyskanie informacji o czasie rozgrywki
		while (!tempProt.compare(OP_TIME, TIME_DURATION, sessionId)) {
			receiveBinProtocol(nodeSocket, tempProt);
			if (tempProt.compare(OP_MESSAGE, MESSAGE_SENDER_DISCONNECTED, NULL)) {
				sync_cerr << GetCurrentTimeTm() << " : " << "Server disconnected.\n";
				return;
			}
		}
		sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "Game duration: " << tempProt.getData_Int() << "s\n";
		sync_cout << GetCurrentTimeTm() << " : " << "Game duration: " << tempProt.getData_Int() << "s\n";
		sync_cout << GetCurrentTimeTm() << " : " << "You can only enter positive numbers lesser than 255.\n\n";
		//Czekanie na rozpoczęcie rozgrywki
		while (!tempProt.compare(OP_GAME, GAME_BEGIN, sessionId)) {
			receiveBinProtocol(nodeSocket, tempProt);
			if (tempProt.compare(OP_TIME, TIME_TO_START, sessionId)) {
				sync_cerr << GetCurrentTimeTm() << " : " << "Time to start: " << tempProt.getData_Int() << "s\n";
				sync_cout << GetCurrentTimeTm() << " : " << "Time to start: " << tempProt.getData_Int() << "s\n";
			}
			else if (tempProt.compare(OP_MESSAGE, MESSAGE_SENDER_DISCONNECTED, NULL)) {
				sync_cerr << GetCurrentTimeTm() << " : " << "Server disconnected.\n";
				return;
			}
			else if (tempProt.compare(OP_MESSAGE, MESSAGE_OPPONENT_DISCONNECTED, sessionId)) {
				sync_cerr << GetCurrentTimeTm() << " : " << "Opponent disconnected.\n";
				return;
			}
		}
		sync_cerr << GetCurrentTimeTm() << " : " << "Game start.\n\n";

		unsigned int number = 0;
		bool stop = false;
		std::thread listener(&ClientTCP::listenForServerMessages, this, std::ref(tempProt), std::ref(stop));

		//Pętla rozgrywki
		while (!stop) {
			//Podawanie liczby
			while (!stop) {
				std::string input;
				std::cin >> input;
				if (stop) { break; }

				//Dobre dane
				if (inputCheck(input)) {
					number = int(std::stoi(input));
					break;
				}
				else sync_cout << "Bad input. Enter number: ";
			}
			if (stop) { break; }

			sync_cerr << GetCurrentTimeTm() << " : " << "Sent number: " << number << '\n';
			this->sendBinProtocol(BinProtocol(OP_DATA, DATA_NUMBER, sessionId, number), nodeSocket);
		}
		listener.join();
	}
};
