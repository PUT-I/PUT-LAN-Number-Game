
#include "Protocol.hpp"
#include "Server.hpp"

int main(){
	//BinProtocol protocol;
	//protocol.setOperation("010");
	//protocol.setAnswer("111");
	//protocol.setId("00100");
	//protocol.display();
	//std::cout << protocol.to_string() << "\n\n";

	ServerTCP server;

	if (server.connectClient()) { std::cout << "Client1 connected\n"; }
	if (server.connectClient()) { std::cout << "Client2 connected\n"; }

	server.receiveBinProtocol().display();

	system("pause");
}
