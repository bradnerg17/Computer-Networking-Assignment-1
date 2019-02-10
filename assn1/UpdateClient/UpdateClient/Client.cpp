/* 
Names: Greg Bradner

UpdateClient.cpp is a client for a server that adds two integers read from a file "data.bin" together and displays the results.
It starts by connecting to UpdateServer using port 50000 and requesting the current version of data.bin being used by the server.
After recieving the current version number used by the server the connection is closed and if the versions match, the program begins
its normal operation by adding the two integers. If the local version does not match the server's version, a new connection to the
server is made and UpdateClient requests the updated data file. After recieving the update data.bin is overwritten by the updated 
version and the connection is closed and the integers from the updated file are added together.

****UpdateServer.cpp must be running before starting UpdateClient.cpp****
*/
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <string>
#include <iostream>
#include "FileHelper.h"
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

const char FILENAME[] = "data.bin";
const char IPADDR[] = "127.0.0.1";
const int  PORT = 50000;
const int  QUERY = 1;
const int  UPDATE = 2;

// Returns the version number from the data file
int getLocalVersion();

// Reads the two data values from the data file.
// When the function ends, num1 and num2 will be holding the
// two values that were read from the file.
void readData(int& num1, int& num2);

void cleanup(SOCKET socket);

int main()
{
	WSADATA		wsaData;
	SOCKADDR_IN	serverAddr;
	int			sum;
	int			num1 = 0;
	int			num2 = 0;	
	int			localVersion;
	int			serverVersion;
	int			updateData[3];
	ofstream	dataFile;
	boolean		done = false;

	localVersion = getLocalVersion();
	cout << "Update client\n";
	cout << "Current data file version: v" << localVersion << "\n\n";

	// Loads Windows DLL (Winsock version 2.2) used in network programming
	if ( WSAStartup( MAKEWORD(2, 2), &wsaData ) != NO_ERROR )
	{
		cerr << "ERROR: Problem with WSAStartup\n";
		return 1;
	}

	// Create a new socket for communication
	SOCKET querySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (querySocket == INVALID_SOCKET)
	{
		cerr << "ERROR: Cannot create socket\n";
		WSACleanup();
		return 1;
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, IPADDR, &serverAddr.sin_addr);

	cout << "Connecting to server...\n";

	// Try to connect
	if (connect(querySocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cerr << "ERROR: Failed to connect\n";
		cleanup(querySocket);
		return 1;
	}

	cout << "Requesting version number...\n";

	if (send(querySocket, (char*)&QUERY, sizeof(QUERY), 0) == SOCKET_ERROR)
	{
		cerr << "ERROR: Failed to send request\n";
		cleanup(querySocket);
		return 1;
	}

	if (recv(querySocket, (char*)&serverVersion, sizeof(serverVersion), 0) == SOCKET_ERROR)
	{
		cerr << "ERROR: Failed to receive version number\n";
		cleanup(querySocket);
		return 1;
	}
	closesocket(querySocket);

	cout << "data.bin server version: v" << serverVersion << "\n\n";

	if(localVersion != serverVersion)
	{
		SOCKET updateSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (updateSocket == INVALID_SOCKET)
		{
			cerr << "ERROR: Cannot create socket\n";
			WSACleanup();
			return 1;
		}

		if (connect(updateSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		{
			cerr << "ERROR: Failed to connect\n";
			cleanup(updateSocket);
			return 1;
		}
		else
		{
			cout << "Requesting update v" << serverVersion << "...\n";

			if (send(updateSocket, (char*)&UPDATE, sizeof(UPDATE), 0) == SOCKET_ERROR)
			{
				cerr << "ERROR: Failed to send update request\n";
				cleanup(updateSocket);
				return 1;
			}

			cout << "Downloading update...\n";

			//Make a seperate function******
			for (int i = 0; i <= 2; i++)
			{
				int updateNum = 0;

				//may need to store this in a temp file or an array based on the size of updateBytes******
				int updateBytes = recv(updateSocket, (char*)&updateNum, sizeof(updateNum), 0);
				if (updateBytes == SOCKET_ERROR)
				{
					cout << "ERROR: Failed to recieve update\n";
					cleanup(updateSocket);
					return 1;
				}

				updateData[i] = updateNum;
			}

			//Make a seperate function********
			openOutputFile(dataFile, FILENAME);
			for (int i = 0; i <= 2; i++)
			{
				writeInt(dataFile, updateData[i]);
			}

			dataFile.close();
		}
		cleanup(updateSocket);
	}

	cout << "Succefully updated to v" << serverVersion << "\n\n";
	readData(num1, num2);	
	sum = num1 + num2;
	cout << "The sum of " << num1 << " and " << num2 << " is " << sum << endl;

	return 0;
}

int getLocalVersion()
{
	ifstream dataFile;
	openInputFile(dataFile, FILENAME);

	int version = readInt(dataFile);
	dataFile.close();

	return version;
}

void readData(int& num1, int& num2)
{
	ifstream dataFile;
	openInputFile(dataFile, FILENAME);

	// Read the version number and discard it
	int tmp = num1 = readInt(dataFile);

	// Read the two data values
	num1 = readInt(dataFile);
	num2 = readInt(dataFile);

	dataFile.close();
}

void cleanup(SOCKET socket)
{
	closesocket(socket);
	WSACleanup();
}