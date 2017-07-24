#ifndef _UDP_SERVER_EFWIN
#define _UDP_SERVER_EFWIN
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

typedef void(*_func_recv_callback)(char* data, int len, struct sockaddr_in* addr);

extern int g_test;
void udp_start(short port, _func_recv_callback callback);

void udp_send_to_client(char* data, int len, struct sockaddr_in* addr);

void udp_send(char* data, int len);

void udp_join_server();

void udp_stop();
#endif // _UDP_SERVER_EFWIN
