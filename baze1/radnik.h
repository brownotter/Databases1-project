#ifndef RADNIK_H_INCLUDED
#define RADNIK_H_INCLUDED

#include <stdio.h>

#define MAX_IME 31
#define MAX_PREZIME 31
#define FAKTOR_BLOKIRANJA_RADNIK 5

typedef struct {
    int mbr;
    char ime[MAX_IME];
    char prezime[MAX_PREZIME];
    float plata;
    float premija;
} Radnik;

typedef struct {
    Radnik slogovi[FAKTOR_BLOKIRANJA_RADNIK];
} BlokRadnik;

void meni_radnik();

void kreiraj_praznu_datoteku_radnik(const char*);

void unos_radnika(const char*);

void ispis_radnika(const char*);

void prikaz_radnika(const char*, int);

void modifikacija_radnika(const char*, int);

void uslov_bonus(const char* radnici_fajl, const char* isplate_fajl);

#endif // RADNIK_H_INCLUDED
