#include "mo5_ctype.h"

int islower(char c) {
    return (c >= 'a' && c <= 'z') ? TRUE : FALSE;
}

int isupper(char c) {
    return (c >= 'A' && c <= 'Z') ? TRUE : FALSE;
}

int isprint(char c) {
    return (c >= 32 && c <= 126) ? TRUE : FALSE;
}

int ispunct(char c) {
    return ((c >= 33 && c <= 47)  ||
            (c >= 58 && c <= 64)  ||
            (c >= 91 && c <= 96)  ||
            (c >= 123 && c <= 126)) ? TRUE : FALSE;
}
