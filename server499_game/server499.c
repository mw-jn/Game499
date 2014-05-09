#include "server499.h"


static struct game *gameList = NULL;
char deck[DECKNUM][CARDNUM+1];
int deck_size = 0;
const int bidTable[4][6] = {{20, 70, 120, 170, 220, 270},
                            {30, 80, 130, 180, 230, 280},
                            {40, 90, 140, 190, 240, 290},
                            {50, 100, 150, 200, 250, 300}
                         };

/* Domain name resolution function */
static struct sockaddr getaddrbyname(const char *hostname, const char *server, struct addrinfo *const hints)
{
    int portnum;
    struct sockaddr sa;
    struct addrinfo *res;
    memset(hints, 0, sizeof(struct addrinfo));
    hints->ai_flags = AI_PASSIVE | AI_NUMERICSERV;
    hints->ai_family = AF_INET;
    hints->ai_socktype = SOCK_STREAM;
    if (getaddrinfo(hostname, server, hints, &res) != 0)
    {
        perror("Domain name resolution failed...");
        _exit(errno);
    }

    portnum = atoi(server);
    if ((portnum < 1) || (portnum > 65535))
        ErrHandle(4);

    sa = *(res->ai_addr);
    freeaddrinfo(res);

    return sa;
}

/* define the thread haddle function*/

static void sendCardsToClient(struct game *const pGame, const int deck_num, char (*cards)[27], int *cards_size)
{
    int i, flag;
    int player_id;
    int playerCard_num;
    char buf[30];
    i = flag = 0;
    playerCard_num = 0;
    memset(pGame->playerCards, 0, 4*27);

    while (i < 104)
    {
        player_id = flag % 4;
        pGame->playerCards[player_id][playerCard_num] = deck[deck_num][i++];
        pGame->playerCards[player_id][playerCard_num + 1] = deck[deck_num][i++];
        flag ++;
        if (flag % 4 == 0)
          playerCard_num += 2;

    }

    for (i = 0; i < 4; i++)
    {
        strncpy(buf, "H", 2);
        strncat(buf, pGame->playerCards[i],26);
        write(pGame->client_fd[i], buf, strlen(buf));
        strncpy(cards[i], buf, 27);
        cards_size[i] = strlen(buf);
        fprintf(stdout, "Send Cards to clinet-%d: %s; string-len: %d\n", pGame->client_fd[i], buf, strlen(buf));
        fflush(stdout);
    }
}

static int charToDigital(const char c)
{
    int integer;
    switch(c)
    {
        case 'S':
            integer = 0;
            break;
        case 'C':
            integer = 1;
            break;
        case 'D':
            integer = 2;
            break;
        case 'H':
            integer = 3;
            break;
        default:
            integer = -1;
    }
    return integer;
}

