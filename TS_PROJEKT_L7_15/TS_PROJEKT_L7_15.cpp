
#include "Protocol.hpp"
#include "Server.hpp"
#include <chrono>

int main()
{
	//BinProtocol protocol;
	//protocol.setOperation("010");
	//protocol.setAnswer("111");
	//protocol.setId("00100");
	//protocol.display();
	//std::cout << protocol.to_string() << "\n\n";

	const unsigned int TIME_TO_FIND_CLIENTS = 10;
	ServerTCP server;

	const auto timeStart = std::chrono::system_clock::now();
	auto dotsTimeStart = std::chrono::system_clock::now();;
	std::chrono::duration<double> dotsTime = dotsTimeStart - std::chrono::system_clock::now();
	const auto messagePeriod = std::chrono::duration<double>(TIME_TO_FIND_CLIENTS);
	bool clientConnected = false;

	for (std::chrono::duration<double> diff = std::chrono::duration<double>(0.0); diff.count() < messagePeriod.count();) {
		diff = std::chrono::system_clock::now() - timeStart;
		dotsTime = std::chrono::system_clock::now() - dotsTimeStart;

		if (server.connectClient()) { std::cout << "Client connected!\n"; clientConnected = true; }
		else if (dotsTime.count() > std::chrono::duration<double>(1.0).count()) {
			dotsTimeStart = std::chrono::system_clock::now();;
			std::cout << "...\n";
		}
	}

	if (!clientConnected) {
		std::cout << "No client connected!\n";
	}

	system("pause");
}
