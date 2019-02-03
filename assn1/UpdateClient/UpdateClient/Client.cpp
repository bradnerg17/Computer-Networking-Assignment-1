#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <iostream>
#include "FileHelper.h"
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

const char FILENAME[] = "data.bin";
const char UPDATENAME[] = "update.bin";
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
	WSADATA     wsaData;
	SOCKADDR_IN serverAddr;
	int			sum;
	int			num1 = 0;
	int			num2 = 0;	
	int			localVersion = 0;
	int			serverVersion;
	char	    updateBuff[BUFSIZ];
	ofstream	dataFile;
	ofstream	updateFile;
	ifstream	readUpdate;
	
	// Loads Windows DLL (Winsock version 2.2) used in network programming
	if ( WSAStartup( MAKEWORD(2, 2), &wsaData ) != NO_ERROR )
	{
		cerr << "ERROR: Problem with WSAStartup\n";
		return 1;
	}

	// Create a new socket for communication
	SOCKET mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (mySocket == INVALID_SOCKET)
	{
		cerr << "ERROR: Cannot create socket\n";
		WSACleanup();
		return 1;
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, IPADDR, &serverAddr.sin_addr);

	// Try to connect
	if (connect(mySocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cerr << "ERROR: Failed to connect\n";
		cleanup(mySocket);
		return 1;
	}

	int sendQuery = send(mySocket, (char*)&QUERY, 1, 0);
	
	cout << "Contacting server...\n";

	if (sendQuery == SOCKET_ERROR) 
	{
		cerr << "ERROR: Failed to send request\n";
		cleanup(mySocket);
		return 1;
	}

	int versionRecv = recv(mySocket, (char*)&serverVersion, BUFSIZ, 0);

	cout << "Recieving version number...\n";

	if (versionRecv == SOCKET_ERROR)
	{
		cerr << "ERROR: Failed to receive version number\n";
		cleanup(mySocket);
		return 1;
	}

	// Main purpose of the program starts here: read two numbers from the data file and calculate the sum
	localVersion = getLocalVersion();

	cout << "\nSum Calculator server version " << serverVersion << "\n\n";

	cout << "Sum Calculator current version " << localVersion << "\n\n";

	if(localVersion != serverVersion)
	{
		cout << "\nSum Calculator must be updated\n\n";

		int sendUpdate = send(mySocket, (char*)&UPDATE, 1, 0);

		cout << "Requesting update version " << serverVersion << "...\n";

		if (sendUpdate == SOCKET_ERROR)
		{
			cerr << "ERROR: Failed to send update request\n";
			cleanup(mySocket);
			return 1;
		}

		int updateRecv = recv(mySocket, updateBuff, BUFSIZ, 0);

		cout << "Recieving update information";

		if (updateRecv == SOCKET_ERROR)
		{
			cout << "ERROR: Failed to recieve update\n";
			cleanup(mySocket);
			return 1;
		}

		updateFile.open(UPDATENAME);
		writeInt(updateFile, (int)updateBuff);
		updateFile.close();

		readUpdate.open(UPDATENAME);
		int toData = readInt(readUpdate);
		readUpdate.close();

		dataFile.open(FILENAME);
		writeInt(dataFile, toData);

		cout << "Updating data file\n";

		dataFile.close();
	}

	
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