void *gameStart(void *arg)
{
    /* read the cards form the specific deck */
    struct game *pGame;
    int team1_score = 0;
    int team2_score = 0;
    int baseScore;
    int i, j, deck_index, temp_int;
    long deck_count;
    char bidX, bidY, pre_bidX, pre_bidY;
    char cards[4][27];
    int cards_size[4];
    char *buf;
    int *passflag;
    int pass_count;
    int bid_valid;
    int trump_index;
    ssize_t numRead;
    int table_row, table_col;
    pGame = (struct game *)arg;          /* convert the pointer void to the struct pointer game */
    buf = (char *)malloc(BUF);           /** declare the storage for buffer **/

    fprintf(stdout, "Game %s Starting...\n", pGame->gamename);
    fflush(stdout);
    /* send the team information to the clients*/
    memset(buf, 0, BUF);
    strcpy(buf, "MTeam1: ");
    strcat(buf, pGame->playname[0]);
    strcat(buf, " ");
    strcat(buf, pGame->playname[2]);
    for (i = 0; i < 4; i++)
        write(pGame->client_fd[i], buf, strlen(buf)+1);
    fprintf(stdout, "MTeam1: %s, %s\n",pGame->playname[0], pGame->playname[2]);
    fflush(stdout);
    memset(buf, 0, BUF);

    strcpy(buf, "MTeam2: ");
    strcat(buf, pGame->playname[1]);
    strcat(buf, " ");
    strcat(buf, pGame->playname[3]);
    for (i = 0; i < 4; i++)
        write(pGame->client_fd[i], buf, strlen(buf)+1);
    fprintf(stdout, "MTeam2: %s, %s\n",pGame->playname[1], pGame->playname[3]);
    fflush(stdout);
    sleep(1);

    /* deal the cards for 4 players */
    deck_count = 0;
    while (team1_score < 499 && team1_score > -499)
    {
        deck_index = deck_count % deck_size;
        sendCardsToClient(pGame, deck_index, cards, cards_size);     /* send the cards to clients */
        sleep(1);
        /* stage of biding */
        passflag = (int *)malloc(4*sizeof(int));
        memset(passflag, 0, 4*sizeof(int));
        do
        {
            write(pGame->client_fd[0], "B", 2);
            numRead = read(pGame->client_fd[0], buf, BUF);
            buf[numRead] = '\0';
            /* check the valid */
            sscanf(buf, "%c%c", &bidX, &bidY);
            temp_int = charToDigital(bidY);
            if ((temp_int >= 0) && (temp_int <= 3) && (bidX >= '4') && (bidX <= '9'))
            {
                sprintf(buf, "M%s bids %c%c", pGame->playname[0], bidX, bidY);
                fprintf(stdout, "%s\n", buf);
                fflush(stdout);
                for (i = 1; i < 4; i++)
                    write(pGame->client_fd[i], buf, strlen(buf)+1);
                break;
            }
        }while(1);

        pass_count = 0;

        for (i = 1; !(pass_count == 3) && !((bidX == '9') && (bidY == 'H')); i++, i %= 4)
        {
            if((bidX != 'P') && (bidY != 'P'))
            {
                pre_bidX = bidX;
                pre_bidY = bidY;
            }
            bid_valid = 0;

            if (!passflag[i])
            {
                do
                {
                    sprintf(buf, "B%c%c", pre_bidX, pre_bidY);
                    write(pGame->client_fd[i], buf, strlen(buf)+1);
                    numRead = read(pGame->client_fd[i], buf, BUF);
                    buf[numRead] = '\0';
                    /* check the biding valid */
                    sscanf(buf, "%c%c", &bidX, &bidY);
                    if ((bidX == 'P') && (bidY == 'P'))
                    {
                        passflag[i] = 1;
                        pass_count ++;
                        bid_valid = 1;
                        sprintf(buf, "M%s passes.", pGame->playname[i]);
                        fprintf(stdout, "%s\n", buf);
                        fflush(stdout);
                        for (j = 0; j < 4; j++)
                        {
                            if (j != i)
                                write(pGame->client_fd[j], buf, strlen(buf)+1);
                        }
                    }


                    if ((bidX >= '4') && (bidX <= '9') && (temp_int >= 0) && (temp_int <= 3))
                    {
                        if (temp_int > charToDigital(pre_bidY))
                            bid_valid = 1;
                        else if((temp_int == charToDigital(pre_bidY)) && (bidX > pre_bidX))
                            bid_valid = 1;
                        else
                            bid_valid = 0;
                        if (bid_valid)
                        {
                            sprintf(buf, "M%s bids %c%c", pGame->playname[i], bidX, bidY);
                            for (j = 0; j < 4; j++)
                            {
                                if (j != i)
                                    write(pGame->client_fd[j], buf, strlen(buf)+1);
                            }
                        }
                    }

                }while(!bid_valid);
            }
            sleep(1);
        }
        /* get the trump and get the client_fd index */
        if ((bidX == '9') && (bidY == 'H'))
        {
            //temp_int = i;//have some problem
            pre_bidX = bidX;
            pre_bidY = bidY;
        }

        if (pass_count == 3)
        {
            for (i = 0; (i < 4) && (passflag[i]); i++);
            //temp_int = i;//have some problem
        }
        temp_int = (i+3)%4;    /* ((i-1)+4)%4*/
        sprintf(buf, "T%c%c", pre_bidX, pre_bidY);
        table_row = charToDigital(pre_bidY);
        table_col = convertChar(pre_bidX)-4;
        baseScore = bidTable[table_row][table_col];
        for (i = 0; i < 4; i++)
            write(pGame->client_fd[i], buf, 3);

        /* Send the first */
        while (team1_score < 499 && team1_score > -499)
        {
            bid_valid = 0;
            do
            {
                write(pGame->client_fd[temp_int], "L", 1);      /* Send the information to trump client */
                numRead = read(pGame->client_fd[temp_int], buf, BUF);
                buf[numRead] = '\0';
                sscanf(buf, "%c%c", &bidX, &bidY);
                if (checkCardsValid(cards, cards_size, bidX, bidY, 0))
                {
                    write(pGame->client_fd[temp_int], "A", 1);
                    pre_bidX = bidX;
                    pre_bidY = bidY;
                    for (i = 1; i < 4; i++)
                    {
                        sprintf(buf, "M%s plays %c%c",pGame->playname[0], bidX, bidY);
                        write(pGame->client_fd[i], buf, strlen(buf));
                    }
                    bid_valid = 1;
                    trump_index = temp_int;
                }
            }while(!bid_valid);

            for (i = (temp_int+1); i != temp_int; i++, i%=4)
            {
                bid_valid = 0;
                do
                {
                    sprintf(buf, "P%c", pre_bidY);
                    write(pGame->client_fd[i], buf, strlen(buf));      /* Send the information to trump client */
                    numRead = read(pGame->client_fd[i], buf, BUF);
                    buf[numRead] = '\0';
                    sscanf(buf, "%c%c", &bidX, &bidY);
                    if (checkCardsValid(cards, cards_size, bidX, bidY, i))
                    {
                        write(pGame->client_fd[i], "A", 1);
                        for (j = 1; j < 4; j++)
                        {
                            if (i != j)
                            {
                                sprintf(buf, "M%s plays %c%c",pGame->playname[0], bidX, bidY);
                                write(pGame->client_fd[j], buf, strlen(buf));
                            }
                        }
                        bid_valid = 1;
                    }
                }while(!bid_valid);

                if ((bidY == pre_bidY) && cardComp(pre_bidX, bidX))
                {
                    pre_bidX = bidX;
                    pre_bidY = bidY;
                    trump_index = i;
                }
            }
            temp_int = trump_index;
            sleep(1);
            for (i = 0; i < 4; i++)
            {
                sprintf(buf, "M%s won", pGame->playname[trump_index]);
                write(pGame->client_fd[i], buf, strlen(buf));
            }

            if (trump_index == 0 || trump_index == 2)
                team1_score += baseScore;
            else
                team1_score -= baseScore;

        }
        sprintf(buf, "MTeam 1=%d, Team 2=%d", team1_score, team2_score);
        for (i = 0; i < 4; i++)
            write(pGame->client_fd[i], buf, strlen(buf));

        free(passflag);
        passflag = NULL;
        deck_count++;
        break;
    }

    free(buf); /** release the storage for buffer **/
    for (i = 0; i < 4; i++)
        write(pGame->client_fd[i], "O", 1);
    pthread_detach(pthread_self());     /* The thread destroyed by itself */
    return NULL;
}

