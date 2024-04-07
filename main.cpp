#include <iostream>
#include <WS2tcpip.h>
#include <sstream>

#pragma comment(lib,"ws2_32.lib")

using namespace std;

void main() {

	//Initialize winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	//Check if winsock is properly initialized
	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0) {
		cerr << "Can't initalize winsock!" << endl;
		return;
	}

	//Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET) {
		cerr << "Can't create a socket!" << endl;
		return;
	}

	//Bind the socket to IP and port
	sockaddr_in hint;
	hint.sin_family = AF_INET;                    //set family to IPv4
	hint.sin_port = htons(54000);				  //set port to 54000
	hint.sin_addr.S_un.S_addr = INADDR_ANY;       //set IP to any on every web interfaces
	
	bind(listening, (sockaddr*)&hint, sizeof(hint));     //binding socket "listening" to the ip settings from "hint"

	//Set socket for listening
	listen(listening, SOMAXCONN);

	fd_set master;		
	FD_ZERO(&master);				//clearing "master"

	FD_SET(listening, &master);     //added listening to master to monitore that socket

	while (true) {
		fd_set copy = master;

		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		for (int i = 0; i < socketCount; i++) {
			SOCKET sock = copy.fd_array[i];
			if (sock == listening) {
				//Accept a new connection
				SOCKET client = accept(listening, nullptr, nullptr);
				
				//Add the new connection to the list of connected clients
				FD_SET(client, &master);

				//Send a welcome message
				string welcomeMsg = "Welcome to the chat\r\n";
				send(client, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);
			}
			else {
				char buf[4096];
				ZeroMemory(buf, 4096);

				//Accept a new message
				int bytesIn = recv(sock, buf, 4096, 0);
				if (bytesIn <= 0) {
					//Drop the clinet
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else {
					//Send message
					for (int j = 0; j < master.fd_count; j++) {
						SOCKET outSock = master.fd_array[j];
							if (outSock != listening && outSock != sock) {
								ostringstream ss;
								ss << "User #" << sock << ": " << buf << "\r\n";
								string strOut = ss.str();

								send(outSock, strOut.c_str(), strOut.size()+1, 0);
						}
					}
				}
			}
		}
	}

	//Shutdown winsock
	WSACleanup();

	system("pause");
}