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
		
		// ���� ������ ���
		buf[received] = '\0';
		printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� �޾ҽ��ϴ�.\n", received);
		printf("[���� ������] %s\n", buf);
	
	}

	return;
}

int main(int argc, char *argv[])
{
	// ���� �ʱ�ȭ
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

	// ������ ��ſ� ����� ����
	char buf[BUFSIZE + 1];
	int len;

	// ������ ������ ���
	while (true) 
	{
		// ������ �Է�
		printf("\n�Է� �ִ� ����Ʈ�� 16000�Դϴ�.>");
		if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
		{
			break;
		}

		// '\n' ���� ����
		len = strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		if (strlen(buf) == 0)
			break;

		// ������ ������
		returnVal = send(sock, buf, strlen(buf), 0);
		if (returnVal == SOCKET_ERROR) 
		{
			printError("send()");
			break;
		}
		printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� ���½��ϴ�.\n", returnVal);

	}

	// closesocket()
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}

wchar_t* ConverCtoWC(char* str)
{
	//wchar_t�� ���� ����
	wchar_t* pStr;
	//��Ƽ ����Ʈ ũ�� ��� ���� ��ȯ
	int strSize = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, NULL);
	//wchar_t �޸� �Ҵ�
	pStr = new WCHAR[strSize];
	//�� ��ȯ
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