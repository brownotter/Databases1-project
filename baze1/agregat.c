#include "agregat.h"
#include "radnik.h"
#include "utils.h"
#include "isplata.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void meni_agregat()
{
    int izbor;

    do {
        printf("\n===== AGREGIRANI MENI =====\n");
        printf("1. Formiranje agregirane datoteke\n");
        printf("2. Prikaz po MBR (dnevnice)\n");
        printf("3. Prikaz uslova (DN > BONUS)\n");
        printf("4. Logicko brisanje\n");
        printf("5. Pretraga sa indeksom (ako imas)\n");
        printf("0. Nazad\n");
        printf("Izbor: ");
        scanf("%d", &izbor);

        switch (izbor) {

        case 1:
            formiraj_agregiranu_datoteku(
                "radnici.bin",
                "isplate.bin",
                "agregat.bin"
            );
            break;

        case 2: {
            int mbr;
            printf("Unesi MBR: ");
            scanf("%d", &mbr);
            prikaz_dnevnice_po_mbr("agregat.bin", mbr);
            break;
        }

        case 3:
            prikaz_uslova("agregat.bin");
            break;

        case 4: {
            int mbr;
            printf("MBR za brisanje: ");
            scanf("%d", &mbr);
            logicko_brisanje_agregat("agregat.bin", mbr);
            break;
        }

        case 5: {
            int mbr;
            printf("MBR pretraga: ");
            scanf("%d", &mbr);
            pretraga_sa_overflow(fopen("agregat.bin","rb"), mbr);
            break;
        }

        case 0:
            break;

        default:
            printf("Pogresan izbor.\n");
        }

    } while (izbor != 0);
}

void formiraj_agregiranu_datoteku(
    const char* radnici_fajl,
    const char* isplate_fajl,
    const char* izlazni_fajl)
{
    FILE *fr = fopen(radnici_fajl, "rb");
    FILE *fi = fopen(isplate_fajl, "rb");
    FILE *fo = fopen(izlazni_fajl, "wb");

    if (!fr || !fi || !fo) {
        printf("Greska pri otvaranju fajlova!\n");
        return;
    }

    Radnik r;
    Isplata i;

    while (fread(&r, sizeof(Radnik), 1, fr) == 1) {

        Agregat a;
        a.mbr = r.mbr;
        strcpy(a.ime, r.ime);
        strcpy(a.prezime, r.prezime);
        a.plata = r.plata;
        a.premija = r.premija;

        a.uk_bonus = 0;
        a.uk_dnevnice = 0;
        a.status = 0;

        rewind(fi);

        while (fread(&i, sizeof(Isplata), 1, fi) == 1) {

            if (i.mbr == r.mbr) {

                if (strcmp(i.razlog, "BONUS") == 0) {
                    a.uk_bonus += i.iznos;
                }

                if (strcmp(i.razlog, "DNEVNICA") == 0) {
                    a.uk_dnevnice += i.iznos;
                }
            }
        }

        fwrite(&a, sizeof(Agregat), 1, fo);
    }

    fclose(fr);
    fclose(fi);
    fclose(fo);

    printf("Agregacija uspesno zavrsena.\n");
}

void prikaz_dnevnice_po_mbr(const char* filename, int mbr)
{
    FILE *f = fopen(filename, "rb");
    if (!f) return;

    Agregat blok[F1];
    int b = 0;

    while (fread(blok, sizeof(Agregat), F1, f) == F1) {
        for (int i = 0; i < F1; i++) {

            if (blok[i].status == 1)
                continue;

            if (blok[i].mbr == mbr) {
                printf("DN: %.2f | blok %d poz %d\n",
                       blok[i].uk_dnevnice, b, i);
                fclose(f);
                return;
            }
        }
        b++;
    }

    printf("Nije pronadjen radnik.\n");
    fclose(f);
}


// =====================================
// 3. USLOV DN > BONUS
// =====================================
void prikaz_uslova(const char* filename)
{
    FILE *f = fopen(filename, "rb");
    if (!f) return;

    Agregat blok[F1];
    int b = 0;
    int found = 0;

    while (fread(blok, sizeof(Agregat), F1, f) == F1) {
        for (int i = 0; i < F1; i++) {

            if (blok[i].status == 1)
                continue;

            if (blok[i].uk_dnevnice > blok[i].uk_bonus) {
                printf("MBR %d | DN %.2f > BONUS %.2f | [%d,%d]\n",
                       blok[i].mbr,
                       blok[i].uk_dnevnice,
                       blok[i].uk_bonus,
                       b, i);

                found = 1;
            }
        }
        b++;
    }

    if (!found)
        printf("Nema rezultata.\n");

    fclose(f);
}


