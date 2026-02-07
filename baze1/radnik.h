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

/* Kreiranje prazne serijske datoteke */
void kreiraj_praznu_serijsku_radnik_datoteku(const char* filename);

/* Unos novog radnika */
void unos_radnika(const char* filename);

/* Prikaz radnika po mbr */
void prikaz_radnika_po_mbr(const char* filename, int mbr);

/* Prikaz radnika sa bonusima preko 2000 */
void radnici_sa_bonusima(const char* radnik_file, const char* isplata_file);

/* Modifikacija radnika */
void izmeni_radnika(const char* filename, int mbr);

/* Provera postojanja radnika */
int postoji_radnik(const char* filename, int mbr);

/* Debug ispis */
void ispis_radnika(const char* filename);

//bonus
void radnici_sa_bonusima(const char* radnik_file, const char* isplata_file);


#endif // RADNIK_H_INCLUDED
