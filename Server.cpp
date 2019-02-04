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
sockaddr * clientAddr;
int getLocalVersion()
{
	ifstream dataFile;
	openInputFile(dataFile, FILENAME);

	int version = readInt(dataFile);
	dataFile.close();

	return version;
}
void receiveData(SOCKET sock, char* updateBuff, int ubSize, int clientAddrSize)
{
	int clientRequest;

	clientRequest = recv(sock, updateBuff, ubSize, 0);
	switch (clientRequest) {
	case SOCKET_ERROR:
		cerr << "ERROR: Listen failed\n";
		WSACleanup();
		return;
		break;
	case 1:
		updateBuff = (char*)getLocalVersion();
		if (send(sock, updateBuff, ubSize, 0) < 0)
		{
			cerr << "ERROR: Send failed\n";
			WSACleanup();
			return;
		}
		closesocket(sock);
		WSACleanup();
		SOCKET newSock;
		if (newSock = accept(sock, clientAddr, &clientAddrSize) > 0) {
			cerr << "ERROR: Accept failed\n";
			WSACleanup();
			return;
		}
		receiveData(newSock, updateBuff, ubSize);
		break;
	case 2:
		ifstream dataFile;
		openInputFile(dataFile, FILENAME);
		updateBuff = (char*)readInt(dataFile);
		dataFile.close();
		if (send(sock, updateBuff, ubSize, 0) < 0)
		{
			cerr << "ERROR: Send failed\n";
			WSACleanup();
			return;
		}
		closesocket(sock);
		WSACleanup();
		//SOCKET newSock;
		if (newSock = accept(sock, clientAddr, &clientAddrSize) > 0) {
			cerr << "ERROR: Accept failed\n";
			WSACleanup();
			return;
		}
		receiveData(newSock, updateBuff, ubSize);
		break;
	}
}
//void cleanUp(SOCKET socket)
//{
	//closesocket(socket);
	//WSACleanup();
//}

int main()
{
	// Add your code here for the server
	WSADATA wsaData;
	
	SOCKET sock1;
	SOCKET sock2;
	int clientAddrSize = sizeof(clientAddr);
	int clientRequest;
	char updateBuff[BUFSIZ] ;
	int ubSize = sizeof(updateBuff);
	//Load Windows DLL
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
		cerr << "ERROR: Problem with WSAStartup\n";
		return 1;
	}
	//Create Socket
	sock1 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock1 == INVALID_SOCKET)
	{
		cerr << "ERROR: Cannot Create Socket\n";
		WSACleanup();
		return 1;
	}
	//clientAddr.sin_family = AF_INET;
	//clientAddr.sin_port = htons(PORT);
	//inet_pton(AF_INET, IPADDR, &clientAddr.sin_addr);

	int currentVersion = getLocalVersion();
	if (bind(sock1, clientAddr, clientAddrSize > 0))
	{
		cerr << "ERROR: Bind failed\n";
			WSACleanup();
			return 1;
	}

	if (listen(sock1, 3) > 0)
	{
		cerr << "ERROR: Listen failed\n";
			WSACleanup();
			return 1;
	}

	if (sock2 = accept(sock1, clientAddr, &clientAddrSize) > 0) {
		cerr << "ERROR: Accept failed\n";
		WSACleanup();
		return 1;
	}

	receiveData(sock2, updateBuff, ubSize);
	closesocket(sock2);
	WSACleanup();
	return 0;
}
