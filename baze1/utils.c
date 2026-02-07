#include "utils.h"
#include <stdio.h>

void ocisti_bafer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int provera_ascii(const char* s) {
    while (*s) {
        if ((unsigned char)(*s) > 127)
            return 0;
        s++;
    }
    return 1;
}
