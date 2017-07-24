#include<stdio.h>
#include "udpserver.h"

void recv_callback(char* data, int len, struct sockaddr_in* addr) {
	int i = 0;
    printf("收到数据:");
    for(i = 0; i < len; i++) {
        printf("%02x ", data[i]);
    }
    udp_send(data, len);
}

int main() {

    udp_start(23121, recv_callback);
    udp_join_server();
    return 0;
}