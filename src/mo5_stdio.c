#include "mo5_defs.h"
#include <cmoc.h>
#include "mo5_stdio.h"
#include "mo5_ctype.h"

int fgets(char* buffer, int max_length)
{
    int pos = 0;
    char ch;

    // Clear the buffer first
    //clear_buffer(buffer, max_length + 1);

    while (1) {
        ch = mo5_getchar();

        // Enter key - finish input
        if (ch == MO5_ENTER_CHAR) {
            // Consume any following LINE FEED character
            // Some systems send CR+LF when ENTER is pressed
            break;
        }

        // Backspace - delete last character
        if (ch == MO5_BACKSPACE_CHAR) {
            if (pos > 0) {
                pos--;
                buffer[pos] = '\0';
                // Visual backspace: move back, print space, move back again
                mo5_putchar(MO5_BACKSPACE_CHAR);
                mo5_putchar(MO5_SPACE_CHAR);
                mo5_putchar(MO5_BACKSPACE_CHAR);
            }
            continue;  // Skip to next iteration
        }

        // Printable character - add to buffer
        if (isprint(ch) && pos < max_length) {
            buffer[pos] = ch;
            pos++;
            mo5_putchar(ch);  // Echo character
        }
        // Ignore all other characters (including null and control characters)
    }

    buffer[pos] = '\0';
    return pos;
}

void fputs(const char* str)
{
    int i = 0;
    while (str[i] != '\0') {
        putchar(str[i]);
        i++;
    }
}

void puts(const char* str)
{
    fputs(str);
    mo5_newline();
}

void clrscr(void)
{
    putchar(MO5_CLEAR_SCREEN);
}