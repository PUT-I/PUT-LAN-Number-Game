
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
		ULONG ulSize = 0;
		GetIpNetTable(pIPNetTable, &ulSize, TRUE);
		pIPNetTable = new MIB_IPNETTABLE[ulSize];
		if (nullptr != pIPNetTable)
		{
			GetIpNetTable(pIPNetTable, &ulSize, TRUE);
		}
	}

	//Pętla szukająca pożądanego IP
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

//Znajduje adres IP dla połączenia przez ethernet
std::string GET_ETHERNET_IP() {
	std::vector<std::string> IPv4_Correct;
	for (const std::vector<std::string>& IPv4 : GET_IP_TABLE_FRAGM()) {
		if (IPv4[0] == "169" &&
			IPv4[1] == "254" &&
			IPv4[2] != "0"   && IPv4[2] != "255" &&
			IPv4[3] != "0"   && IPv4[3] != "255") {
			IPv4_Correct = IPv4;
			break;
		}
	}

	std::string result;
	for (unsigned int i = 0; i < IPv4_Correct.size(); i++) {
		result += IPv4_Correct[i];
		if (i < IPv4_Correct.size() - 1) { result += '.'; }
	}
	return result;
}

//Połączenie z serwerem i rozpoczęcie rozgrywki
bool connectServer(ClientTCP& client) {
	if (client.connectServer()) {
		sync_cout << GetCurrentTimeTm() << " : " << "Server connected.\n\n";

		client.startGame();
		return true;
	}
	else { return false; }
}

//Funkcja użyta do szukania serwera na wielu wątkach
void tryToConnectServer(const std::string& address, bool& connected) {
	ClientTCP client(inet_addr(""), port);
	if (!connected) {
		client.infoInit(inet_addr(address.c_str()), port);
		connected = connectServer(client);

		if (!connected) {
			sync_cerr << GetCurrentTimeTm() << " : " << "Failed to connect on address: " << address << "\n";
			sync_cerr << GetCurrentTimeTm() << " : " << "Trying another address.\n";
		}
	}
}

//Wersja dla sieci lokalnej (i loopback)
void localNetwork() {
	std::vector<std::string> IPv4_Table = GET_IP_TABLE();

	//Jeśli znaleziono adresy IP
	if (IPv4_Table.size() > 0) {
		ClientTCP client(inet_addr(""), port);
		sync_cout << GetCurrentTimeTm() << " : " << "---- Client ----\n";
		sync_cout << GetCurrentTimeTm() << " : " << "Trying to connect with server.\n";
		sync_cerr << GetCurrentTimeTm() << " : " << "Trying to connect with server.\n";

		bool connected = false;

		//Próba połączenia z każdym z adresów (każda na osobnym wątku)
		std::vector<std::thread>threads;
		for (const auto& elem : IPv4_Table) {
			threads.push_back(std::thread(&tryToConnectServer, std::ref(elem), std::ref(connected)));
			if (connected == true) { break; }
		}
		//Łączenie wątków szukania, jeśli jeszcze się nie zakończyłu po zagrywce
		for (auto& thread : threads) { thread.join(); }
		if(!connected){
			client.infoInit(inet_addr("127.0.0.1"), port);
			connected = connectServer(client);
		}
		if(!connected){
			sync_cout << '\n' << GetCurrentTimeTm() << " : " << "Could not connect with any IP address.\n";
			sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "Could not connect with any IP address.\n";
		}
	}
	else {
		sync_cout << '\n' << GetCurrentTimeTm() << " : " << "No IP address found.\n";
		sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "No IP address found.\n";
	}
}

int main() {
	CONSOLE_MANIP::show_console_cursor(false);
	localNetwork();
	sync_cerr << '\n' << GetCurrentTimeTm() << " : " << "Program end.\n";
	sync_cout << '\n';
	CONSOLE_MANIP::show_console_cursor(true);
	system("pause");
}