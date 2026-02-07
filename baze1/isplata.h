#ifndef ISPLATA_H_INCLUDED
#define ISPLATA_H_INCLUDED

#include <stdio.h>

#define MAX_RAZLOG 21
#define FAKTOR_BLOKIRANJA_ISPLATA 5

typedef struct {
    int identifikator;          // identifikator isplate
    int mbr;                    // identifikator radnika
    int mesec;
    int godina;
    char datum_isplate[11];     // D-M-YYYY
    float iznos;
    char razlog[MAX_RAZLOG];
} Isplata;

/* Kreiranje prazne serijske datoteke */
void kreiraj_praznu_serijsku_isplata_datoteku(const char* filename);

/* Unos nove isplate */
void unos_isplate(const char* filename, const char* radnik_file);

/* Provera postojanja isplate */
int postoji_isplata(const char* filename, int identifikator);

/* Debug ispis isplata */
void ispis_isplata(const char* filename);

#endif // ISPLATA_H_INCLUDED
