#ifndef AGREGAT_H_INCLUDED
#define AGREGAT_H_INCLUDED

#include <stdio.h>

#define F1 5   // faktor blokiranja primarne zone
#define N 2    // faktor indeksa (red stabla = 2)
#define FZ 1   // zona prekoraèenja

typedef struct {
    int mbr;
    char ime[31];
    char prezime[31];
    float plata;
    float premija;
    float uk_bonus;
    float uk_dnevnice;
    int status; //0=aktivan, 1=obrisan
    int next;
} Agregat;

typedef struct {
    int mbr;
    long adresa_bloka;
} IndexSlog;

typedef struct {
    IndexSlog slogovi[N];
    int broj;
} IndexBlok;

void formiraj_agregiranu_datoteku(
    const char* radnici_fajl,
    const char* isplate_fajl,
    const char* izlazni_fajl
);

void meni_agregat();

// prikaz po mbr
void prikaz_dnevnice_po_mbr(const char* filename, int mbr);

// uslov: dnevnica > bonus
void prikaz_uslova(const char* filename);

// logicko brisanje
void logicko_brisanje_agregat(const char* filename, int mbr);

void formiraj_indeks(FILE *f);

void pretraga_po_mbr(const char* filename, int mbr);

void upis_agregat_sa_overflow(FILE *f, Agregat novi);

int dodaj_u_lanac(FILE *f, Agregat *glava, Agregat novi);

void pretraga_sa_overflow(FILE *f, int mbr);

#endif // AGREGAT_H_INCLUDED
