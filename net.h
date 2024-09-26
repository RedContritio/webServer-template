#ifndef __NET_H__
#define __NET_H__

#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_DEFAULT_PORT 5913

#define ADDRSTR_SIZE (INET_ADDRSTRLEN + 1)

#endif // __NET_H__