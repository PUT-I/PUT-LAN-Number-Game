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
	//Tablica gniazdek po³¹czonych klientów (klucz to id sesji klienta)
	std::unordered_map<unsigned int, SOCKET>clientSockets;
	//Tablica id sesji poszczególnych klientów liczonych od 0
	std::vector<unsigned int> sessionIds;

	//Metody prywatne
	bool socketBind() {
		if (bind(nodeSocket, reinterpret_cast<SOCKADDR*>(&nodeInfo), sizeof(nodeInfo)) == SOCKET_ERROR) {
			closesocket(nodeSocket);
			sync_cerr << GetCurrentTimeTm() << " : " << "Socket binding failed with error code : " << WSAGetLastError << '\n';
			return false;
		}
		return true;
	}

	//Konstruktory
public:
	ServerTCP(const unsigned long& address, const unsigned int& port) : NodeTCP(address, port) {
		if (!socketBind()) { exit(0); }

		if (listen(nodeSocket, 2) == SOCKET_ERROR) {
			sync_cerr << GetCurrentTimeTm() << " : " << "Error listening on socket.\n";
		}
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
		this->sendBinProtocolToAll(BinProtocol(OP_MESSAGE, MESSAGE_WAITING_FOR_OPPONENT, NULL, NULL));
		SOCKET clientSocket = accept(this->nodeSocket, nullptr, nullptr);

		if (clientSocket == SOCKET_ERROR) { return false; }

		//Szukanie wolnego identyfikatora sesji dla klienta (zwrócenie prawdy jeœli znaleziony)
		for (unsigned int i = 0; i < pow(2, 5) - 1; i++) {
			mutex.lock();
			const unsigned int sessionId = randInt(1, int(pow(2, 5) - 1));
			//Jeœli identyfikator nie jest zajêty to przypisanie klientowi
			if (clientSockets.find(sessionId) == clientSockets.end()) {
				sync_cout << GetCurrentTimeTm() << " : " << "Session id: " << sessionId << "\n";
				clientSockets[sessionId] = clientSocket;
				sessionIds.push_back(sessionId);
				sendBinProtocol(BinProtocol(OP_DATA, DATA_ID, NULL, sessionId), clientSocket);
				mutex.unlock();
				return true;
			}
			mutex.unlock();
		}
		//Zwrócenia fa³szu jeœli nie znaleziony wolny identyfikator sesji
		return false;
	}

	void sendBinProtocolToAll(BinProtocol data) {
		for (const unsigned int id : sessionIds) {
			data.setId(id);
			sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "Sent to session " << id << "\n";
			this->sendBinProtocol(data, clientSockets[id]);
		}
	}

	//Funkcja u¿yta do odbierania transmisji od klientów na wielu w¹tkach
	void receiveBinProtocols(BinProtocol& output, SOCKET& clientSocket, bool& stop) {
		char* recvBuf = new char[BUF_LENGTH];

		//Pêtla odbierania wiadomoœci
		//Zatrzymuje siê gdy rozgrywka zostanie zakoñczona/przerwana
		while (!stop) {
			const int bytesRecv = recv(clientSocket, recvBuf, BUF_LENGTH, 0);
			if (bytesRecv <= 0 || bytesRecv == WSAECONNRESET) {
				for (unsigned int i = 0; i < sessionIds.size(); i++) {
					if (clientSockets[sessionIds[i]] == clientSocket) {
						sync_cout << GetCurrentTimeTm() << " : " << "Client " << i + 1 << " (id " << std::setfill('0') << std::setw(2) << sessionIds[i] << ") disconnected.\n";
						sync_cerr << GetCurrentTimeTm() << " : " << "Client " << i + 1 << " (id " << std::setfill('0') << std::setw(2) << sessionIds[i] << ") disconnected.\n";
						sessionIds.erase(sessionIds.begin() + i);
					}
				}
				stop = true;
				output = BinProtocol();
				return;
			}

			output.from_char_a(recvBuf);
			sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "Bytes received: " << bytesRecv << "\n";
			sync_cerr << GetCurrentTimeTm() << " : " << "Received protocol: " << output << '\n';
			sync_cerr << GetCurrentTimeTm() << " : " << "Received bits: ";
			for (unsigned int i = 0; i < BUF_LENGTH; i++) { sync_cerr << std::bitset<8>(recvBuf[i]) << (i < BUF_LENGTH - 1 ? " " : ""); }
			sync_cerr << '\n';
		}
	}

	//Rozgrywka
	void startGame() {
		//Obliczenie czasu rozgrywki
		const unsigned int gameDuration = (abs(int(sessionIds[0] - sessionIds[1])) * 74) % 90 + 25;

		sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "GAME DURATION: " << gameDuration << "s\n";
		sync_cout << '\n' << GetCurrentTimeTm() << " : " << "GAME DURATION: " << gameDuration << "s\n";
		this->sendBinProtocolToAll(BinProtocol(OP_TIME, TIME_DURATION, NULL, gameDuration));

		//Zmienne do zarz¹dzania w¹tkami
		bool stop = false; //Zmiena gdy prawda przerywa odbieranie danych
		std::vector<BinProtocol> inputs(clientSockets.size());
		std::vector<std::thread> threads(clientSockets.size());

		//Uruchamianie w¹tków odbierania
		{
			threads[0] = std::thread([this, &inputs, &stop] {this->receiveBinProtocols(inputs[0], clientSockets[sessionIds[0]], stop); });
			threads[1] = std::thread([this, &inputs, &stop] {this->receiveBinProtocols(inputs[1], clientSockets[sessionIds[1]], stop); });
		}

		//Wygenerowanie losowej liczby
		const unsigned int secretNumber = randInt(0, int(pow(2, 8)) - 1);
		sync_cout << '\n' << GetCurrentTimeTm() << " : " << "SECRET NUMBER: " << secretNumber << "\n\n";
		sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "SECRET NUMBER: " << secretNumber << "\n\n";

		//Odliczanie do pocz¹tku rozgrywki
		{
			for (unsigned int i = 30; i > 0; i--) {
				if (stop) { break; }
				sync_cerr << GetCurrentTimeTm() << " : " << "Time to start: " << i << "s\n";
				sync_cout << GetCurrentTimeTm() << " : " << "Time to start: " << i << "s\n";
				this->sendBinProtocolToAll(BinProtocol(OP_TIME, TIME_TO_START, NULL, i));
				Sleep(1000);
			}
			sync_cout << '\n';
			sync_cerr << '\n';
		}

		//Rozpoczynanie rozgrywki
		if (!stop) {
			//Wys³anie wiadomoœci o starcie rozgrywki
			sync_cerr << GetCurrentTimeTm() << " : " << "Game start.\n";
			sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "Send game start info to players.\n";
			this->sendBinProtocolToAll(BinProtocol(OP_GAME, GAME_BEGIN, NULL, NULL));

			//Wys³anie wiadomoœci o czasie
			sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "Send time to players.\n";
			sync_cerr << GetCurrentTimeTm() << " : " << "Time left: " << gameDuration << "s\n";
			sync_cout << GetCurrentTimeTm() << " : " << "Time left: " << gameDuration << "s\n";
			this->sendBinProtocolToAll(BinProtocol(OP_TIME, TIME_LEFT, NULL, gameDuration));
		}

		//Zmienne do zarz¹dzania rozgrywk¹
		std::bitset<2> wins;

		//Rozgrywka
		if (!stop) {
			//Zmienne do informowania o pozosta³ym czasie
			const auto timeStart = std::chrono::system_clock::now();		  //Czas rozpoczêcia
			const auto timeEnd = std::chrono::duration<double>(gameDuration); //Czas rozgrywki
			auto timeMessageStart = std::chrono::system_clock::now();		  //Czas rozpoczêcia od wiadomoœci
			auto timeMessage = std::chrono::duration<double>(0);			  //Czas u¿ywany przy wiadomoœci

			for (auto time = std::chrono::duration<double>(0); time < timeEnd; time = std::chrono::system_clock::now() - timeStart) {
				timeMessage = std::chrono::system_clock::now() - timeMessageStart;

				if (stop) {
					break;
				}
				if (timeMessage >= std::chrono::duration<double>(15)) {
					sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "Send time to players.\n";
					sync_cerr << GetCurrentTimeTm() << " : " << "Time left: " << unsigned int(ceil((timeEnd - time).count())) << "s\n";
					sync_cout << GetCurrentTimeTm() << " : " << "Time left: " << unsigned int(ceil((timeEnd - time).count())) << "s\n";
					//Wys³anie wiadomoœci o czasie
					this->sendBinProtocolToAll(BinProtocol(OP_TIME, TIME_LEFT, NULL, unsigned int(ceil((timeEnd - time).count()))));
					//Koniec wysy³ania wiadomoœci o czasie
					timeMessage = std::chrono::duration<double>(0);
					timeMessageStart = std::chrono::system_clock::now();
				}

				for (unsigned int i = 0; i < sessionIds.size(); i++) {
					if (inputs[i].compare(OP_DATA, DATA_NUMBER, sessionIds[i])) {
						const unsigned int tempNumber = inputs[i].getData_Int();
						inputs[i] = BinProtocol();
						sync_cout << GetCurrentTimeTm() << " : " << "Received number " << tempNumber << " from session " << sessionIds[i] << "\n";
						sync_cerr << GetCurrentTimeTm() << " : " << "Received number " << tempNumber << " from session " << sessionIds[i] << "\n";

						if (tempNumber > secretNumber) {
							sync_cout << GetCurrentTimeTm() << " : " << "Sent message 'NUMBER TOO BIG' to session " << sessionIds[i] << "\n";
							sendBinProtocol(BinProtocol(OP_NUMBER, NUMBER_TOO_BIG, sessionIds[i], tempNumber), clientSockets[sessionIds[i]]);
						}
						else if (tempNumber == secretNumber) {
							wins[i] = true;
						}
						else if (tempNumber < secretNumber) {
							sync_cout << GetCurrentTimeTm() << " : " << "Sent message 'NUMBER TOO SMALL' to session " << sessionIds[i] << "\n";
							sendBinProtocol(BinProtocol(OP_NUMBER, NUMBER_TOO_SMALL, sessionIds[i], tempNumber), clientSockets[sessionIds[i]]);
						}
					}
				}
				if (wins[0] == true || wins[1] == true) { break; }
			}
		}

		//Zakoñczenie rozgrywki
		sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Game end.\n";
		sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "Game end.\n";
		if (!stop) {
			if (wins[0] == false && wins[1] == false) {
				sync_cout << GetCurrentTimeTm() << " : " << "Sent message 'GAME DRAW' to all players\n";
				sendBinProtocolToAll(BinProtocol(OP_GAME, GAME_DRAW, NULL, NULL));
			}
			else if (wins[0] == true) {
				sync_cout << GetCurrentTimeTm() << " : " << "Sent message 'GAME WON' to session " << sessionIds[0] << "\n";
				sendBinProtocol(BinProtocol(OP_GAME, GAME_WON, sessionIds[0], NULL), clientSockets[sessionIds[0]]);

				sync_cout << GetCurrentTimeTm() << " : " << "Sent message 'GAME LOST' to session " << sessionIds[1] << "\n";
				sendBinProtocol(BinProtocol(OP_GAME, GAME_LOST, sessionIds[1], NULL), clientSockets[sessionIds[1]]);
			}
			else if (wins[1] == true) {
				sync_cout << GetCurrentTimeTm() << " : " << "Sent message 'GAME WON' to session " << sessionIds[1] << "\n";
				sendBinProtocol(BinProtocol(OP_GAME, GAME_WON, sessionIds[1], NULL), clientSockets[sessionIds[1]]);

				sync_cout << GetCurrentTimeTm() << " : " << "Sent message 'GAME LOST' to session " << sessionIds[0] << "\n";
				sendBinProtocol(BinProtocol(OP_GAME, GAME_LOST, sessionIds[0], NULL), clientSockets[sessionIds[0]]);
			}

			stop = true;
			this->sendBinProtocolToAll(BinProtocol(OP_GAME, GAME_END, NULL, NULL));
		}
		else {
			this->sendBinProtocolToAll(BinProtocol(OP_MESSAGE, MESSAGE_OPPONENT_DISCONNECTED, NULL, NULL));
		}

		//£¹czenie w¹tków odbierania
		sync_cout << GetCurrentTimeTm() << " : " << "Waiting for clients to disconnect.\n";
		for (std::thread& thread : threads) { thread.join(); }
	}
};
