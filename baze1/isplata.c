#include "isplata.h"
#include "radnik.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>

int validan_razlog(const char* razlog) {
    return strcmp(razlog, "PLATA_DEO1") == 0 ||
           strcmp(razlog, "PLATA_DEO2") == 0 ||
           strcmp(razlog, "PUTNI_TROSKOVI") == 0 ||
           strcmp(razlog, "BONUS") == 0 ||
           strcmp(razlog, "DNEVNICA") == 0;
}

void kreiraj_praznu_serijsku_isplata_datoteku(const char* filename) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        printf("Greska pri kreiranju datoteke.\n");
        return;
    }

    Isplata blok[FAKTOR_BLOKIRANJA_ISPLATA] = {0};
    blok[0].identifikator = -1;   // EOF marker

    fwrite(blok, sizeof(Isplata), FAKTOR_BLOKIRANJA_ISPLATA, f);
    fclose(f);

    printf("Datoteka \"%s\" uspesno kreirana.\n", filename);
}

void unos_isplate(const char* filename, const char* radnik_file) {
    Isplata nova;

    printf("Unesite identifikator isplate: ");
    scanf("%d", &nova.identifikator);
    ocisti_bafer();

    if (nova.identifikator <= 0) {
        printf("Identifikator mora biti pozitivan.\n");
        return;
    }

    printf("Unesite MBR radnika: ");
    scanf("%d", &nova.mbr);
    ocisti_bafer();

    if (!postoji_radnik(radnik_file, nova.mbr)) {
        printf("Radnik sa datim MBR ne postoji.\n");
        return;
    }

    printf("Unesite mesec: ");
    scanf("%d", &nova.mesec);
    ocisti_bafer();

    printf("Unesite godinu: ");
    scanf("%d", &nova.godina);
    ocisti_bafer();

    printf("Unesite datum isplate (D-M-YYYY): ");
    fgets(nova.datum_isplate, sizeof(nova.datum_isplate), stdin);
    nova.datum_isplate[strcspn(nova.datum_isplate, "\n")] = 0;

    if (!provera_ascii(nova.datum_isplate)) {
        printf("Datum mora biti ASCII.\n");
        return;
    }

    printf("Unesite iznos: ");
    scanf("%f", &nova.iznos);
    ocisti_bafer();

    printf("Unesite razlog isplate: ");
    fgets(nova.razlog, MAX_RAZLOG, stdin);
    nova.razlog[strcspn(nova.razlog, "\n")] = 0;

    if (!validan_razlog(nova.razlog)) {
        printf("Nevalidan razlog isplate.\n");
        return;
    }

    FILE* f = fopen(filename, "rb+");
    if (!f) {
        printf("Datoteka ne postoji.\n");
        return;
    }

    Isplata blok[FAKTOR_BLOKIRANJA_ISPLATA];
    int blok_broj = 0;

    while (fread(blok, sizeof(Isplata), FAKTOR_BLOKIRANJA_ISPLATA, f)
           == FAKTOR_BLOKIRANJA_ISPLATA) {

        for (int i = 0; i < FAKTOR_BLOKIRANJA_ISPLATA; i++) {

            if (blok[i].identifikator == nova.identifikator) {
                printf("Isplata sa datim identifikatorom vec postoji.\n");
                fclose(f);
                return;
            }

            if (blok[i].identifikator == -1) {
                blok[i] = nova;

                if (i < FAKTOR_BLOKIRANJA_ISPLATA - 1)
                    blok[i + 1].identifikator = -1;

                fseek(f, blok_broj * sizeof(blok), SEEK_SET);
                fwrite(blok, sizeof(Isplata), FAKTOR_BLOKIRANJA_ISPLATA, f);

                if (i == FAKTOR_BLOKIRANJA_ISPLATA - 1) {
                    Isplata novi_blok[FAKTOR_BLOKIRANJA_ISPLATA] = {0};
                    novi_blok[0].identifikator = -1;
                    fwrite(novi_blok, sizeof(Isplata),
                           FAKTOR_BLOKIRANJA_ISPLATA, f);
                }

                printf("Isplata upisana (blok %d, pozicija %d).\n", blok_broj, i);
                fclose(f);
                return;
            }
        }
        blok_broj++;
    }
    fclose(f);
}

int postoji_isplata(const char* filename, int identifikator) {
    FILE* f = fopen(filename, "rb");
    if (!f) return 0;

    Isplata blok[FAKTOR_BLOKIRANJA_ISPLATA];

    while (fread(blok, sizeof(Isplata), FAKTOR_BLOKIRANJA_ISPLATA, f)
           == FAKTOR_BLOKIRANJA_ISPLATA) {
        for (int i = 0; i < FAKTOR_BLOKIRANJA_ISPLATA; i++)
            if (blok[i].identifikator == identifikator) {
                fclose(f);
                return 1;
            }
    }
    fclose(f);
    return 0;
}

void ispis_isplata(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return;

    Isplata blok[FAKTOR_BLOKIRANJA_ISPLATA];
    int b = 0;

    while (fread(blok, sizeof(Isplata), FAKTOR_BLOKIRANJA_ISPLATA, f)
           == FAKTOR_BLOKIRANJA_ISPLATA) {

        for (int i = 0; i < FAKTOR_BLOKIRANJA_ISPLATA; i++) {
            if (blok[i].identifikator == -1) {
                fclose(f);
                return;
            }

            printf("%d %d %d %d %s %.2f %s (blok %d, %d)\n",
                   blok[i].identifikator,
                   blok[i].mbr,
                   blok[i].mesec,
                   blok[i].godina,
                   blok[i].datum_isplate,
                   blok[i].iznos,
                   blok[i].razlog,
                   b, i);
        }
        b++;
    }
    fclose(f);
}
