#pragma once

#include "Node.hpp"

#include <thread>
#include <random>
#include <chrono>
#include <unordered_map>

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
	static const unsigned int timeToStart = 30;
	//Tablica gniazdek po��czonych klient�w (klucz to id sesji klienta)
	std::unordered_map<unsigned int, SOCKET>clientSockets;
	//Tablica id sesji poszczeg�lnych klient�w liczonych od 0
	std::vector<unsigned int> sessionIds;

	//Metody prywatne
	bool socket_bind() {
		if (bind(nodeSocket, reinterpret_cast<SOCKADDR*>(&nodeInfo), sizeof(nodeInfo)) == SOCKET_ERROR) {
			closesocket(nodeSocket);
			sync_cerr << GET_CURRENT_TIME() << " : " << "Socket binding failed with error code : " << WSAGetLastError << '\n';
			return false;
		}
		return true;
	}

	//Konstruktory
public:
	ServerTCP(const unsigned long& address, const unsigned int& port) : NodeTCP(address, port) {
		if (!socket_bind()) { exit(0); }

		if (listen(nodeSocket, 2) == SOCKET_ERROR) {
			sync_cerr << GET_CURRENT_TIME() << " : " << "Error listening on socket.\n";
		}
	}
	virtual ~ServerTCP() {
		//Zamykanie gniazdek klient�w
		for (const auto& elem : clientSockets) {
			closesocket(elem.second);
		}
		clientSockets.clear();
	}

	//Metody publiczne
	bool accept_client() {
		this->send_bin_protocol_to_all(BinProtocol(OP_MESSAGE, MESSAGE_WAITING_FOR_OPPONENT, NULL, NULL));
		SOCKET clientSocket = accept(this->nodeSocket, nullptr, nullptr);

		if (clientSocket == SOCKET_ERROR) { return false; }

		//Szukanie wolnego identyfikatora sesji dla klienta (zwr�cenie prawdy je�li znaleziony)
		for (unsigned int i = 0; i < pow(2, 5) - 1; i++) {
			mutex.lock();
			const unsigned int sessionId = randInt(1, int(pow(2, 5) - 1));
			//Je�li identyfikator nie jest zaj�ty to przypisanie klientowi
			if (clientSockets.find(sessionId) == clientSockets.end()) {
				sync_cout << GET_CURRENT_TIME() << " : " << "Session id: " << sessionId << "\n";
				clientSockets[sessionId] = clientSocket;
				sessionIds.push_back(sessionId);
				send_bin_protocol(BinProtocol(OP_DATA, DATA_ID, NULL, sessionId), clientSocket);
				mutex.unlock();
				return true;
			}
			mutex.unlock();
		}
		//Zwr�cenia fa�szu je�li nie znaleziony wolny identyfikator sesji
		return false;
	}

	void send_bin_protocol_to_all(BinProtocol data) {
		for (const unsigned int id : sessionIds) {
			data.set_id(id);
			sync_cerr << '\n' << GET_CURRENT_TIME() << " : " << "Sent to session " << id << "\n";
			this->send_bin_protocol(data, clientSockets[id]);
		}
	}

	//Funkcja u�yta do odbierania transmisji od klient�w na wielu w�tkach
	void receive_bin_protocols(BinProtocol& output, SOCKET& clientSocket, bool& stop) {
		char* recvBuf = new char[BUF_LENGTH];

		//P�tla odbierania wiadomo�ci
		//Zatrzymuje si� gdy rozgrywka zostanie zako�czona/przerwana
		while (!stop) {
			const int bytesRecv = recv(clientSocket, recvBuf, BUF_LENGTH, 0);
			if (bytesRecv <= 0 || bytesRecv == WSAECONNRESET) {
				for (unsigned int i = 0; i < sessionIds.size(); i++) {
					if (clientSockets[sessionIds[i]] == clientSocket) {
						sync_cout << GET_CURRENT_TIME() << " : " << "Client " << i + 1 << " (id " << std::setfill('0') << std::setw(2) << sessionIds[i] << ") disconnected.\n";
						sync_cerr << GET_CURRENT_TIME() << " : " << "Client " << i + 1 << " (id " << std::setfill('0') << std::setw(2) << sessionIds[i] << ") disconnected.\n";
						sessionIds.erase(sessionIds.begin() + i);
					}
				}
				stop = true;
				output = BinProtocol();
				return;
			}

			output.from_char_a(recvBuf);
			sync_cerr << '\n' << GET_CURRENT_TIME() << " : " << "Bytes received: " << bytesRecv << "\n";
			sync_cerr << GET_CURRENT_TIME() << " : " << "Received protocol: " << output << '\n';
			sync_cerr << GET_CURRENT_TIME() << " : " << "Received bits: ";
			for (unsigned int i = 0; i < BUF_LENGTH; i++) { sync_cerr << std::bitset<8>(recvBuf[i]) << (i < BUF_LENGTH - 1 ? " " : ""); }
			sync_cerr << '\n';
		}
	}

	//Rozgrywka
	void start_game() {
		//Obliczenie czasu rozgrywki
		const unsigned int gameDuration = (abs(int(sessionIds[0] - sessionIds[1])) * 74) % 90 + 25;

		sync_cerr << '\n' << GET_CURRENT_TIME() << " : " << "GAME DURATION: " << gameDuration << "s\n";
		sync_cout << '\n' << GET_CURRENT_TIME() << " : " << "GAME DURATION: " << gameDuration << "s\n";
		this->send_bin_protocol_to_all(BinProtocol(OP_TIME, TIME_DURATION, NULL, gameDuration));

		//Zmienne do zarz�dzania w�tkami
		bool stop = false; //Zmiena gdy prawda przerywa odbieranie danych
		std::vector<BinProtocol> inputs(clientSockets.size());
		std::vector<std::thread> threads(clientSockets.size());

		//Uruchamianie w�tk�w odbierania
		{
			threads[0] = std::thread([this, &inputs, &stop] {this->receive_bin_protocols(inputs[0], clientSockets[sessionIds[0]], stop); });
			threads[1] = std::thread([this, &inputs, &stop] {this->receive_bin_protocols(inputs[1], clientSockets[sessionIds[1]], stop); });
		}

		//Wygenerowanie losowej liczby
		const unsigned int secretNumber = randInt(0, int(pow(2, 8)) - 1);
		sync_cout << '\n' << GET_CURRENT_TIME() << " : " << "SECRET NUMBER: " << secretNumber << "\n\n";
		sync_cerr << '\n' << GET_CURRENT_TIME() << " : " << "SECRET NUMBER: " << secretNumber << "\n\n";

		//Odliczanie do pocz�tku rozgrywki
		{
			for (unsigned int i = timeToStart; i > 0; i--) {
				if (stop) { break; }
				sync_cerr << GET_CURRENT_TIME() << " : " << "Time to start: " << i << "s\n";
				sync_cout << GET_CURRENT_TIME() << " : " << "Time to start: " << i << "s\n";
				this->send_bin_protocol_to_all(BinProtocol(OP_TIME, TIME_TO_START, NULL, i));
				Sleep(1000);
			}
			sync_cout << '\n';
			sync_cerr << '\n';
		}

		//Rozpoczynanie rozgrywki
		if (!stop) {
			//Wys�anie wiadomo�ci o starcie rozgrywki
			sync_cerr << GET_CURRENT_TIME() << " : " << "Game start.\n";
			sync_cerr << '\n' << GET_CURRENT_TIME() << " : " << "Send game start info to players.\n";
			this->send_bin_protocol_to_all(BinProtocol(OP_GAME, GAME_BEGIN, NULL, NULL));

			//Wys�anie wiadomo�ci o czasie
			sync_cerr << '\n' << GET_CURRENT_TIME() << " : " << "Send time to players.\n";
			sync_cerr << GET_CURRENT_TIME() << " : " << "Time left: " << gameDuration << "s\n";
			sync_cout << GET_CURRENT_TIME() << " : " << "Time left: " << gameDuration << "s\n";
			this->send_bin_protocol_to_all(BinProtocol(OP_TIME, TIME_LEFT, NULL, gameDuration));
		}

		//Zmienne do zarz�dzania rozgrywk�
		std::bitset<2> wins;

		//Rozgrywka
		if (!stop) {
			//Zmienne do informowania o pozosta�ym czasie
			const auto timeStart = std::chrono::system_clock::now();		  //Czas rozpocz�cia
			const auto timeEnd = std::chrono::duration<double>(gameDuration); //Czas rozgrywki
			auto timeMessageStart = std::chrono::system_clock::now();		  //Czas rozpocz�cia od wiadomo�ci
			auto timeMessage = std::chrono::duration<double>(0);			  //Czas u�ywany przy wiadomo�ci

			for (auto time = std::chrono::duration<double>(0); time < timeEnd; time = std::chrono::system_clock::now() - timeStart) {
				timeMessage = std::chrono::system_clock::now() - timeMessageStart;

				if (stop) {
					break;
				}
				if (timeMessage >= std::chrono::duration<double>(15)) {
					sync_cerr << '\n' << GET_CURRENT_TIME() << " : " << "Send time to players.\n";
					sync_cerr << GET_CURRENT_TIME() << " : " << "Time left: " << unsigned int(ceil((timeEnd - time).count())) << "s\n";
					sync_cout << GET_CURRENT_TIME() << " : " << "Time left: " << unsigned int(ceil((timeEnd - time).count())) << "s\n";
					//Wys�anie wiadomo�ci o czasie
					this->send_bin_protocol_to_all(BinProtocol(OP_TIME, TIME_LEFT, NULL, unsigned int(ceil((timeEnd - time).count()))));
					//Koniec wysy�ania wiadomo�ci o czasie
					timeMessage = std::chrono::duration<double>(0);
					timeMessageStart = std::chrono::system_clock::now();
				}

				for (unsigned int i = 0; i < sessionIds.size(); i++) {
					if (inputs[i].compare(OP_DATA, DATA_NUMBER, sessionIds[i])) {
						const unsigned int tempNumber = inputs[i].get_data_int();
						inputs[i] = BinProtocol();
						sync_cout << GET_CURRENT_TIME() << " : " << "Received number " << tempNumber << " from session " << sessionIds[i] << "\n";
						sync_cerr << GET_CURRENT_TIME() << " : " << "Received number " << tempNumber << " from session " << sessionIds[i] << "\n";

						if (tempNumber > secretNumber) {
							sync_cout << GET_CURRENT_TIME() << " : " << "Sent message 'NUMBER TOO BIG' to session " << sessionIds[i] << "\n";
							send_bin_protocol(BinProtocol(OP_NUMBER, NUMBER_TOO_BIG, sessionIds[i], tempNumber), clientSockets[sessionIds[i]]);
						}
						else if (tempNumber == secretNumber) {
							wins[i] = true;
						}
						else if (tempNumber < secretNumber) {
							sync_cout << GET_CURRENT_TIME() << " : " << "Sent message 'NUMBER TOO SMALL' to session " << sessionIds[i] << "\n";
							send_bin_protocol(BinProtocol(OP_NUMBER, NUMBER_TOO_SMALL, sessionIds[i], tempNumber), clientSockets[sessionIds[i]]);
						}
					}
				}
				if (wins[0] == true || wins[1] == true) { break; }
			}
		}

		//Zako�czenie rozgrywki
		sync_cout << '\n' << GET_CURRENT_TIME() << " : " << "Game end.\n";
		sync_cerr << '\n' << GET_CURRENT_TIME() << " : " << "Game end.\n";
		if (!stop) {
			if (wins[0] == false && wins[1] == false) {
				sync_cout << GET_CURRENT_TIME() << " : " << "Sent message 'GAME DRAW' to all players\n";
				send_bin_protocol_to_all(BinProtocol(OP_GAME, GAME_DRAW, NULL, NULL));
			}
			else if (wins[0] == true) {
				sync_cout << GET_CURRENT_TIME() << " : " << "Sent message 'GAME WON' to session " << sessionIds[0] << "\n";
				send_bin_protocol(BinProtocol(OP_GAME, GAME_WON, sessionIds[0], NULL), clientSockets[sessionIds[0]]);

				sync_cout << GET_CURRENT_TIME() << " : " << "Sent message 'GAME LOST' to session " << sessionIds[1] << "\n";
				send_bin_protocol(BinProtocol(OP_GAME, GAME_LOST, sessionIds[1], NULL), clientSockets[sessionIds[1]]);
			}
			else if (wins[1] == true) {
				sync_cout << GET_CURRENT_TIME() << " : " << "Sent message 'GAME WON' to session " << sessionIds[1] << "\n";
				send_bin_protocol(BinProtocol(OP_GAME, GAME_WON, sessionIds[1], NULL), clientSockets[sessionIds[1]]);

				sync_cout << GET_CURRENT_TIME() << " : " << "Sent message 'GAME LOST' to session " << sessionIds[0] << "\n";
				send_bin_protocol(BinProtocol(OP_GAME, GAME_LOST, sessionIds[0], NULL), clientSockets[sessionIds[0]]);
			}

			stop = true;
			this->send_bin_protocol_to_all(BinProtocol(OP_GAME, GAME_END, NULL, NULL));
		}
		else {
			this->send_bin_protocol_to_all(BinProtocol(OP_MESSAGE, MESSAGE_OPPONENT_DISCONNECTED, NULL, NULL));
		}

		//��czenie w�tk�w odbierania
		sync_cout << GET_CURRENT_TIME() << " : " << "Waiting for clients to disconnect.\n";
		for (std::thread& thread : threads) { thread.join(); }
	}
};
