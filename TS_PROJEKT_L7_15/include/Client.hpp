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
	bool badInput = false;
	std::string userInput;

	//Metody prywatne
	bool input_check(const std::string& input) const {
		if (input.find_first_not_of("0123456789") != std::string::npos) { return false; }
		else {
			inet_ntoa();
			const int numberCheck = std::stoi(input);
			//Czy dodatnia i mniejsza niż zakres
			if (numberCheck < 0 || numberCheck > int(pow(2, 8) - 1)) { return false; }
		}
		return true;
	}

	//Funkcja użyta do odbierania transmisji od serwera na osobnym wątku
	void listen_for_server_messages(BinProtocol& tempProt, bool& stop) const {
		//Pętla odbierania wiadomości
		//Zatrzymuje się gdy rozgrywka zostanie zakończona/przerwana
		while (!stop) {
			this->receive_bin_protocol(nodeSocket, tempProt);

			//Serwer rozłączony
			if (tempProt.compare(OP_MESSAGE, MESSAGE_SENDER_DISCONNECTED, NULL)) {
				CONSOLE_MANIP::cursor_set_pos(0, CONSOLE_MANIP::cursor_get_pos().Y - 2);
				sync_cout << GET_CURRENT_TIME() << " : " << "Server disconnected.       \n\n";
				sync_cerr << GET_CURRENT_TIME() << " : " << "Server disconnected.       \n";
				sync_cout << GET_CURRENT_TIME() << " : " << "Enter anything to exit: ";
				sync_cout << "                    ";
				CONSOLE_MANIP::cursor_move(-20, 0);
				stop = true;
			}

			//Przeciwnik rozłączony
			else if (tempProt.compare(OP_MESSAGE, MESSAGE_OPPONENT_DISCONNECTED, sessionId)) {
				CONSOLE_MANIP::cursor_set_pos(0, CONSOLE_MANIP::cursor_get_pos().Y - 2);
				sync_cout << GET_CURRENT_TIME() << " : " << "Opponent disconnected.       \n\n";
				sync_cerr << GET_CURRENT_TIME() << " : " << "Opponent disconnected.       \n";
				sync_cout << GET_CURRENT_TIME() << " : " << "Enter anything to exit: ";
				sync_cout << "                    ";
				CONSOLE_MANIP::cursor_move(-20, 0);
				stop = true;
			}

			//Czas do końca
			else if (tempProt.compare(OP_TIME, TIME_LEFT, sessionId)) {
				CONSOLE_MANIP::cursor_set_pos(0, CONSOLE_MANIP::cursor_get_pos().Y - 2);
				sync_cout << GET_CURRENT_TIME() << " : " << "TIME LEFT: " << tempProt.get_data_int() << "s       \n\n";
				sync_cerr << GET_CURRENT_TIME() << " : " << "TIME LEFT: " << tempProt.get_data_int() << "s       \n\n";
				sync_cout << GET_CURRENT_TIME() << " : " << (badInput ? "Bad input. " : "") <<  "Enter number: ";
				sync_cout << "                    ";
				CONSOLE_MANIP::cursor_move(-20, 0);
				sync_cout << userInput;
			}

			//Liczba za duża
			else if (tempProt.compare(OP_NUMBER, NUMBER_TOO_BIG, sessionId)) {
				CONSOLE_MANIP::cursor_set_pos(0, CONSOLE_MANIP::cursor_get_pos().Y - 1);
				sync_cout << GET_CURRENT_TIME() << " : " << "Number " << tempProt.get_data_int() << " is too big. Try again.       \n";
				sync_cerr << '\n' << GET_CURRENT_TIME() << " : " << "Number " << tempProt.get_data_int() << " is too big. Try again.       \n";
				sync_cout << GET_CURRENT_TIME() << " : " << "Enter number: ";
				sync_cout << "                    ";
				CONSOLE_MANIP::cursor_move(-20, 0);
			}

			//Liczba za mała
			else if (tempProt.compare(OP_NUMBER, NUMBER_TOO_SMALL, sessionId)) {
				CONSOLE_MANIP::cursor_set_pos(0, CONSOLE_MANIP::cursor_get_pos().Y - 1);
				sync_cout << GET_CURRENT_TIME() << " : " << "Number " << tempProt.get_data_int() << " is too small. Try again.       \n";
				sync_cerr << '\n' << GET_CURRENT_TIME() << " : " << "Number " << tempProt.get_data_int() << " is too small. Try again.       \n";
				sync_cout << GET_CURRENT_TIME() << " : " << "Enter number: ";
				sync_cout << "                    ";
				CONSOLE_MANIP::cursor_move(-20, 0);
			}

			//Remis
			else if (tempProt.compare(OP_GAME, GAME_DRAW, sessionId)) {
				CONSOLE_MANIP::cursor_set_pos(0, CONSOLE_MANIP::cursor_get_pos().Y - 2);
				sync_cout << GET_CURRENT_TIME() << " : " << "NO ONE WON THE GAME.       \n";
				sync_cerr << '\n' << GET_CURRENT_TIME() << " : " << "NO ONE WON THE GAME.       \n";
			}

			//Wygrana
			else if (tempProt.compare(OP_GAME, GAME_WON, sessionId)) {
				CONSOLE_MANIP::cursor_set_pos(0, CONSOLE_MANIP::cursor_get_pos().Y - 2);
				sync_cout << GET_CURRENT_TIME() << " : " << "YOU WON THE GAME.       \n";
				sync_cerr << '\n' << GET_CURRENT_TIME() << " : " << "YOU WON THE GAME.       \n";
			}

			//Przegrana
			else if (tempProt.compare(OP_GAME, GAME_LOST, sessionId)) {
				CONSOLE_MANIP::cursor_set_pos(0, CONSOLE_MANIP::cursor_get_pos().Y - 2);
				sync_cout << GET_CURRENT_TIME() << " : " << "YOU LOST THE GAME.       \n";
				sync_cerr << '\n' << GET_CURRENT_TIME() << " : " << "YOU LOST THE GAME.       \n";
			}

			//Koniec rozgrywki
			else if (tempProt.compare(OP_GAME, GAME_END, sessionId)) {
				sync_cout << GET_CURRENT_TIME() << " : " << "GAME END.                         \n";
				sync_cerr << GET_CURRENT_TIME() << " : " << "GAME END.                         \n";
				sync_cout << GET_CURRENT_TIME() << " : " << "Enter anything to exit: ";
				sync_cout << "                    ";
				CONSOLE_MANIP::cursor_move(-20, 0);
				stop = true;
			}
		}
	}

	//Rozłączanie się z serwerem

	//Konstruktory
