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
		sync_cout << GET_CURRENT_TIME() << " : " << "---- Server ----\n";
		sync_cout << GET_CURRENT_TIME() << " : " << "Client accepting.\n";
		sync_cerr << GET_CURRENT_TIME() << " : " << "Client accepting.\n";

		//Łączenie się z klientami
		for (unsigned int i = 0; i < clientNumber; i++) {
			sync_cout << '\n' << GET_CURRENT_TIME() << " : " << "Waiting for client " << i + 1 << ".\n";
			if (server.accept_client()) { sync_cout << GET_CURRENT_TIME() << " : " << "Client " << i + 1 << " connected.\n"; }
		}

		//Rozpoczęcie rozgrywki
		server.start_game();

		//Pytanie o inicjowanie kolejne rozgrywki
		CONSOLE_MANIP::show_console_cursor(true);
		sync_cout << '\n' << GET_CURRENT_TIME() << " : " << "Do you want to setup another game?(Y/N) ";
		while (true) {
			std::string input;
			CONSOLE_MANIP::input_string_letters_y_n(input, 1);
			if (input == "Y" || input == "y") { CONSOLE_MANIP::clear_console(); break; }
			else if (input == "N" || input == "n") { game = false; break; }
			else { sync_cout << GET_CURRENT_TIME() << " : " << "Bad input. Do you want to setup another game?(Y/N) "; }
		}
		CONSOLE_MANIP::show_console_cursor(false);
	}

	CONSOLE_MANIP::show_console_cursor(true);
	sync_cerr << '\n' << GET_CURRENT_TIME() << " : " << "Program end.\n";
	sync_cout << '\n';
	system("pause");
}
