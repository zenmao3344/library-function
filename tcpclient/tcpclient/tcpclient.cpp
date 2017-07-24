// tcpclient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include<Windows.h>

#pragma comment(lib, "ws2_32.lib")

const char A2X[] = {
	'0', '1', '2', '3',
	'4', '5', '6', '7',
	'8', '9', 'A', 'B',
	'C', 'D', 'E', 'F'
};

FILE* file = NULL;
char buf[1024] = { 0 };
void write(char* src, int len) {
	int l, h, i;
	for (i = 0; i < len; i++) {
		l = src[i] & 0x0f;
		h = (src[i] >> 4) & 0x0f;
		buf[2 * i] = A2X[h];
		buf[2 * i + 1] = A2X[l];
	}
	buf[2 * i] = '\n';
	buf[2 * i + 1] = '\0';
	fwrite(buf, 1, 2 * i + 1, file);
}

int main(int argc, char* argv[])
{
	if (argc < 3) {
		return -1;
	}
	const char* IP = argv[1];
	const char* strPort = argv[2];
	int port = atoi(strPort);

	
	fopen_s(&file, "save.txt", "a+");

	WSADATA wsaData = { 0 };
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = inet_addr(IP);
	addr.sin_port = htons(port);

	int res = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
	if (res == SOCKET_ERROR) {
		printf("连接失败!\r\n");
		return -1;
	}
	char buf[128];
	char buf1[128];

	int len;
	while (true) {
		len = recv(sock, buf, 128, 0);
		if (len > 0) {
			write(buf, len);
			send(sock, buf1, len, 0);
		}
		else {
			break;
		}
	}
	fclose(file);
	printf("连接断开\r\n");
    return 0;
}
