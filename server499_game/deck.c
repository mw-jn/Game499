#include "deck.h"

extern char deck[DECKNUM][CARDNUM+1];
extern int deck_size;

int deck_check(const char * const filename)
{
    FILE *deck_file;
    char buf[BUF];
    int flag[53];      /* flag[0] isn't used. Thus, the index will be 1 to 52*/
    int base;
    int i;

    memset(flag, 0, 53);
    if ((deck_file = fopen(filename, "r")) == NULL)
    {
        return 1;
    }

    while ((fgets(buf, BUF, deck_file)) && (deck_size < DECKNUM))
    {
        for (i = 0; buf[i]; i += 2)
        {
            switch (buf[i+1])
            {
                case 'S':
                    base = 0;
                    break;
                case 'C':
                    base = 13;
                    break;
                case 'D':
                    base = 26;
                    break;
                case 'H':
                    base = 39;
            }

            if ((buf[i] > '1') && (buf[i] <= '9'))
            {
                base += buf[i] - '0';
            }
            else if ((buf[i] >= 'A') && (buf[i] <= 'Z'))
            {
                switch(buf[i])
                {
                    case 'A':
                        base += 1;
                        break;
                    case 'T':
                        base += 10;
                        break;
                    case 'J':
                        base += 11;
                        break;
                    case 'Q':
                        base += 12;
                        break;
                    case 'K':
                        base += 13;
                }
            }
            else
            {
                fprintf(stderr, "Exsit the invalid charactor...\n");
                return 2;
            }
            flag[base] = 1;
        }

        for (i = 1; (i < 53) && flag[i]; i++);    /* if all flags are 1, then the deck is valid. */
        if (i != 53)
        {
            fprintf(stderr, "The deck invalid...");
            return 3;
        }
        strcpy(deck[deck_size++], buf);
    }
    return 0;
}
