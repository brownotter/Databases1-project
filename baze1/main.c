#include <stdio.h>
#include <string.h>

#include "radnik.h"
#include "isplata.h"
#include "utils.h"


int main() {

    //printf("Hello world!\n");
    //return 0;

    int izbor;

    do {
        printf("\n--- GLAVNI MENI ---\n");
        printf("1. Radnici\n");
        printf("2. Isplate\n");
        printf("0. Izlaz\n");
        printf("Izbor: ");
        scanf("%d", &izbor);
        ocisti_bafer();

        switch(izbor) {
            case 1:
                meni_radnik();
                break;
            case 2:
                meni_isplata();
                break;
        }

    } while(izbor != 0);

    return 0;
}


