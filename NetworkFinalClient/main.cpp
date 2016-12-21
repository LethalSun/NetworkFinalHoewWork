#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <thread>

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 64394
#define BUFSIZE    16000

wchar_t* ConverCtoWC(char* str);
void printErrorQuit(wchar_t *msg);
void printError(char *msg);


void LoopForRecv(SOCKET sock)
{
	int received;
	char buf[BUFSIZE];
	
	while (true) {
		received = recv(sock, buf, BUFSIZE, 0);
		if (received == SOCKET_ERROR)
		{
			printError("recv()");
			break;
		}
		else if (received == 0)
		{
			break;
		}
		
		// 받은 데이터 출력
		buf[received] = '\0';
		printf("[TCP 클라이언트] %d바이트를 받았습니다.\n", received);
		printf("[받은 데이터] %s\n", buf);
	
	}

	return;
}

int main(int argc, char *argv[])
{
	// 윈속 초기화
	WSADATA wsa;
	int returnVal = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (returnVal != 0)
	{
		return 1;
	}

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		printErrorQuit(L"socket()");
	}

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	auto ret = inet_pton(AF_INET, SERVERIP, (void *)&serveraddr.sin_addr.s_addr);
	serveraddr.sin_port = htons(SERVERPORT);

	returnVal = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (returnVal == SOCKET_ERROR) 
	{
		printErrorQuit(L"connect()");
	}

	std::thread th(LoopForRecv, sock);
	th.detach();

	// 데이터 통신에 사용할 변수
	char buf[BUFSIZE + 1];
	int len;

	// 서버와 데이터 통신
	while (true) 
	{
		// 데이터 입력
		printf("\n입력 최대 바이트는 16000입니다.>");
		if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
		{
			break;
		}

		// '\n' 문자 제거
		len = strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		if (strlen(buf) == 0)
			break;

		// 데이터 보내기
		returnVal = send(sock, buf, strlen(buf), 0);
		if (returnVal == SOCKET_ERROR) 
		{
			printError("send()");
			break;
		}
		printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n", returnVal);

	}

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}

wchar_t* ConverCtoWC(char* str)
{
	//wchar_t형 변수 선언
	wchar_t* pStr;
	//멀티 바이트 크기 계산 길이 반환
	int strSize = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, NULL);
	//wchar_t 메모리 할당
	pStr = new WCHAR[strSize];
	//형 변환
	MultiByteToWideChar(CP_ACP, 0, str, strlen(str) + 1, pStr, strSize);
	return pStr;
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

void printError(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}