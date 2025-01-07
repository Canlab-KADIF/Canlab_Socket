#ifndef NETWORK_H
#define NETWORK_H

#include "util.h"

#define IPADDR "127.0.0.1"
#define PORT 12345

int connect_to_server(const char* ip, int port);
void* receive_chat(void* arg);
void* send_chat(void* arg);

#endif
