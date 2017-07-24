#include "udpserver.h"
#include<pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

_func_recv_callback g_recv_callback;
int g_isRun;
int g_sock;
void* func_recv_thread(void* param);
struct sockaddr_in *g_client_addr = NULL;
int g_test = 0;
pthread_t g_thread;
void udp_start(short port, _func_recv_callback callback)
{
	g_recv_callback = callback;
	struct sockaddr_in address = { 0 };
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	g_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (g_sock <= 0) {
		printf("创建socket失败\n");
		return;
	}
	int res = bind(g_sock, (struct sockaddr*)&address, sizeof(address));
    if(res < 0) {
        printf("绑定服务地址失败\n");
    }
	g_isRun = 1;
	pthread_create(&g_thread, NULL, func_recv_thread, NULL);
}

void udp_send(char * data, int len)
{
	if (g_sock > 0 && g_client_addr != NULL) {
		int size = sendto(g_sock, data, len, 0, (struct sockaddr*)g_client_addr, sizeof(struct sockaddr_in));
		if (size != 0) {
			printf("发送数据成功\n");
		}
		else {
			printf("发送数据失败\n");
		}
	}
}

void udp_send_to_client(char* data, int len, struct sockaddr_in* addr) {
    if (g_sock > 0) {
        int size = sendto(g_sock, data, len, 0, (struct sockaddr*)addr, sizeof(struct sockaddr_in));
        if (size != 0) {
            printf("发送数据成功\n");
        }
        else {
            printf("发送数据失败\n");
        }
    }
}

void udp_stop()
{
	g_isRun = 0;
    if(g_sock > 0) {
        close(g_sock);
    }
	int res = pthread_join(g_thread, NULL);
}

void udp_join_server() {
    int res = pthread_join(g_thread, NULL);
}

void* func_recv_thread(void* param) {
	unsigned char buf[128] = { 0 };
	while (g_isRun)
	{
		int addrLen = sizeof(struct sockaddr_in);
		struct sockaddr_in addr = {0};
		
		int res = recvfrom(g_sock, (char*)buf, 128, 0, (struct sockaddr*)&addr, &addrLen);
		if (res > 2) {
			if (g_recv_callback != NULL) {
				g_recv_callback(buf, res, &addr);
			}

			if (buf[0] == 0xfa && buf[1] == 0xaf) {
				if (g_client_addr != NULL) {
					if (g_client_addr->sin_addr.s_addr != addr.sin_addr.s_addr || g_client_addr->sin_port != addr.sin_port) {
						memcpy(g_client_addr, &addr, addrLen);
					}
					g_test = 1;
				}
				else {
					g_client_addr = (struct sockaddr_in*)malloc(addrLen);
					memcpy(g_client_addr, &addr, addrLen);
				}
			}
		}
		else if(res < 0){
			if (g_client_addr != NULL) {
				free(g_client_addr);
				g_client_addr = NULL;
			}
			break;
		}
	}
    return NULL;
}