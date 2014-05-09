
#include "errnohandle.h"

void ErrHandle(const int status)
{
    char *str = NULL;
    switch (status)
    {
        case 1:
            str = "Uasge: client499 name game port [host]";
            break;
        case 4:
            str = "Invalid Arguments.";
            break;
        case 2:
            str = "Bad Server.";
            break;
        case 6:
            str = "Protocol Error.";
            break;
        case 7:
            str = "User Quit.";
            break;
        case 8:
            str = "System Error.";
    }
    fprintf(stderr, "%s\n", str);
    exit(0);
}
