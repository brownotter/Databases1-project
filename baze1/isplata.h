#ifndef ISPLATA_H_INCLUDED
#define ISPLATA_H_INCLUDED

#include <stdio.h>

#define MAX_DATUM 11
#define MAX_RAZLOG 21
#define FAKTOR_BLOKIRANJA_ISPLATA 5

typedef struct {
    int identifikator;
    int mbr;
    int mesec;
    int godina;
    char datum_isplate[MAX_DATUM];
    float iznos;
    char razlog[MAX_RAZLOG];
} Isplata;

typedef struct {
    Isplata slogovi[FAKTOR_BLOKIRANJA_ISPLATA];
} BlokIsplata;

void meni_isplata();
void kreiraj_praznu_datoteku_isplata(const char*);
void unos_isplate(const char*);
void ispis_isplata(const char*);

#endif // ISPLATA_H_INCLUDED
