#include "isplata.h"
#include "radnik.h"
#include "utils.h"
#include "log.h"
#include "agregat.h"

#include <string.h>
#include <stdlib.h>

void meni_isplata() {
    int izbor;
    char naziv_fajla[64] = "isplate.bin";

    do {
        printf("\n--- MENI ISPLATE ---\n");
        printf("Aktivna datoteka: %s\n", naziv_fajla);
        printf("1. Izaberi naziv fajla\n");
        printf("2. Kreiraj praznu datoteku (f=5)\n");
        printf("3. Unos isplate\n");
        printf("4. Ispis svih isplata\n");
        printf("0. Nazad\n");
        printf("Izbor: ");

        scanf("%d", &izbor);
        ocisti_bafer();

        switch (izbor) {
            case 1:
                printf("Unesite ime fajla: ");
                fgets(naziv_fajla, sizeof(naziv_fajla), stdin);
                naziv_fajla[strcspn(naziv_fajla, "\n")] = 0;
                break;

            case 2:
                kreiraj_praznu_datoteku_isplata(naziv_fajla);
                break;

            case 3:
                unos_isplate(naziv_fajla);
                break;

            case 4:
                ispis_isplata(naziv_fajla);
                break;

            case 0:
                printf("Nazad.\n");
                break;

            default:
                printf("Nepoznat izbor!\n");
        }

    } while (izbor != 0);
}

void kreiraj_praznu_datoteku_isplata(const char* filename) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        printf("Greska!\n");
        return;
    }

    BlokIsplata blok = {0};
    blok.slogovi[0].identifikator = -1;

    fwrite(&blok, sizeof(BlokIsplata), 1, f);
    fclose(f);

    printf("Kreirana datoteka isplata.\n");
}

int validan_razlog(const char* r) {
    return strcmp(r, "PLATA_DEO1") == 0 ||
           strcmp(r, "PLATA_DEO2") == 0 ||
           strcmp(r, "PUTNI_TROSKOVI") == 0 ||
           strcmp(r, "BONUS") == 0 ||
           strcmp(r, "DNEVNICA") == 0;
}

void unos_isplate(const char* filename) {
    FILE* f = fopen(filename, "rb+");
    if (!f) {
        printf("Greska!\n");
        return;
    }

    Isplata nova;

    //unos podataka
    printf("ID isplate: ");
    scanf("%d", &nova.identifikator); ocisti_bafer();

    printf("MBR radnika: ");
    scanf("%d", &nova.mbr); ocisti_bafer();

    printf("Mesec: ");
    scanf("%d", &nova.mesec); ocisti_bafer();

    printf("Godina: ");
    scanf("%d", &nova.godina); ocisti_bafer();

    printf("Datum (D-M-YYYY): ");
    fgets(nova.datum_isplate, MAX_DATUM, stdin);
    nova.datum_isplate[strcspn(nova.datum_isplate, "\n")] = 0;

    printf("Iznos: ");
    scanf("%f", &nova.iznos); ocisti_bafer();

    printf("Razlog: ");
    fgets(nova.razlog, MAX_RAZLOG, stdin);
    nova.razlog[strcspn(nova.razlog, "\n")] = 0;

    if (!validan_razlog(nova.razlog)) {
        printf("Nevalidan razlog!\n");
        fclose(f);
        return;
    }

    BlokIsplata blok;
    int blok_index = 0;

    while (fread(&blok, sizeof(BlokIsplata), 1, f) == 1) {

        for (int i = 0; i < FAKTOR_BLOKIRANJA_ISPLATA; i++) {

            //  provera da li postoji uneseni id
            if (blok.slogovi[i].identifikator == nova.identifikator) {
                printf("ID vec postoji!\n");
                fclose(f);
                return;
            }

            // EOF
            if (blok.slogovi[i].identifikator == -1) {

                blok.slogovi[i] = nova;

                if (i < FAKTOR_BLOKIRANJA_ISPLATA - 1)
                    blok.slogovi[i + 1].identifikator = -1;

                fseek(f, -sizeof(BlokIsplata), SEEK_CUR);
                fwrite(&blok, sizeof(BlokIsplata), 1, f);

                if (i == FAKTOR_BLOKIRANJA_ISPLATA - 1) {
                    BlokIsplata novi = {0};
                    novi.slogovi[0].identifikator = -1;
                    fwrite(&novi, sizeof(BlokIsplata), 1, f);
                }

                printf("Upisana isplata (blok %d, pozicija %d)\n", blok_index, i);
                upisi_log(nova.mbr, "INSERT_ISPLATA",1);
                fclose(f);
                return;
            }
        }

        blok_index++;
    }

    printf("Greska: EOF nije pronadjen.\n");
    fclose(f);
}

void ispis_isplata(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        printf("Greska!\n");
        return;
    }

    BlokIsplata blok;
    int blok_index = 0;

    while (fread(&blok, sizeof(BlokIsplata), 1, f) == 1) {
        for (int i = 0; i < FAKTOR_BLOKIRANJA_ISPLATA; i++) {

            if (blok.slogovi[i].identifikator == -1) {
                fclose(f);
                return;
            }

            printf("[%d,%d] ID:%d MBR:%d %d/%d %s Iznos:%.2f Razlog:%s\n",
                   blok_index, i,
                   blok.slogovi[i].identifikator,
                   blok.slogovi[i].mbr,
                   blok.slogovi[i].mesec,
                   blok.slogovi[i].godina,
                   blok.slogovi[i].datum_isplate,
                   blok.slogovi[i].iznos,
                   blok.slogovi[i].razlog);
        }
        blok_index++;
    }

    fclose(f);
}
