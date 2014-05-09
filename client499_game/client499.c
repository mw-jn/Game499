#include "client499.h"

/* Domain name resolution function */
static struct sockaddr getaddrbyname(const char *hostname, const char *server, struct addrinfo *const hints)
{
    int portnum;
    struct addrinfo *res;
    struct sockaddr sa;
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

/* sort by the specific order */
static void sortCards(int (*cards)[13], int *cards_size, const char *const msg)
{
    int i, j, k;
    int index;
    int char_value;
    int len;
    len = strlen(msg);
    for (i = 0; i < len; i++)
    {
        char_value = charToDigtal(msg[i]);
        switch(msg[++i])
        {
            case 'S':
                index = 0;
                break;
            case 'C':
                index = 1;
                break;
            case 'D':
                index = 2;
                break;
            case 'H':
                index = 3;
        }

        if (cards_size[index] == 0)
        {
            cards[index][0] = char_value;
        }
        else
        {
            for (j = cards_size[index]-1; (j >= 0) && (char_value > cards[index][j]); j--);

            for (k = cards_size[index]-1; k > j; k--)
                cards[index][k+1] = cards[index][k];
            cards[index][j+1] = char_value;
        }
        cards_size[index]++;
    }
}

static void printCards(int (*cards)[13], int *cards_size)
{
    char c;
    int i, j;
    for (i = 0; i < 4; i++)
    {
        switch(i)
        {
            case 0:
                c = 'S';
                break;
            case 1:
                c = 'C';
                break;
            case 2:
                c = 'D';
                break;
            case 3:
                c = 'H';
        }
        fprintf(stdout, "%c:", c);
        fflush(stdout);
        for (j = 0; j < cards_size[i]; j++)
        {
            c = digtalToChar(cards[i][j]);
            fprintf(stdout, " %c", c);
            fflush(stdout);
        }
        fprintf(stdout, "\n");
    }
}

/* The program entrance */
int main(int argc, char *argv[])
{
    int sfd;    /* declare the socket file descrition */
    struct sockaddr sa;  /* declare the socket address to store the server's socket address */
    struct addrinfo hints;
    char buf[BUF];          /* declare the buffer to store the information */
    char msg[BUF];
    int cards[4][13];      /* the row 4 represent S,C,D,H and the column represent cards 'A':14 '2'-'9':2-9 'T':10 'J':11 'Q':12 'K':13 */
    int cards_size[4];         /* record the cards size */
    char *hostname;
    ssize_t numRead;
    int i;
    char cardX, cardY;

    /* The parameters of the input error */
    if ((argc != 4) && (argc != 5))
        ErrHandle(1);

    /* Get the hostname according to the number of the input */
    if (argc == 4)
        hostname = "localhost";
    else
        hostname = argv[4];

    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    /* create the socket */
    {
        perror("Socket creating failed...");
        _exit(errno);
    }

    /* Initialize the information of the socket */
    sa = getaddrbyname(hostname, argv[3], &hints);

    if (connect(sfd, &sa, sizeof(struct sockaddr)) == -1)
    {
        perror("Connecting failed...");
        _exit(errno);
    }
    /* send the playername and gamename to the server */
    strncpy(msg, argv[1], strlen(argv[1])+1);
    strncat(msg, " ", 2);
    strncat(msg, argv[2], strlen(argv[2])+1);
    write(sfd, msg, strlen(msg)+1);

    memset(cards, 0, 4*13*sizeof(int));
    memset(cards_size, 0, 4*sizeof(int));
    /* Got the socket file description(sfd) and sfd will communication with server in future */
    while ((numRead = read(sfd, buf, BUF)) != -1)
    {
        for (i = 1; i < numRead; i++)
            msg[i-1] = buf[i];
        msg[numRead-1] = '\0';             /** convert the C style string in end of '\0' **/
        switch(buf[0])
        {
            case 'M':
                fprintf(stdout, "Info: %s\n", msg);
                fflush(stdout);
                break;
            case 'H':
                fprintf(stdout, "%s\n", msg);
                sortCards(cards, cards_size, msg);
                printCards(cards, cards_size);
                break;
            case 'B':
                if (!strcmp(msg, ""))   /* if the msg is empty, then it explain that the server specific firstly */
                {
                    fprintf(stdout, "Bid>");
                }
                else
                {
                    sscanf(msg, "%c%c", &cardX, &cardY);
                    fprintf(stdout, "[%c%c] - Bid (or pass)>",cardX, cardY);
                }
                fflush(stdout);
                /* Deal with like this in order to flush the system core buffer */
                read(STDIN_FILENO, buf, BUF);
                sscanf(buf, "%c%c", &cardX, &cardY);
                sprintf(buf, "%c%c", cardX, cardY);
                write(sfd, buf, 2);
                break;
            case 'T':
                fprintf(stdout, "%c%c\n", msg[0], msg[1]);
                fflush(stdout);
                break;
            case 'L':
                printCards(cards, cards_size);
                fprintf(stdout, "Lead>");
                fflush(stdout);
                numRead = read(STDIN_FILENO, buf, BUF);
                sscanf(buf, "%c%c", &cardX, &cardY);
                sprintf(msg, "%c%c", cardX, cardY);
                write (sfd, msg, 2);
                break;
            case 'P':
                printCards(cards, cards_size);
                fprintf(stdout, "[%c] play>", msg[0]);
                fflush(stdout);
                numRead = read(STDIN_FILENO, buf, BUF);
                sscanf(buf, "%c%c", &cardX, &cardY);
                sprintf(msg, "%c%c", cardX, cardY);
                write (sfd, msg, 2);
                break;
            case 'A':
                removeCard(cardX, cardY, cards, cards_size);
                break;
            case 'O':
                close(sfd);
                return 0;
        }
        memset(buf, 0, BUF);
    }

    close(sfd);     /* close the unused file description */
    return 0;
}

static void removeCard(const char cardX, const char cardY, int (*cards)[13], int *cards_size)
{
    int index, i, j;
    switch(cardY)
    {
        case 'S':
            index = 0;
            break;
        case 'C':
            index = 1;
            break;
        case 'D':
            index = 2;
            break;
        case 'H':
            index = 3;
    }
    for (i = 0; (i < cards_size[index]) && (charToDigtal(cardX) != cards[index][i]); i++);
    for (j = i; j < cards_size[index]-1; j++)
    {
        cards[index][j] = cards[index][j+1];
    }
    cards_size[index] --;
}
