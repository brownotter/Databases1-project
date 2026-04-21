#ifndef AGREGAT_H_INCLUDED
#define AGREGAT_H_INCLUDED

#include "isplata.h"

#define F1 5  //fakt blok prim zone
#define N 2   //zona indeksa
#define FZ 1 //faktor blokiranja

#define DATA_FILE "agregat.bin"
#define INDEX_FILE "index.bin"
#define OVERFLOW_FILE "overflow.bin"

typedef struct {
    int mbr;
    char ime[31];
    char prezime[31];
    float plata;
    float premija;
    float uk_bonus;
    float uk_dnevnice;
    int status;
    long next; // pokazivac na overflow
} Agregat;

typedef struct {
    int max_mbr;
    long adresa_bloka;
} IndexSlog;

typedef struct {
    IndexSlog slogovi[N];
    int broj;
} IndexBlok;

void meni_agregat();

void debug_ispis();

void formiraj_aktivnu_datoteku(const char*, const char*);
void formiraj_indeks();

void pretraga_po_mbr(int mbr);
void prikaz_uslova();

void logicko_brisanje(int mbr);

void insert_agregat(Agregat novi);
void pretraga_sa_overflow(int mbr);
void logicko_brisanje_overflow(int mbr);

void update_agregat(Agregat novi);
void azuriraj_agregat_isplata(Isplata i);

void reorganizuj_datoteku();

#endif // AGREGAT_H_INCLUDED
