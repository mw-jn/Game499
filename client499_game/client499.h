
#ifndef CLIENT499_H
#define CLIENT499_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include "errnohandle.h"
#include "charOperation.h"

#define BUF 1024     /* define the buffer to store the information */

/* Domain name resolution function */
static struct sockaddr getaddrbyname(const char *hostname, const char *server, struct addrinfo *const hints);
static void sortCards(int (*cards)[13], int *cards_size, const char *const msg);
static void printCards(int (*cards)[13], int *cards_size);
static void removeCard(const char cardX, const char cardY, int (*cards)[13], int *cards_size);

#endif
