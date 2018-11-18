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
	bool inputCheck(const std::string& input) const {
		if (input.find_first_not_of("0123456789") != std::string::npos) { return false; }
		else {
			const int numberCheck = std::stoi(input);
			//Czy dodatnia i mniejsza niż zakres
			if (numberCheck < 0 || numberCheck > int(pow(2, 8) - 1)) { return false; }
		}
		return true;
	}

	//Funkcja użyta do odbierania transmisji od serwera na osobnym wątku
	void listenForServerMessages(BinProtocol& tempProt, bool& stop) const {
		//Pętla odbierania wiadomości
		//Zatrzymuje się gdy rozgrywka zostanie zakończona/przerwana
		while (!stop) {
			this->receiveBinProtocol(nodeSocket, tempProt);

			//Serwer rozłączony
			if (tempProt.compare(OP_MESSAGE, MESSAGE_SENDER_DISCONNECTED, NULL)) {
				if (tempProt.getData_Int() != gameDuration) { sync_cout << " INTERRUPTED"; }
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Server disconnected.\n";
				sync_cerr << GetCurrentTimeTm() << " : " << "Server disconnected.\n";
				sync_cout << GetCurrentTimeTm() << " : " << "Enter anything to exit: ";
				stop = true;
			}

			//Przeciwnik rozłączony
			else if (tempProt.compare(OP_MESSAGE, MESSAGE_OPPONENT_DISCONNECTED, sessionId)) {
				if (tempProt.getData_Int() != gameDuration) { sync_cout << " INTERRUPTED"; }
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Opponent disconnected.\n";
				sync_cerr << GetCurrentTimeTm() << " : " << "Opponent disconnected.\n";
				sync_cout << GetCurrentTimeTm() << " : " << "Enter anything to exit: ";
				stop = true;
			}

			//Koniec rozgrywki
			else if (tempProt.compare(OP_GAME, GAME_END, sessionId)) {
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Game end.\n";
				sync_cerr << GetCurrentTimeTm() << " : " << "Game end.\n";
				sync_cout << GetCurrentTimeTm() << " : " << "Enter anything to exit: ";
				stop = true;
			}

			//Czas do końca
			else if (tempProt.compare(OP_TIME, TIME_LEFT, sessionId)) {
				if (tempProt.getData_Int() != gameDuration) { sync_cout << " INTERRUPTED"; }
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "TIME LEFT: " << tempProt.getData_Int() << "s\n\n";
				sync_cerr << GetCurrentTimeTm() << " : " << "TIME LEFT: " << tempProt.getData_Int() << "s\n\n";
				sync_cout << GetCurrentTimeTm() << " : " << "Enter number: " << userInput;
			}

			//Liczba za duża
			else if (tempProt.compare(OP_NUMBER, NUMBER_TOO_BIG, sessionId)) {
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Number " << tempProt.getData_Int() << " is too big. Try again.\n";
				sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "Number " << tempProt.getData_Int() << " is too big. Try again.\n";
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Enter number: ";
			}

			//Liczba za mała
			else if (tempProt.compare(OP_NUMBER, NUMBER_TOO_SMALL, sessionId)) {
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Number " << tempProt.getData_Int() << " is too small. Try again.\n";
				sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "Number " << tempProt.getData_Int() << " is too small. Try again.\n";
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Enter number: ";
			}

			//Remis
			else if (tempProt.compare(OP_GAME, GAME_DRAW, sessionId)) {
				if (tempProt.getData_Int() != gameDuration) { sync_cout << " INTERRUPTED"; }
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "NO ONE WON THE GAME.\n";
				sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "NO ONE WON THE GAME.\n";
			}

			//Wygrana
			else if (tempProt.compare(OP_GAME, GAME_WON, sessionId)) {
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "YOU WON THE GAME.\n";
				sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "YOU WON THE GAME.\n";
			}

			//Przegrana
			else if (tempProt.compare(OP_GAME, GAME_LOST, sessionId)) {
				if (tempProt.getData_Int() != gameDuration) { sync_cout << " INTERRUPTED"; }
				sync_cout << '\n' << GetCurrentTimeTm() << " : " << "YOU LOST THE GAME.\n";
				sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "YOU LOST THE GAME.\n";
			}
		}
	}

	//Konstruktory
