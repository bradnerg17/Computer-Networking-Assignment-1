#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <iostream>
#include "FileHelper.h"
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

const char FILENAME[] = "data.bin";
const char IPADDR[] = "127.0.0.1";
const int  PORT = 50000;
const int  QUERY = 1;
const int  UPDATE = 2;

void cleanup(SOCKET socket);

int main()
{
	WSADATA		wsaData;
	SOCKET		listenSocket;
	SOCKET		acceptSocket;
	SOCKADDR_IN	serverAddr;
	int			clientRequest = 0;
	ifstream	dataFile;
	ofstream	updateFile;

	// Loads Windows DLL (Winsock version 2.2) used in network programming
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
	{
		cerr << "ERROR: Problem with WSAStartup\n";
		return 1;
	}
	// Create a new socket to listen for client connections
	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (listenSocket == INVALID_SOCKET)
	{
		cerr << "ERROR: Cannot create socket\n";
		WSACleanup();
		return 1;
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, IPADDR, &serverAddr.sin_addr);

	if (bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cerr << "ERROR: Cannot bind to port\n";
		cleanup(listenSocket);
		return 1;
	}

	cout << "Waiting for connection...\n";
	// Start listening for incoming connections
	if (listen(listenSocket, 1) == SOCKET_ERROR)
	{
		cerr << "ERROR: Problem with listening on socket\n";
		cleanup(listenSocket);
		return 1;
	}

	// Accept incoming connection.  Program pauses here until
	// a connection arrives.
	acceptSocket = accept(listenSocket, NULL, NULL); 

	// For this program, the listen socket is no longer needed so it will be closed
	closesocket(listenSocket);

	cout << "Connected...\n\n";
	do
	{
		// Wait to receive a message from the remote computer
		cout << "\n\t--WAIT--\n\n";
		int iRecv = recv(acceptSocket, (char*)&clientRequest, BUFSIZ, 0);

		if (iRecv == SOCKET_ERROR)
		{
			cerr << "ERROR: Failed to receive message\n";
			cleanup(acceptSocket);
			return 1;
		}

		if (clientRequest == QUERY)
		{
			dataFile.open(FILENAME);

			int version = readInt(dataFile);

			dataFile.close();

			int sendVer = send(acceptSocket, (char*)&version, BUFSIZ, 0);
			if (sendVer == SOCKET_ERROR)
			{
				cerr << "ERROR: Failed to receive message\n";
				cleanup(acceptSocket);
				return 1;
			}
		}

		if (clientRequest == UPDATE)
		{
			dataFile.open(FILENAME);

			int updateVer = readInt(dataFile);

			dataFile.close();

			int sendUpdate = send(acceptSocket, (char*)&updateVer, BUFSIZ, 0);
			if (sendUpdate == SOCKET_ERROR)
			{
				cerr << "ERROR: Failed to receive message\n";
				cleanup(acceptSocket);
				return 1;
			}

			cleanup(acceptSocket);
		}
	} while (clientRequest == 0);
	
	return 0;
}

void cleanup(SOCKET socket)
{
	closesocket(socket);
	WSACleanup();
}