public:
	ClientTCP(const unsigned long& address, const unsigned int& port) : NodeTCP(address, port) {}

	//Metody publiczne
	bool connect_server() {
		if (connect(nodeSocket, reinterpret_cast<SOCKADDR*>(&nodeInfo), sizeof(nodeInfo)) != SOCKET_ERROR) {
			BinProtocol tempProt;
			receive_bin_protocol(nodeSocket, tempProt);
			//Sprawdzanie czy odebrany protokół dotyczy przydzielenia Id
			if (!tempProt.compare(OP_DATA, DATA_ID, NULL)) {
				sync_cout << GET_CURRENT_TIME() << " : " << "Session id not received.\n";
				sync_cerr << GET_CURRENT_TIME() << " : " << "Session id not received.\n";
				return false;
			}
			else {
				sessionId = tempProt.get_data_int();
				sync_cout << GET_CURRENT_TIME() << " : " << "Session id: " << sessionId << "\n";
				sync_cerr << GET_CURRENT_TIME() << " : " << "Session id: " << sessionId << "\n";
				return true;
			}
		}
		else { return false; }
	}

	void start_game() {
		CONSOLE_MANIP::show_console_cursor(false);
		BinProtocol tempProt;

		//Czekanie na uzyskanie informacji o czasie rozgrywki
		while (!tempProt.compare(OP_TIME, TIME_DURATION, sessionId)) {
			receive_bin_protocol(nodeSocket, tempProt);

			//Serwer rozłączony
			if (tempProt.compare(OP_MESSAGE, MESSAGE_SENDER_DISCONNECTED, NULL)) {
				sync_cout << GET_CURRENT_TIME() << " : " << "Server disconnected.\n";
				sync_cerr << GET_CURRENT_TIME() << " : " << "Server disconnected.\n";
				return;
			}

			//Czekanie na przeciwnika
			else if (tempProt.compare(OP_MESSAGE, MESSAGE_WAITING_FOR_OPPONENT, sessionId)) {
				sync_cout << GET_CURRENT_TIME() << " : " << "Waiting for opponent.\n\n";
				sync_cerr << GET_CURRENT_TIME() << " : " << "Waiting for opponent.\n\n";
			}
		}

		//Wyświetlenie informacji o zasadach rozgrywki
		{
			sync_cout << GET_CURRENT_TIME() << " : " << "---- GAME RULES ----\n";
			sync_cout << GET_CURRENT_TIME() << " : " << "YOU HAVE 30 SECONDS BEFORE GAME STARTS.\n";
			sync_cout << GET_CURRENT_TIME() << " : " << "EACH PLAYER TRIES TO GUESS RANDOM NUMBER(0-255) GENERATED BY SERVER.\n";
			sync_cout << GET_CURRENT_TIME() << " : " << "GAME STOPS WHEN:\n";
			sync_cout << GET_CURRENT_TIME() << " : " << " - TIME RUNS OUT,\n";
			sync_cout << GET_CURRENT_TIME() << " : " << " - ONE OF THE PLAYERS WINS,\n";
			sync_cout << GET_CURRENT_TIME() << " : " << " - ONE OF THE PLAYERS DISCONNECTS,\n";
			sync_cout << GET_CURRENT_TIME() << " : " << " - SERVER DISCONNECTS.\n";
			sync_cout << GET_CURRENT_TIME() << " : " << "YOU CAN ENTER ONLY POSITIVE NUMBERS LESSER THAN 256.\n";
			sync_cout << GET_CURRENT_TIME() << " : " << "CONSOLE INPUT IS LIMITED TO 3 DIGITS.\n\n";
		}

		//Wyświetlenie informacji o czasie trwania rozgrywki
		gameDuration = tempProt.get_data_int();
		sync_cerr << '\n' << GET_CURRENT_TIME() << " : " << "GAME DURATION: " << gameDuration << "s\n\n";
		sync_cout << GET_CURRENT_TIME() << " : " << "GAME DURATION: " << tempProt.get_data_int() << "s\n\n";

		//Czekanie na rozpoczęcie rozgrywki
		while (!tempProt.compare(OP_GAME, GAME_BEGIN, sessionId)) {
			receive_bin_protocol(nodeSocket, tempProt);

			//Czas do rozpoczęcia rozgrywki
			if (tempProt.compare(OP_TIME, TIME_TO_START, sessionId)) {
				CONSOLE_MANIP::cursor_set_pos(0, CONSOLE_MANIP::cursor_get_pos().Y);
				sync_cerr << GET_CURRENT_TIME() << " : " << "Time to start: " << tempProt.get_data_int() << "s   ";
				sync_cout << GET_CURRENT_TIME() << " : " << "Time to start: " << tempProt.get_data_int() << "s   ";
			}

			//Serwer rozłączony
			else if (tempProt.compare(OP_MESSAGE, MESSAGE_SENDER_DISCONNECTED, NULL)) {
				CONSOLE_MANIP::cursor_set_pos(0, CONSOLE_MANIP::cursor_get_pos().Y);
				sync_cout << GET_CURRENT_TIME() << " : " << "Server disconnected.                    \n";
				sync_cerr << '\n' << GET_CURRENT_TIME() << " : " << "Server disconnected.\n";
				return;
			}

			//Przeciwnik rozłączony
			else if (tempProt.compare(OP_MESSAGE, MESSAGE_OPPONENT_DISCONNECTED, sessionId)) {
				CONSOLE_MANIP::cursor_set_pos(0, CONSOLE_MANIP::cursor_get_pos().Y);
				sync_cout << GET_CURRENT_TIME() << " : " << "Opponent disconnected.                    \n";
				sync_cerr << '\n' << GET_CURRENT_TIME() << " : " << "Opponent disconnected.\n";
				return;
			}
		}
		CONSOLE_MANIP::cursor_set_pos(0, CONSOLE_MANIP::cursor_get_pos().Y);
		sync_cout << GET_CURRENT_TIME() << " : " << "GAME START.      \n\n";
		sync_cerr << GET_CURRENT_TIME() << " : " << "GAME START.      \n\n";

		//Uruchomienie nasłuchiwania wiadomości od serwera
		bool stop = false;
		std::thread listener(&ClientTCP::listen_for_server_messages, this, std::ref(tempProt), std::ref(stop));

		//Pętla rozgrywki
		CONSOLE_MANIP::show_console_cursor(true);
		unsigned int number = 0;
		while (!stop) {
			//Podawanie liczby
			while (!stop) {
				//Ograniczone wprowadzanie danych
				CONSOLE_MANIP::input_string_digits(userInput, 3);
				badInput = false;
				if (stop) { break; }

				//Sprawdzanie czy wprowadzono właściwe dane
				if (input_check(userInput)) {
					number = int(std::stoi(userInput));
					userInput.clear();
					break;
				}
				else {
					badInput = true;
					CONSOLE_MANIP::cursor_set_pos(0, CONSOLE_MANIP::cursor_get_pos().Y);
					sync_cout << GET_CURRENT_TIME() << " : " << "Bad input. Enter number: ";
					userInput.clear();
				}
			}
			if (stop) { break; }

			//Wysłanie liczby
			sync_cerr << GET_CURRENT_TIME() << " : " << "Sent number: " << number << '\n';
			this->send_bin_protocol(BinProtocol(OP_DATA, DATA_NUMBER, sessionId, number), nodeSocket);
		}
		CONSOLE_MANIP::show_console_cursor(false);

		//Zakończenie rozgrywki, rozłączenie z serwerem
		listener.join();
		closesocket(nodeSocket);
		WSACleanup();
		sync_cout << '\n' << GET_CURRENT_TIME() << " : " << "Connection closed.\n";
	}
};