public:
	ClientTCP(const unsigned long& address, const unsigned int& port) : NodeTCP(address, port) {}

	//Metody publiczne
	bool connectServer() {
		if (connect(nodeSocket, reinterpret_cast<SOCKADDR*>(&nodeInfo), sizeof(nodeInfo)) != SOCKET_ERROR) {
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

			//Serwer rozłączony
			if (tempProt.compare(OP_MESSAGE, MESSAGE_SENDER_DISCONNECTED, NULL)) {
				sync_cout << GetCurrentTimeTm() << " : " << "Server disconnected.\n";
				sync_cerr << GetCurrentTimeTm() << " : " << "Server disconnected.\n";
				return;
			}

			//Czekanie na przeciwnika
			else if (tempProt.compare(OP_MESSAGE, MESSAGE_WAITING_FOR_OPPONENT, sessionId)) {
				sync_cout << GetCurrentTimeTm() << " : " << "Waiting for opponent.\n\n";
				sync_cerr << GetCurrentTimeTm() << " : " << "Waiting for opponent.\n\n";
			}
		}
		
		//Wyświetlenie informacji o zasadach rozgrywki
		sync_cout << GetCurrentTimeTm() << " : " << "---- GAME RULES ----\n";
		sync_cout << GetCurrentTimeTm() << " : " << "EACH PLAYER TRIES TO GUESS RANDOM NUMBER GENERATED BY SERVER.\n";
		sync_cout << GetCurrentTimeTm() << " : " << "GAME STOPS WHEN:\n";
		sync_cout << GetCurrentTimeTm() << " : " << " - TIME RUNS OUT\n";
		sync_cout << GetCurrentTimeTm() << " : " << " - ONE OF THE PLAYERS WINS\n";
		sync_cout << GetCurrentTimeTm() << " : " << " - ONE OF THE PLAYERS DISCONNECTS\n";
		sync_cout << GetCurrentTimeTm() << " : " << " - SERVER DISCONNECTS\n";
		sync_cout << GetCurrentTimeTm() << " : " << "YOU CAN ENTER ONLY POSITIVE NUMBERS LESSER THAN 256.\n";
		sync_cout << GetCurrentTimeTm() << " : " << "CONSOLE INPUT IS LIMITED TO 3 DIGITS.\n\n";

		//Wyświetlenie informacji o czasie trwania rozgrywki
		gameDuration = tempProt.getData_Int();
		sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "GAME DURATION: " << gameDuration << "s\n\n";
		sync_cout << GetCurrentTimeTm() << " : " << "GAME DURATION: " << tempProt.getData_Int() << "s\n\n";

		//Czekanie na rozpoczęcie rozgrywki
		while (!tempProt.compare(OP_GAME, GAME_BEGIN, sessionId)) {
			receiveBinProtocol(nodeSocket, tempProt);

			//Czas do rozpoczęcia rozgrywki
			if (tempProt.compare(OP_TIME, TIME_TO_START, sessionId)) {
				sync_cerr << GetCurrentTimeTm() << " : " << "Time to start: " << tempProt.getData_Int() << "s\n";
				sync_cout << GetCurrentTimeTm() << " : " << "Time to start: " << tempProt.getData_Int() << "s\n";
			}

			//Serwer rozłączony
			else if (tempProt.compare(OP_MESSAGE, MESSAGE_SENDER_DISCONNECTED, NULL)) {
				sync_cout << GetCurrentTimeTm() << " : " << "Server disconnected.\n";
				sync_cerr << GetCurrentTimeTm() << " : " << "Server disconnected.\n";
				return;
			}

			//Przeciwnik rozłączony
			else if (tempProt.compare(OP_MESSAGE, MESSAGE_OPPONENT_DISCONNECTED, sessionId)) {
				sync_cout << GetCurrentTimeTm() << " : " << "Opponent disconnected.\n";
				sync_cerr << GetCurrentTimeTm() << " : " << "Opponent disconnected.\n";
				return;
			}
		}
		sync_cerr << GetCurrentTimeTm() << " : " << "Game start.\n\n";

		//Uruchomienie nasłuchiwania wiadomości od serwera
		bool stop = false;
		std::thread listener(&ClientTCP::listenForServerMessages, this, std::ref(tempProt), std::ref(stop));

		//Pętla rozgrywki
		unsigned int number = 0;
		while (!stop) {
			//Podawanie liczby
			while (!stop) {
				//Ograniczone wprowadzanie danych
				CONSOLE_MANIP::input_string(userInput, 3);
				if (stop) { break; }

				//Sprawdzanie czy wprowadzono właściwe dane
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

			//Wysłanie liczby
			sync_cerr << GetCurrentTimeTm() << " : " << "Sent number: " << number << '\n';
			this->sendBinProtocol(BinProtocol(OP_DATA, DATA_NUMBER, sessionId, number), nodeSocket);
		}

		//Zakończenie rozgrywki, rozłączenie z serwerem
		listener.join();
		shutdown(nodeSocket, 2);
	}
};
