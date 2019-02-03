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

int main()
{
	WSADATA     wsaData;
	SOCKADDR_IN serverAddr;
	int			sum;
	int			num1 = 0;
	int			num2 = 0;	
	int			localVersion = 0;
	int			serverVersion = 0;
	char*	    updateBuff;
	ofstream	dataFile;
	ofstream	updateFile;
	ifstream	readUpdate;

	// Add code here to
	// 1) make sure that we are using the current version of the data file
	// 2) update the data file if it is out of data
	
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
		//cleanup(mySocket);
		return 1;
	}

	int sendQuery = send(mySocket, (char*)QUERY, 1, 0);
	if (sendQuery == SOCKET_ERROR) 
	{
		cerr << "ERROR: Failed to send message\n";
		//cleanup(mySocket);
		return 1;
	}

	int iRecv = recv(mySocket, (char*)serverVersion, BUFSIZ, 0);
	if (iRecv == 0)
	{
		cout << "Connection closed\n";
		//cleanup(mySocket);
		return 0;
	}
	else
	{
		cerr << "ERROR: Failed to receive message\n";
		//cleanup(mySocket);
		return 1;
	}

	// Main purpose of the program starts here: read two numbers from the data file and calculate the sum
	localVersion = getLocalVersion();
	cout << "\nSum Calculator current version " << localVersion << "\n\n";

	if(localVersion != serverVersion)
	{
		int sendUpdate = send(mySocket, (char*)UPDATE, 1, 0);
		if (sendUpdate == SOCKET_ERROR)
		{
			cerr << "ERROR: Failed to send message\n";
			//cleanup(mySocket);
			return 1;
		}

		cout << "\n Application must be updated\n\n";

		recv(mySocket, updateBuff, BUFSIZ, 0);

		updateFile.open(UPDATENAME);
		writeInt(updateFile, (int)updateBuff);
		updateFile.close();

		readUpdate.open(UPDATENAME);
		int toData = readInt(readUpdate);
		readUpdate.close();

		dataFile.open(FILENAME);
		writeInt(dataFile, toData);
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