// =====================================
// 4. LOGICKO BRISANJE
// =====================================
void logicko_brisanje_agregat(const char* filename, int mbr)
{
    FILE *f = fopen(filename, "rb+");
    if (!f) return;

    Agregat blok[F1];
    int b = 0;

    while (fread(blok, sizeof(Agregat), F1, f) == F1) {
        for (int i = 0; i < F1; i++) {

            if (blok[i].mbr == mbr) {
                blok[i].status = 1;

                fseek(f, -(long)sizeof(Agregat)*F1, SEEK_CUR);
                fwrite(blok, sizeof(Agregat), F1, f);

                printf("Obrisan mbr %d\n", mbr);
                fclose(f);
                return;
            }
        }
        b++;
    }

    printf("Nije pronadjen.\n");
    fclose(f);
}

void formiraj_indeks(FILE *f)
{
    rewind(f);

    Agregat blok[5];
    IndexBlok iblok = {0};

    long adresa = 0;

    while (fread(blok, sizeof(Agregat), 5, f) == 5) {

        // uzmi MAX mbr iz bloka
        int max_mbr = blok[0].mbr;

        for (int i = 1; i < 5; i++) {
            if (blok[i].mbr > max_mbr)
                max_mbr = blok[i].mbr;
        }

        // upis u indeks
        iblok.slogovi[iblok.broj].mbr = max_mbr;
        iblok.slogovi[iblok.broj].adresa_bloka = adresa;

        iblok.broj++;

        if (iblok.broj == N) {
            fwrite(&iblok, sizeof(IndexBlok), 1, f);
            iblok.broj = 0;
        }

        adresa += sizeof(Agregat) * 5;
    }

    if (iblok.broj > 0)
        fwrite(&iblok, sizeof(IndexBlok), 1, f);
}

void pretraga_po_mbr(const char* filename, int mbr)
{
    FILE *f = fopen(filename, "rb");
    if (!f) return;

    // prvo preskoci primarnu zonu (moramo znati koliko blokova ima)
    fseek(f, 0, SEEK_SET);

    Agregat blok[5];

    // pronalazak u indeks zoni
    IndexBlok iblok;

    // skok na kraj (u realnom projektu zna se offset, ovde pojednostavljeno)
    fseek(f, 0, SEEK_END);
    long size = ftell(f);

    rewind(f);

    while (fread(&iblok, sizeof(IndexBlok), 1, f)) {

        for (int i = 0; i < iblok.broj; i++) {

            if (mbr <= iblok.slogovi[i].mbr) {

                long adr = iblok.slogovi[i].adresa_bloka;

                fseek(f, adr, SEEK_SET);
                fread(blok, sizeof(Agregat), 5, f);

                for (int j = 0; j < 5; j++) {
                    if (blok[j].mbr == mbr) {
                        printf("Nadjeno DN=%.2f BL=%d POZ=%d\n",
                               blok[j].uk_dnevnice, i, j);
                        fclose(f);
                        return;
                    }
                }
            }
        }
    }

    printf("Nije pronadjeno.\n");
    fclose(f);
}

void upis_agregat_sa_overflow(FILE *f, Agregat novi)
{
    Agregat blok[F1];
    long pos;

    rewind(f);

    while (fread(blok, sizeof(Agregat), F1, f) == F1) {

        for (int i = 0; i < F1; i++) {

            if (blok[i].status == 0 && blok[i].mbr == 0) {

                // slobodno mesto = upis
                blok[i] = novi;
                blok[i].next = -1;

                fseek(f, -sizeof(Agregat)*F1, SEEK_CUR);
                fwrite(blok, sizeof(Agregat), F1, f);

                return;
            }
        }
    }

    // PREKORAÈENJE
    fseek(f, 0, SEEK_END);
    novi.next = -1;
    fwrite(&novi, sizeof(Agregat), 1, f);
}

int dodaj_u_lanac(FILE *f, Agregat *glava, Agregat novi)
{
    while (glava->next != -1) {
        fseek(f, glava->next, SEEK_SET);
        fread(glava, sizeof(Agregat), 1, f);
    }

    long pos = ftell(f);
    fwrite(&novi, sizeof(Agregat), 1, f);

    glava->next = pos;
    return 1;
}

void pretraga_sa_overflow(FILE *f, int mbr)
{
    Agregat a;

    rewind(f);

    while (fread(&a, sizeof(Agregat), 1, f) == 1) {

        if (a.status == 0 && a.mbr == mbr) {
            printf("DN: %.2f | %.2f\n", a.uk_dnevnice, a.uk_bonus);
            return;
        }

        // ako postoji lanac
        int next = a.next;

        while (next != -1) {
            fseek(f, next, SEEK_SET);
            fread(&a, sizeof(Agregat), 1, f);

            if (a.mbr == mbr) {
                printf("DN: %.2f (OVERFLOW)\n", a.uk_dnevnice);
                return;
            }

            next = a.next;
        }
    }

    printf("Nije pronadjen.\n");
}


