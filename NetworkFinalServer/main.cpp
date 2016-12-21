#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <list>
#define SERVER_PORT 64394
#define BUFFER_SIZE 16000

void printError(wchar_t *msg);
void printErrorQuit(wchar_t *msg);

void LoopForEachClient(SOCKET clientSock,std::list<SOCKET> &allclients , std::mutex & mutex)
{
	int returnVal;
	SOCKADDR_IN clientAddr;
	int addrLen;
	char buf[BUFFER_SIZE + 1];
	std::list<SOCKET> shutDownedSocekts;
	addrLen = sizeof(clientAddr);
	getpeername(clientSock, (SOCKADDR *)&clientAddr, &addrLen);

	char clientIP[32] = { 0, };
	inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, 32 - 1);

	while (true)
	{
		returnVal = recv(clientSock, buf, BUFFER_SIZE, 0);
		if (returnVal == SOCKET_ERROR) 
		{
			printError(L"recv()");
			break;
		}
		else if (returnVal == 0)
		{
			break;
		}

		buf[returnVal] = '\0';
		printf("[TCP/%s:%d] %s\n", clientIP, ntohs(clientAddr.sin_port), buf);

		mutex.lock();
		for (auto& client : allclients)
		{
			returnVal = send(client, buf, returnVal, 0);
			if (returnVal == SOCKET_ERROR) 
			{
				printError(L"send()");
				int errorNum = WSAGetLastError();
				if (errorNum == WSAESHUTDOWN)
				{
					shutDownedSocekts.push_back(client);
				}
			}
	
		}

		for (auto& deleteClient : shutDownedSocekts)
		{
			auto iter = std::find(allclients.begin(), allclients.end(), deleteClient);
			allclients.erase(iter);
		}
		mutex.unlock();
	}

	closesocket(clientSock);
	printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n", clientIP, ntohs(clientAddr.sin_port));

	return;
}

int main(int argc, char *argv[])
{
	std::mutex mtxClientLock;
	std::list<SOCKET> clientSockets;
	WSADATA wsa;

	int returnVal = WSAStartup(MAKEWORD(2, 2), &wsa);

	if (returnVal != 0)
	{
		return 1;
	}

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		printErrorQuit(L"socket()");
	}

	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(SERVER_PORT);
	returnVal = bind(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (returnVal == SOCKET_ERROR)
	{
		printErrorQuit(L"bind()");
	}

	returnVal = listen(sock, SOMAXCONN);
	if(returnVal == SOCKET_ERROR)
	{
		printErrorQuit(L"listen()");
	}                            

	SOCKET clientSock;
	SOCKADDR_IN clientAddr;
	int addrLen;

	while (true)
	{
		addrLen = sizeof(clientAddr);
		clientSock = accept(sock, (SOCKADDR *)&clientAddr, &addrLen);
		if (clientSock == INVALID_SOCKET)
		{
			printError(L"accept()");
			break;
		}

		char clientIP[32] = { 0, };
		inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, 32 - 1);
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n", clientIP, ntohs(clientAddr.sin_port));
		mtxClientLock.lock();
		clientSockets.push_back(clientSock);
		std::thread th(LoopForEachClient, clientSock, std::ref(clientSockets), std::ref(mtxClientLock));
		th.detach();
		mtxClientLock.unlock();
	}
}

void printErrorQuit(wchar_t *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void printError(wchar_t *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
}