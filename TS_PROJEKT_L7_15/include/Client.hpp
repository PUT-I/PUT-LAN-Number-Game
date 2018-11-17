#pragma once
#include "Node.hpp"
#include "Console.hpp"

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable:4996) 

class ClientTCP : public NodeTCP {
private:
	//Id sesji klienta
	unsigned sessionId = 0;
	unsigned int gameDuration = 0;
	std::string userInput;

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

	void listenForServerMessages(BinProtocol& tempProt, bool& stop) const {
		while (!stop) {
			this->receiveBinProtocol(nodeSocket, tempProt);
			if (tempProt.compare(OP_MESSAGE, MESSAGE_SENDER_DISCONNECTED, NULL)) {
				if (tempProt.getData_Int() != gameDuration) { sync_cout << " INTERRUPTED"; }
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Server disconnected.\n";
				sync_cerr << GetCurrentTimeTm() << " : " << "Server disconnected.\n";
				sync_cout << GetCurrentTimeTm() << " : " << "Enter anything to exit: ";
				stop = true;
			}
			else if (tempProt.compare(OP_MESSAGE, MESSAGE_OPPONENT_DISCONNECTED, sessionId)) {
				if (tempProt.getData_Int() != gameDuration) { sync_cout << " INTERRUPTED"; }
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
				if (tempProt.getData_Int() != gameDuration) { sync_cout << " INTERRUPTED"; }
				std::cout << '\n' << GetCurrentTimeTm() << " : " << "TIME LEFT: " << tempProt.getData_Int() << "s\n\n";
				sync_cerr << GetCurrentTimeTm() << " : " << "TIME LEFT: " << tempProt.getData_Int() << "s\n\n";
				sync_cout << GetCurrentTimeTm() << " : " << "Enter number: " << userInput;
			}
			else if (tempProt.compare(OP_NUMBER, NUMBER_TOO_BIG, sessionId)) {
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Number " << tempProt.getData_Int() << " is too big. Try again.\n";
				sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "Number " << tempProt.getData_Int() << " is too big. Try again.\n";
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Enter number: ";
			}
			else if (tempProt.compare(OP_NUMBER, NUMBER_TOO_SMALL, sessionId)) {
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Number " << tempProt.getData_Int() << " is too small. Try again.\n";
				sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "Number " << tempProt.getData_Int() << " is too small. Try again.\n";
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Enter number: ";
			}
			else if (tempProt.compare(OP_GAME, GAME_DRAW, sessionId)) {
				if (tempProt.getData_Int() != gameDuration) { sync_cout << " INTERRUPTED"; }
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "NO ONE WON THE GAME.\n";
				sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "NO ONE WON THE GAME.\n";
			}
			else if (tempProt.compare(OP_GAME, GAME_WON, sessionId)) {
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "YOU WON THE GAME.\n";
				sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "YOU WON THE GAME.\n";
			}
			else if (tempProt.compare(OP_GAME, GAME_LOST, sessionId)) {
				if (tempProt.getData_Int() != gameDuration) { sync_cout << " INTERRUPTED"; }
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "YOU LOST THE GAME.\n";
				sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "YOU LOST THE GAME.\n";
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
			else {
				sessionId = tempProt.getData_Int();
				sync_cout << GetCurrentTimeTm() << " : " << "Session id: " << sessionId << "\n";
				sync_cerr << GetCurrentTimeTm() << " : " << "Session id: " << sessionId << "\n";
				return true;
			}
		}
		else { return false; }
	}

	void startGame() {
		BinProtocol tempProt;

		//Czekanie na uzyskanie informacji o czasie rozgrywki
		while (!tempProt.compare(OP_TIME, TIME_DURATION, sessionId)) {
			receiveBinProtocol(nodeSocket, tempProt);
			if (tempProt.compare(OP_MESSAGE, MESSAGE_SENDER_DISCONNECTED, NULL)) {
				sync_cout << GetCurrentTimeTm() << " : " << "Server disconnected.\n";
				sync_cerr << GetCurrentTimeTm() << " : " << "Server disconnected.\n";
				return;
			}
			else if (tempProt.compare(OP_MESSAGE,MESSAGE_WAITING_FOR_OPPONENT,sessionId)){
				sync_cout << GetCurrentTimeTm() << " : " << "Waiting for opponent.\n\n";
				sync_cerr << GetCurrentTimeTm() << " : " << "Waiting for opponent.\n\n";
			}
		}
		gameDuration = tempProt.getData_Int();
		sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "Game duration: " << gameDuration << "s\n";
		sync_cout << GetCurrentTimeTm() << " : " << "Game duration: " << tempProt.getData_Int() << "s\n";
		sync_cout << GetCurrentTimeTm() << " : " << "You can only enter positive numbers lesser than 256.\n\n";

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
				CONSOLE_MANIP::INPUT_STRING(userInput, 3);
				if (stop) { break; }

				//Dobre dane
				if (inputCheck(userInput)) {
					number = int(std::stoi(userInput));
					userInput.clear();
					break;
				}
				else {
					sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Bad input. Enter number: ";
					userInput.clear();
				}
			}
			if (stop) { break; }

			sync_cerr << GetCurrentTimeTm() << " : " << "Sent number: " << number << '\n';
			this->sendBinProtocol(BinProtocol(OP_DATA, DATA_NUMBER, sessionId, number), nodeSocket);
		}
		listener.join();
		shutdown(nodeSocket, 2);
	}
};
