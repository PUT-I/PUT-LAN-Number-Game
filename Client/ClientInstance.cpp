
#include "Client.hpp"
#include <Iphlpapi.h>

#pragma comment(lib, "Iphlpapi.lib")

const unsigned int port = 8888;

//Zwraca tablicę adresów IP z tablicy ARP, gdzie adresy podzielone są na 4 części
std::vector<std::vector<std::string>> GET_IP_TABLE_FRAGM() {
	//Zmienna do przechowania zawartości tablicy ARP
	PMIB_IPNETTABLE pIPNetTable = nullptr;
	//Pobieranie tablicy ARP
	{
		/*
			Fragment kodu bazowany na informacjach z poniższej strony:
			https://www.codeproject.com/Articles/5960/WebControls/?fid=32450&df=90&mpp=25&prof=True&sort=Position&view=Normal&spc=Relaxed&fr=51
		*/
		ULONG ulSize = 0;
		GetIpNetTable(pIPNetTable, &ulSize, TRUE);
		pIPNetTable = new MIB_IPNETTABLE[ulSize];
		if (nullptr != pIPNetTable) {
			GetIpNetTable(pIPNetTable, &ulSize, TRUE);
		}
	}

	//Pętla parsująca uzyskane dane z tablicy ARP
	std::vector<std::string> IPv4_Correct;
	std::vector<std::vector<std::string>> IPv4_Table;
	for (unsigned int i = 0; i < pIPNetTable->dwNumEntries; i++) {
		struct in_addr addr;
		addr.s_addr = ntohl(static_cast<long>(pIPNetTable->table[i].dwAddr));
		std::string IPv4(inet_ntoa(addr));
		std::string temp;
		for (int j = IPv4.size() - 1; j >= 0; j--) {
			if (IPv4[j] != '.') { temp.insert(temp.begin(), IPv4[j]); }
			if (IPv4[j] == '.' || j == 0) {
				IPv4_Correct.push_back(temp);
				temp.clear();
			}
		}
		if (IPv4_Correct.size() == 4) {
			if (IPv4_Correct[0] != "224" && IPv4_Correct[0] != "255" &&
				IPv4_Correct[2] != "255" && IPv4_Correct[3] != "255") {
				IPv4_Table.push_back(IPv4_Correct);
			}
			IPv4_Correct.clear();
		}
	}
	return IPv4_Table;
}

//Zwraca tablicę adresów IP z tablicy ARP
std::vector<std::string> GET_IP_TABLE() {
	//Tablica całych adresów IP
	std::vector<std::string> IPv4_Table;
	//Pętla szukająca pożądanego IP
	for (const std::vector<std::string>& elem : GET_IP_TABLE_FRAGM()) {
		std::string result;
		for (unsigned int i = 0; i < elem.size(); i++) {
			result += elem[i];
			if (i < elem.size() - 1) { result += '.'; }
		}
		IPv4_Table.push_back(result);
	}
	return IPv4_Table;
}

//Połączenie z serwerem i rozpoczęcie rozgrywki
bool connect_server(ClientTCP& client) {
	if (client.connect_server()) {
		sync_cout << GET_CURRENT_TIME() << " : " << "Server connected.\n\n";

		client.start_game();
		return true;
	}
	else { return false; }
}

//Funkcja użyta do szukania serwera na wielu wątkach
void try_to_connect_server(const std::string& address, bool& connected) {
	if (!connected) {
		ClientTCP client(inet_addr(address.c_str()), port);
		connected = connect_server(client);

		if (!connected) {
			sync_cerr << GET_CURRENT_TIME() << " : " << "Failed to connect on address: " << address << "\n";
			sync_cerr << GET_CURRENT_TIME() << " : " << "Trying another address.\n";
		}
	}
}

//Wersja dla sieci lokalnej (i loopback)
void local_network() {
	std::vector<std::string> IPv4_Table = GET_IP_TABLE();

	//Jeśli znaleziono adresy IP
	if (IPv4_Table.size() > 0) {
		ClientTCP client(inet_addr(""), port);
		sync_cout << GET_CURRENT_TIME() << " : " << "---- Client ----\n";
		sync_cout << GET_CURRENT_TIME() << " : " << "Trying to connect with server.\n";
		sync_cerr << GET_CURRENT_TIME() << " : " << "Trying to connect with server.\n";

		bool connected = false;

		//Próba połączenia z każdym z adresów (każda na osobnym wątku)
		std::vector<std::thread>threads;
		for (const auto& elem : IPv4_Table) {
			threads.push_back(std::thread(&try_to_connect_server, std::ref(elem), std::ref(connected)));
			if (connected == true) { break; }
		}
		//Łączenie wątków szukania, jeśli jeszcze się nie zakończyłu po zagrywce
		for (auto& thread : threads) { thread.join(); }
		if (!connected) {
			client.info_init(inet_addr("127.0.0.1"), port);
			connected = connect_server(client);
		}
		if (!connected) {
			sync_cout << '\n' << GET_CURRENT_TIME() << " : " << "Could not connect with any IP address.\n";
			sync_cerr << '\n' << GET_CURRENT_TIME() << " : " << "Could not connect with any IP address.\n";
		}
	}
	else {
		sync_cout << '\n' << GET_CURRENT_TIME() << " : " << "No IP address found.\n";
		sync_cerr << '\n' << GET_CURRENT_TIME() << " : " << "No IP address found.\n";
	}
}

int main() {
	CONSOLE_MANIP::show_console_cursor(false);
	local_network();
	sync_cerr << '\n' << GET_CURRENT_TIME() << " : " << "Program end.\n";
	sync_cout << '\n';
	CONSOLE_MANIP::show_console_cursor(true);
	system("pause");
}