/* Check the play cards valid */
static int checkCardsValid(char (*cards)[27], int *cards_size, const char cardsX, const char cardsY, const int client_index)
{
    int i, j;
    for (i = 0; (i < cards_size[client_index]) && ((cards[client_index][i] != cardsX) || (cards[client_index][i+1] != cardsY)); i += 2);
    if (i == cards_size[client_index])
        return 0;
    for (j =i; j < cards_size[client_index]-2; j+=2)
    {
        cards[client_index][j] = cards[client_index][j+1];
        cards[client_index][j+1] = cards[client_index][j+2];
    }
    cards_size[client_index] -= 2;
    return 1;
}

static int cardComp(const char card1, const char card2)
{
    int char_int = convertChar(card1);
    return char_int < convertChar(card2) ? 1 : 0;
}

static int convertChar(const char ch)
{
    int num;
    if (ch >= '2' && ch <= '9')
        num = ch - '0';
    else
    {
        switch (ch)
        {
            case 'A':
                num = 14;
                break;
            case 'K':
                num = 13;
                break;
            case 'Q':
                num = 12;
                break;
            case 'J':
                num = 11;
                break;
            case 'T':
                num = 10;
                break;
            default:
                num = -1;
        }
    }
    return num;
}

/* The program entrance */
int main(int argc, char *argv[])
{
    int sfd, accsfd;    /* declare the socket file descrition */
    struct sockaddr sa;  /* declare the socket address to store the server's socket address */
    struct sockaddr_in sa_client;
    socklen_t len_addr_client;
    struct addrinfo hints;
    int numRead;
    char buf[BUF];              /* declare the buffer to store the information */
    char gamename[MAXCHAR];
    char playname[MAXCHAR];
    struct game *pGame;
    struct game *preGame;
    char greetinginfo[MAXCHAR];

    /* The parameters of the input error */
    if ((argc != 3) && (argc != 4))
        ErrHandle(1);

    /* create the socket */
    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket creating failed...");
        _exit(errno);
    }

    /* Initialize the information of the socket */
    sa = getaddrbyname(NULL, argv[1], &hints);

    if (bind(sfd, &sa, sizeof(struct sockaddr)) == -1)
    {
        perror("Binding failed...");
        _exit(errno);
    }

    pGame = (struct game *)malloc(sizeof(struct game));    /* declare a empty node so that insert and delete node convenience */
    pGame->nextGame = NULL;
    gameList = pGame;

    /* load the deck from the specific deck name */
    if (deck_check(argv[3]) != 0)
    {
        ErrHandle(6);
        exit(0);
    }

    greetinginfo[0] = 'M';        /* meet the client protocol messages */
    strncat(greetinginfo, argv[2], strlen(argv[2])+1);
    greetinginfo[strlen(greetinginfo)] = '\0';

    while (1)
    {
        //addtion information
        fprintf(stdout, "Main thread listening...\n");
        fflush(stdout);

        if (listen(sfd, MAXLIS) == -1)
        {
            perror("Server listen failed...");
            _exit(errno);
        }
        len_addr_client = sizeof (struct sockaddr_in);
        if ((accsfd = accept(sfd, (struct sockaddr*) &sa_client, &len_addr_client)) == -1)
        {
            perror("Accept failed...");
        }

        fprintf(stdout, "Accept the client-%d\n", accsfd);
        fflush(stdout);
        write(accsfd, greetinginfo,strlen(greetinginfo)+1);    /* Send the greeting information to the client */
        fprintf(stdout, "Send the greeting infomation \"M%s\" to client-%d\n", argv[2], accsfd);
        fflush(stdout);

        memset(buf, 0, BUF);                       /* Get the play name and get the game name*/
        numRead = read(accsfd, buf, BUF);
        strncpy(playname, buf, numRead);
        sscanf(buf, "%s%s", playname, gamename);

        preGame = gameList;
        pGame = gameList->nextGame;
        while ((pGame != NULL) && strcmp(pGame->gamename, gamename))      /* check the sam game name */
        {
            preGame = pGame;
            pGame = pGame->nextGame;
        }

        if (pGame == NULL)
        {  /* if the gamename isn't in the game list, create new game and add the client into the new game and append the new game to the game list */
            pGame = (struct game *)malloc(sizeof(struct game));
            pGame->client_num = 0;
            strcpy(pGame->playname[0], playname);
            pGame->client_fd[0] = accsfd;
            strcpy(pGame->gamename, gamename);
            (pGame->client_num)++;
            pGame->nextGame = gameList->nextGame;
            gameList->nextGame = pGame;
        }
        else
        {/* if not meet the condition, then add the client to the game */
            strcpy(pGame->playname[pGame->client_num], playname);
            pGame->client_fd[pGame->client_num] = accsfd;
            (pGame->client_num)++;
            if (pGame->client_num == 4)      /* if the game starting meets, remove the game and start the game */
            {
                preGame->nextGame = pGame->nextGame;
                if (pthread_create(&(pGame->tid), NULL, gameStart, (void *)pGame) != 0)
                {
                    perror("Pthread_create failed...");
                }
                pGame = NULL;
            }
        }

    }

    /* Got the socket file description(sfd) and sfd will communication with server in future */


    return 0;
}
