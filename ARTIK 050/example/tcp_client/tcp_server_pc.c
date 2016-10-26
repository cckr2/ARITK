#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <windows.h>
#if defined(_WIN32)
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN	
#pragma comment(lib, "Ws2_32.lib")
#define sleep(sec)  Sleep((sec)*1000)
#endif
typedef unsigned long long u64;
u64 GetMicroCounter();
#define BUF_SIZE 100


int main(int argc, char **argv) {
	u64 start, end;
	WSADATA wsaData;
	struct sockaddr_in local_addr;
	SOCKET	s_listen;

	if (argc != 2) {
		printf("Command parameter does not right.\n");
		exit(1);
	}

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	if ((s_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Socket Creat Error.\n");
		exit(1);
	}

	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	local_addr.sin_port = htons(atoi(argv[1]));

	if (bind(s_listen, (SOCKADDR *)&local_addr, sizeof(local_addr)) == SOCKET_ERROR) {
		printf("Socket Bind Error.\n");
		exit(1);
	}

	if (listen(s_listen, 5) == SOCKET_ERROR) {
		printf("Socket Listen Error.\n");
		exit(1);
	}

	printf("This server is listening... \n");

	struct sockaddr_in client_addr;
	int	len_addr = sizeof(client_addr);
	SOCKET	s_accept;

	char buf[BUF_SIZE];
	int Bytes;
	char *msg = "pong\n";
	int sendBytes;

	s_accept = accept(s_listen, (SOCKADDR *)&client_addr, &len_addr);
	if (s_accept) {
		printf("Connection Request from Client [IP:%s, Port:%d] has been Accepted\n",
		inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

	
		Bytes = recv(s_accept, buf, BUF_SIZE, 0);
		printf("%s\n", buf);
		sendBytes = send(s_accept, msg, sizeof(msg), 0);
		Sleep(5000);
		closesocket(s_accept);
	}

	closesocket(s_listen);
	WSACleanup();

	return 0;
}

