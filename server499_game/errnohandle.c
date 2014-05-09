
#include "errnohandle.h"

void ErrHandle(const int status)
{
    char *str = NULL;
    switch (status)
    {
        case 1:
            str = "Uasge: serv499 port greeting deck";
            break;
        case 4:
            str = "Invalid Port";
            break;
        case 5:
            str = "Port Error";
            break;
        case 6:
            str = "Deck Error";
            break;
        case 8:
            str = "System Error";
    }
    fprintf(stderr, "%s\n", str);
    exit(0);
}
