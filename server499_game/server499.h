
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
#include <pthread.h>
#include "errnohandle.h"
#include "deck.h"

#define MAXCHAR 20
#define MAXLIS 100   /* define the maximum number of listening of server */
#define DECKNUM 5
#define CARDNUM 104


struct game
{
    pthread_t tid;
    int client_fd[4];
    char gamename[MAXCHAR];   /* record the game name fo this game */
    char playname[4][MAXCHAR];    /* record the play information of this game */
    int client_num;
    char playerCards[4][27];
    struct game *nextGame;
};

/* Domain name resolution function */
static struct sockaddr getaddrbyname(const char *hostname, const char *server, struct addrinfo *const hints);
void *gameStart(void *arg);
static void sendCardsToClient(struct game *const pGame, const int deck_num, char (*cards)[27], int *cards_size);
static int charToDigital(const char c);
static int checkCardsValid(char (*cards)[27], int *cards_size, const char cardsX, const char cardsY, const int client_index);
static int cardComp(const char card1, const char card2);
static int convertChar(const char ch);

#endif


#ifndef BUF
#define BUF 1024
#endif
