/*
Names: Greg Bradner, Jamel Koonce

UpdateServer.cpp is a server program that allows connection from a client program to host and then execute its code.
It starts by opening itself on the local IP host address '127.0.0.1' using port 50000 and begins listening for cnnections
from a client program. After recieving and validating the client connection, it checks to see what type of request is being
made by the client program. The client will check to see what version of the program is currently being hosted by the server
and compare it to it's local copy. If they are different, the server will establish a new connection to provide the client with
the updated file information. After completion, the connection will close and the client will execute the program. The server will
then continue to wait for a new connection from a client.

****UpdateServer.cpp must be running before starting UpdateClient.cpp****
*/
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

int sendUpdate(SOCKET acceptSocket, ifstream& inFile);
void cleanup(SOCKET socket);

int main()
{
	WSADATA		wsaData;
	SOCKET		listenSocket;
	SOCKET		acceptSocket;
	SOCKADDR_IN	serverAddr;
	ifstream	inFile;
	int			version;
	boolean		done = false;
	int			numRequests = 0;

	openInputFile(inFile, FILENAME);
	version = readInt(inFile);
	inFile.close();

	cout << "Update server\n";
	cout << "Current data file version: v" << version << "\n";
	cout << "Running on port number " << PORT << "\n\n";

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

	// Start listening for incoming connections
	if (listen(listenSocket, 1) == SOCKET_ERROR)
	{
		cerr << "ERROR: Problem with listening on socket\n";
		cleanup(listenSocket);
		return 1;
	}


	// Accept incoming connection.  Program pauses here until
	// a connection arrives.

	while (numRequests >= 0)
	{
		cout << "Waiting for connections...\n";

		acceptSocket = accept(listenSocket, NULL, NULL);
		if (acceptSocket == INVALID_SOCKET)
		{
			cerr << "ERROR: Invalid socket\n\n";
			cleanup(listenSocket);
			cleanup(acceptSocket);
			return 1;
		}

		cout << "Connection received\n";

		numRequests++;
		//Force a version check on every 5 connections
		if (numRequests % 5 == 0)
		{
			openInputFile(inFile, FILENAME);
			version = readInt(inFile);
			inFile.close();
		}

		int	clientRequest;

		// Wait to receive a message from the remote computer
		int iRecv = recv(acceptSocket, (char*)&clientRequest, sizeof(clientRequest), 0);

		if (iRecv == SOCKET_ERROR)
		{
			cerr << "ERROR: Failed to recieve request.\n\n";
			cleanup(listenSocket);
			cleanup(acceptSocket);
			return 1;
		}
		else
		{	//Provides Client with current copy of the data file it can access.
			if (clientRequest == QUERY)
			{
				cout << "\tRequest for current version number: v" << version << "\n";
				int sendVer = send(acceptSocket, (char*)&version, sizeof(version), 0);
				if (sendVer == SOCKET_ERROR)
				{
					cerr << "ERROR: Failed to send version.\n\n";
					cleanup(listenSocket);
					cleanup(acceptSocket);
					return 1;
				}
			}//Prepares to give the client updated file information using the 'sendUpdate' function
			else if (clientRequest == UPDATE)
			{
				cout << "\t Request for update: v" << version << "\n";
				openInputFile(inFile, FILENAME);
				int readUpdate = sendUpdate(acceptSocket, inFile);
				inFile.close();

				if (readUpdate == SOCKET_ERROR)
				{
					cerr << "ERROR: Failed to send update.\n\n";
					cleanup(listenSocket);
					cleanup(acceptSocket);
					return 1;
				}
			}
			else
			{
				cerr << "ERROR: Invalid request.\n\n";
			}
		}

		closesocket(acceptSocket);
		cout << "\tConnection closed\n";
		cout << "Total requests handled: " << numRequests << "\n\n";

	} 

	return 0;
}
//Each version of the data file contains 3 lines of information. Top line contains
//file version number, 2nd and 3rd lines contain two values to be added by the client.
//sendUpdate loops through the file, reads, and sends those values to the connected client program.
int sendUpdate(SOCKET acceptSocket, ifstream& inFile)
{
	int sendUpdate = 0;
	for (int i = 0; i <= 2; i++)
	{
		int updateByte = readInt(inFile);
		cout << "\t\tSending " << updateByte << "\n";
		sendUpdate = send(acceptSocket, (char*)&updateByte, sizeof(updateByte), 0);

		if (sendUpdate == SOCKET_ERROR)
		{
			break;
		}
	}
	return sendUpdate;
}
void cleanup(SOCKET socket)
{
	closesocket(socket);
	WSACleanup();
}
