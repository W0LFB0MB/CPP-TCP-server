#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

void main() {
	//Initilise winsock
	WSADATA wsData; //WinSock Application Data
	WORD ver = MAKEWORD(2, 2); // Winsock version

	int wsOk = WSAStartup(ver, &wsData); //Initialise/startup winsock with specified version & memor address of WSAData
	if (wsOk != 0) { //Checking if Winsock started ok
		cerr << "Failed to initialise Winsock, quitting\n";
		return;
	}

	//Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0); // AF_INET = IPv4 (AF_INET6 = IPv6) SOCK_STREAM = TCP SOCK_DGRAM = UDP
	if (listening == INVALID_SOCKET) { //Checking if socket is valid
		cerr << "Failed to create socket, quitting \n";
		return;
	}

	//Bind the socket to an IP and port
	sockaddr_in hint; //Socketaddress instance?
	hint.sin_family = AF_INET; //Socket in using IPv4
	hint.sin_port = htons(54000); //htons = Host To Network Short
	hint.sin_addr.S_un.S_addr = INADDR_ANY; //socket address doesn't need to be bound to a specific IP address

	bind(listening, (sockaddr*)&hint, sizeof(hint)); //No clue what this does :O
	//https://youtu.be/WDn-htpBlnU?t=810

	//Tell winsock the socket is listening
	listen(listening, SOMAXCONN); //Sets port for listening with max connections SOMAXCONN = Socket Max Connections

	fd_set master; //creating master set
	FD_ZERO(&master); //zeroing out master set

	FD_SET(listening, &master);//adding the listening port to the master set

	while (true) {
		fd_set copy = master;

		int socketCount = select(0, &copy, nullptr, nullptr, nullptr); // get amount of sockets, nullptr=null pointer
		for (int i = 0; i < socketCount; i++) { // loop through sockets
			SOCKET sock = copy.fd_array[i]; // get socket
			if (sock == listening) { //check if socket is the listening socket
				//Accept new connection
				SOCKET client = accept(listening, nullptr, nullptr);

				//Add new connection to the master set
				FD_SET(client, &master);

				//TODO: log the new Connection
				clog << "A New user has connected" << endl;

				//Send a welcome message to the connected client
				string welcomeMsg = "[SERVER] > Welcome to the chat server!\n";
				send(client, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);

				//Broadcast a message notifying other users of the new user
				for (int j = 0; j < master.fd_count; j++) {
					SOCKET outSock = master.fd_array[j];
					if (outSock != listening && outSock != client) {
						string newUserMessage = "[SERVER] > A new user has joined the chat!\n";
						send(outSock, newUserMessage.c_str(), newUserMessage.size(), 0);
					}
				}

				//TODO: Allow entering an alias/username
			} else {
				char buf[8192]; //8KB buffer (8192 char array)
				ZeroMemory(buf, 8192); //zero out the buffer

				//Accept new message
				int bytesIn = recv(sock, buf, 8192, 0);
				if (bytesIn <= 0) {
					//Drop the client
					closesocket(sock);
					FD_CLR(sock, &master);
				} else {
					//Send message to other clients and NOT the listening socket
					for (int j = 0; j < master.fd_count; j++) {
						SOCKET outSock = master.fd_array[j];
						if (outSock != listening && outSock != sock) {
							ostringstream ss;
							ss << "SOCKET " << sock << " > " << buf << "\n";
							string strOut = ss.str();

							send(outSock, strOut.c_str(), strOut.size() + 1, 0);
						}
					}
				}	
			}
		}
	}

	//Cleanup Winsock
	WSACleanup();
}


////Wait for a conenction
	//sockaddr_in client; //Socketaddress instance?
	//int clientSize = sizeof(client); // size of client

	//SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize); // accepts client connection I think??? no clue what the arguments are for

	//if (clientSocket == INVALID_SOCKET) { // checking if clientSocket is valid
	//	cerr << "Failed to create client socket, quitting\n";
	//	return;
	//}

	//char host[NI_MAXHOST]; // clients remote name
	//char service[NI_MAXSERV]; // service (i.e. port) client is connected on

	//ZeroMemory(host, NI_MAXHOST); //uninitialised function???
	//ZeroMemory(service, NI_MAXSERV); //uninitialised function???

	//if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) { // check ifyou can get name info
	//	cout << host << " connected on port " << service << endl;
	//} else {
	//	inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST); // get IP address I think
	//	cout << host << " connected on port " << ntohs(client.sin_port) << endl;
	//}

	////Close listenning socket
	//closesocket(listening); //closing listening socket

	////While loop: accept and echo message back to client
	//char buf[4096]; // 4KB buffer

	//while (true) {
	//	ZeroMemory(buf, 4096); //zeroing out the entirety of the buffer

	//	//Wait for client to send data
	//	int bytesRecieved = recv(clientSocket, buf, 4096, 0); //amount of bytes recieved
	//	if (bytesRecieved == SOCKET_ERROR) { // check if socket error
	//		cerr << "Error in recieve, quitting\n";
	//		break;
	//	}

	//	if (bytesRecieved == 0) { // check if client disconnected
	//		cout << "Client Disconnected\n";
	//		break;
	//	}

	//	//Echo message back to client
	//	send(clientSocket, buf, bytesRecieved + 1, 0); //echo message back, null terminated as buf was set to zeroes before recieving data

	//}