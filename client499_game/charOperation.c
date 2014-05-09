#include "charOperation.h"

int charToDigtal(const char c)
{
    int a = -1;
    if ((c > '0') && (c <= '9'))
        a = c - '0';
    else if ((c >= 'A') && (c <= 'Z'))
    {
        switch(c)
        {
            case 'A':
                a = 14;
                break;
            case 'T':
                a = 10;
                break;
            case 'J':
                a = 11;
                break;
            case 'Q':
                a = 12;
                break;
            case 'K':
                a = 13;
        }
    }

    return a;
}

char digtalToChar(const int digtal)
{
    char c = '0';
    if((digtal >= 2) && (digtal <= 9))
        c = digtal + '0';
    else
    {
        switch(digtal)
        {
            case 10:
                c = 'T';
                break;
            case 11:
                c = 'J';
                break;
            case 12:
                c = 'Q';
                break;
            case 13:
                c = 'K';
                break;
            case 14:
                c = 'A';
        }
    }

    return c;
}
