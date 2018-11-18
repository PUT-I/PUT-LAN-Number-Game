#include "Server.hpp"
#include "Console.hpp"

const unsigned int port = 8888;

int main() {
	//Serwer nasłuchujący połączenia z każdego interfejsu
	ServerTCP server(INADDR_ANY, port);

	const unsigned int clientNumber = 2;
	bool game = true;

	CONSOLE_MANIP::show_console_cursor(false);
	while (game) {
		sync_cout << GetCurrentTimeTm() << " : " << "---- Server ----\n";
		sync_cout << GetCurrentTimeTm() << " : " << "Client accepting.\n";
		sync_cerr << GetCurrentTimeTm() << " : " << "Client accepting.\n";

		//Łączenie się z klientami
		for (unsigned int i = 0; i < clientNumber; i++) {
			sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Waiting for client " << i + 1 << ".\n";
			if (server.acceptClient()) { sync_cout << GetCurrentTimeTm() << " : " << "Client " << i + 1 << " connected.\n"; }
		}

		//Rozpoczęcie rozgrywki
		server.startGame();

		//Pytanie o inicjowanie kolejne rozgrywki
		CONSOLE_MANIP::show_console_cursor(true);
		sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Do you want to setup another game?(Y/N) ";
		while (true) {
			std::string input;
			std::cin >> input;
			if (input == "Y" || input == "y") { CONSOLE_MANIP::clear_console(); break; }
			if (input == "N" || input == "n") { game = false; break; }
			else { sync_cout << GetCurrentTimeTm() << " : " << "Bad input. Do you want to setup another game?(Y/N) "; }
		}
		CONSOLE_MANIP::show_console_cursor(false);
	}

	CONSOLE_MANIP::show_console_cursor(true);
	sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "Program end.\n";
	sync_cout << '\n';
	system("pause");
}
