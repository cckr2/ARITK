#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN 
#pragma comment(lib, "Ws2_32.lib")
#define BUF_SIZE 100


int main(int argc, char **argv) {
	WSADATA wsaData;
	struct sockaddr_in server_addr;
	SOCKET s;

	if (argc != 3) {
		printf("Command parameter does not right.\n");
		exit(1);
	}

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Socket Creat Error.\n");
		exit(1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));

	if (connect(s, (SOCKADDR *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		printf("Socket Connection Error.\n");
		exit(1);
	}

	printf("Connect Complete\n");

	char *msg = "ping";
	int sendBytes;
	sendBytes = send(s, msg, sizeof(msg), 0);
	printf("Send message : %s\n", msg);

	char buf[BUF_SIZE];
	int recvBytes;
	recvBytes = recv(s, buf, BUF_SIZE, 0);
	printf("Recv message : %s\n", buf);

	closesocket(s);
	WSACleanup();

	return 0;
}