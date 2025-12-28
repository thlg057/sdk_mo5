#include "mo5_defs.h"

char mo5_getchar(void)
{
    asm {
        swi
        fcb $0A
    }
}
//int code = (int)ch;
void mo5_putchar(char c)
{   
    asm {
        ldb c
        swi
        fcb $02
    }
}

void mo5_newline(void)
{
    mo5_putchar(MO5_ENTER_CHAR);
    mo5_putchar(MO5_LINE_FEED);
}