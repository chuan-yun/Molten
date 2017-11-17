#ifndef __MOLTEN_AGENT_SOCKET_H
#define __MOLTEN_AGENT_SOCKET_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <fcntl.h>

#include "common.h"

int create_tcp_listen_server(char *addr, int port, int backlog, int famlily, char *err);
int set_nodelay(int fd);
int set_noblock(int fd);
#endif
