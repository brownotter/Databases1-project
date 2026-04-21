#include "agregat.h"
#include "radnik.h"
#include "utils.h"
#include "isplata.h"
#include "log.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void meni_agregat()
{
    int izbor;

    do {
        printf("\n--- MENI AGREGAT ---\n");
        printf("1. Formiranje datotek\n");
        printf("2. Pretraga po MBR (index + overflow)\n");
        printf("3. Prikaz DN > BONUS\n");
        printf("4. Logicko brisanje\n");
        //printf("5. Insert (test overflow)\n");
        printf("5. Ispis (primarna + overflow)\n");
         //printf("7. Reorganizacija (manual)\n");
        printf("0. Nazad\n");
        printf("Izbor: ");

        scanf("%d", &izbor);

        switch (izbor)
        {
        case 1:
            formiraj_aktivnu_datoteku("radnici.bin", "isplate.bin");
            break;

        case 2: {
            int mbr;
            printf("Unesi MBR: ");
            scanf("%d", &mbr);
            pretraga_sa_overflow(mbr);
            break;
        }

        case 3:
            prikaz_uslova();
            break;

        case 4: {
            int mbr;
            printf("MBR za brisanje: ");
            scanf("%d", &mbr);
            logicko_brisanje_overflow(mbr);
            break;
        }

        /*case 5: {
            Agregat a;

            printf("MBR: "); scanf("%d", &a.mbr);
            printf("Ime: "); scanf("%s", a.ime);
            printf("Prezime: "); scanf("%s", a.prezime);
            printf("Plata: "); scanf("%f", &a.plata);
            printf("Premija: "); scanf("%f", &a.premija);

            a.uk_bonus = 0;
            a.uk_dnevnice = 0;
            a.status = 0;
            a.next = -1;

            insert_agregat(a);
            break;
        }*/

        case 5:
            debug_ispis();
            break;

        //case 7:
          //  reorganizuj_datoteku();
            //break;

        case 0:
            printf("Nazad.\n");
            break;

        default:
            printf("Pogresan izbor!\n");
        }

    } while (izbor != 0);
}

void formiraj_aktivnu_datoteku(const char* radnici, const char* isplate)
{
    FILE *fr = fopen(radnici, "rb");
    FILE *fi = fopen(isplate, "rb");
    FILE *fo = fopen(DATA_FILE, "wb");
    FILE *fover = fopen(OVERFLOW_FILE, "wb");

    if (!fr || !fi || !fo || !fover) {
        printf("Greska!\n");
        return;
    }

    Radnik r;
    Isplata i;

    Agregat blok[F1];
    int idx = 0;

    while (fread(&r, sizeof(Radnik), 1, fr)) {

        Agregat a = {0};

        a.mbr = r.mbr;
        strcpy(a.ime, r.ime);
        strcpy(a.prezime, r.prezime);
        a.plata = r.plata;
        a.premija = r.premija;
        a.status = 0;
        a.next = -1;

        rewind(fi);

        while (fread(&i, sizeof(Isplata), 1, fi)) {

            if (i.mbr == r.mbr) {

                if (strcmp(i.razlog, "BONUS") == 0)
                    a.uk_bonus += i.iznos;

                if (strcmp(i.razlog, "DNEVNICA") == 0)
                    a.uk_dnevnice += i.iznos;
            }
        }

        blok[idx++] = a;

        if (idx == F1) {
            fwrite(blok, sizeof(Agregat), F1, fo);
            idx = 0;
        }
    }

    if (idx > 0) {
    for (int i = idx; i < F1; i++) {
        blok[i].mbr = -1;
        blok[i].status = 0;
        blok[i].next = -1;
    }

    fwrite(blok, sizeof(Agregat), F1, fo);
}
    fclose(fr);
    fclose(fi);
    fclose(fo);
    fclose(fover);

    printf("Primarna zona formirana.\n");

    formiraj_indeks();
}


void formiraj_indeks()
{
    FILE *f = fopen(DATA_FILE, "rb");
    FILE *fi = fopen(INDEX_FILE, "wb");

    if (!f || !fi) return;

    Agregat blok[F1];
    IndexBlok ib = {0};

    long adresa = 0;

    while (fread(blok, sizeof(Agregat), F1, f) == F1) {

        int max = blok[0].mbr;

        for (int i = 1; i < F1; i++)
            if (blok[i].mbr > max)
                max = blok[i].mbr;

        ib.slogovi[ib.broj].max_mbr = max;
        ib.slogovi[ib.broj].adresa_bloka = adresa;

        ib.broj++;

        if (ib.broj == N) {
            fwrite(&ib, sizeof(IndexBlok), 1, fi);
            ib.broj = 0;
        }

        adresa += sizeof(Agregat) * F1;
    }

    if (ib.broj > 0)
        fwrite(&ib, sizeof(IndexBlok), 1, fi);

    fclose(f);
    fclose(fi);

    printf("Indeks formiran.\n");
}

void pretraga_po_mbr(int mbr)
{
    FILE *fi = fopen(INDEX_FILE, "rb");
    FILE *fd = fopen(DATA_FILE, "rb");

    if (!fi || !fd) return;

    IndexBlok ib;
    Agregat blok[F1];

    while (fread(&ib, sizeof(IndexBlok), 1, fi)) {

        for (int i = 0; i < ib.broj; i++) {

            if (mbr <= ib.slogovi[i].max_mbr) {

                fseek(fd, ib.slogovi[i].adresa_bloka, SEEK_SET);
                fread(blok, sizeof(Agregat), F1, fd);

                for (int j = 0; j < F1; j++) {

                    if (blok[j].status == 0 && blok[j].mbr == mbr) {
                        printf("Dnevnica=%.2f | blok %ld poz %d\n",
                               blok[j].uk_dnevnice,
                               ib.slogovi[i].adresa_bloka / sizeof(Agregat),
                               j);
                        fclose(fi);
                        fclose(fd);
                        return;
                    }
                }
            }
        }
    }

    printf("Nije pronadjen.\n");

    fclose(fi);
    fclose(fd);
}


void prikaz_uslova()
{
    FILE *f = fopen(DATA_FILE, "rb");
    if (!f) return;

    Agregat blok[F1];
    int b = 0, found = 0;

    while (fread(blok, sizeof(Agregat), F1, f) == F1) {

        for (int i = 0; i < F1; i++) {

            if (blok[i].status == 1) continue;

            if (blok[i].uk_dnevnice > blok[i].uk_bonus) {
                printf("MBR %d [%d,%d]\n", blok[i].mbr, b, i);
                found = 1;
            }
        }
        b++;
    }

    if (!found) printf("Nema rezultata.\n");

    fclose(f);
}


void logicko_brisanje(int mbr)
{
    FILE *f = fopen(DATA_FILE, "rb+");
    if (!f) return;

    Agregat blok[F1];

    while (fread(blok, sizeof(Agregat), F1, f) == F1) {

        for (int i = 0; i < F1; i++) {

            if (blok[i].mbr == mbr) {

                blok[i].status = 1;

                fseek(f, -(long)sizeof(Agregat)*F1, SEEK_CUR);
                fwrite(blok, sizeof(Agregat), F1, f);

                printf("Obrisan.\n");
                fclose(f);
                return;
            }
        }
    }

    printf("Nije pronadjen.\n");
    fclose(f);
}

int read_overflow(FILE *f, long pos, Agregat *a)
{
    fseek(f, pos, SEEK_SET);
    return fread(a, sizeof(Agregat), 1, f);
}

long write_overflow(FILE *f, Agregat *a)
{
    fseek(f, 0, SEEK_END);
    long pos = ftell(f);

    fwrite(a, sizeof(Agregat), 1, f);

    return pos;
}

void insert_agregat(Agregat novi)
{
    FILE *fd = fopen(DATA_FILE, "rb+");
    FILE *fo = fopen(OVERFLOW_FILE, "rb+");

    if (!fd || !fo) {
        printf("Greska fajl\n");
        return;
    }

    Agregat blok[F1];

    while (fread(blok, sizeof(Agregat), F1, fd) == F1) {

        // pokušaj ubacivanja u blok
        for (int i = 0; i < F1; i++) {

            if (blok[i].status == 1 || blok[i].mbr == 0) {

                blok[i] = novi;
                blok[i].next = -1;

                fseek(fd, -(long)sizeof(Agregat)*F1, SEEK_CUR);
                fwrite(blok, sizeof(Agregat), F1, fd);

                printf("Upis u primarnu zonu.\n");
                fclose(fd); fclose(fo);
                return;
            }
        }

        // nema mesta - ide overflow NA PRVI SLOG BLOKA
        Agregat *glava = &blok[0];

        // ako nema lanac
        if (glava->next == -1) {

            novi.next = -1;
            long pos = write_overflow(fo, &novi);

            glava->next = pos;

            fseek(fd, -(long)sizeof(Agregat)*F1, SEEK_CUR);
            fwrite(blok, sizeof(Agregat), F1, fd);

            printf("Napravljen overflow lanac.\n");
            fclose(fd); fclose(fo);
            return;
        }

        // postoji lanac - idi do kraja
        long current = glava->next;
        Agregat temp;

        while (1) {
            read_overflow(fo, current, &temp);

            if (temp.next == -1) break;

            current = temp.next;
        }

        novi.next = -1;
        long pos = write_overflow(fo, &novi);

        temp.next = pos;

        fseek(fo, current, SEEK_SET);
        fwrite(&temp, sizeof(Agregat), 1, fo);

        printf("Dodat u overflow lanac.\n");
        fclose(fd); fclose(fo);
        return;
    }

    fclose(fd);
    fclose(fo);
}


void pretraga_sa_overflow(int mbr)
{
    FILE *fd = fopen(DATA_FILE, "rb");
    FILE *fo = fopen(OVERFLOW_FILE, "rb");

    if (!fd || !fo) return;

    Agregat blok[F1];

    while (fread(blok, sizeof(Agregat), F1, fd) == F1) {

        for (int i = 0; i < F1; i++) {

            if (blok[i].status == 1) continue;

            if (blok[i].mbr == mbr) {
                printf("Nadjen u primarnoj Dnevnica=%.2f\n", blok[i].uk_dnevnice);
                fclose(fd); fclose(fo);
                return;
            }

            // overflow lanac
            long next = blok[i].next;
            Agregat temp;

            while (next != -1) {

                read_overflow(fo, next, &temp);

                if (temp.mbr == mbr) {
                    printf("Nadjen u OVERFLOW Dnevnica=%.2f\n", temp.uk_dnevnice);
                    fclose(fd); fclose(fo);
                    return;
                }

                next = temp.next;
            }
        }
    }

    printf("Nije pronadjen.\n");
    fclose(fd);
    fclose(fo);
}

void logicko_brisanje_overflow(int mbr)
{
    FILE *fd = fopen(DATA_FILE, "rb+");
    FILE *fo = fopen(OVERFLOW_FILE, "rb+");

    Agregat blok[F1];

    while (fread(blok, sizeof(Agregat), F1, fd) == F1) {

        for (int i = 0; i < F1; i++) {

            if (blok[i].mbr == mbr) {
                blok[i].status = 1;

                fseek(fd, -(long)sizeof(Agregat)*F1, SEEK_CUR);
                fwrite(blok, sizeof(Agregat), F1, fd);

                printf("Obrisan u primarnoj.\n");
                return;
            }

            long next = blok[i].next;
            Agregat temp;

            while (next != -1) {

                read_overflow(fo, next, &temp);

                if (temp.mbr == mbr) {
                    temp.status = 1;

                    fseek(fo, next, SEEK_SET);
                    fwrite(&temp, sizeof(Agregat), 1, fo);

                    printf("Obrisan u overflow.\n");
                    return;
                }

                next = temp.next;
            }
        }
    }


}


void debug_ispis()
{
    FILE *fd = fopen(DATA_FILE, "rb");
    FILE *fo = fopen(OVERFLOW_FILE, "rb");

    if (!fd || !fo) {
        printf("Greska pri otvaranju.\n");
        return;
    }

    Agregat blok[F1];
    int b = 0;

    printf("\n--- PRIMARNA ZONA ---\n");

    while (fread(blok, sizeof(Agregat), F1, fd) == F1) {

        printf("Blok %d:\n", b);

        for (int i = 0; i < F1; i++) {
            printf(" [%d] MBR=%d Dnevnica=%.2f NEXT=%ld STATUS=%d\n",
                   i,
                   blok[i].mbr,
                   blok[i].uk_dnevnice,
                   blok[i].next,
                   blok[i].status);
        }
        b++;
    }

    printf("\n--- OVERFLOW ZONA ---\n");

    Agregat a;
    int i = 0;

    rewind(fo);

    while (fread(&a, sizeof(Agregat), 1, fo) == 1) {
        printf(" [%d] MBR=%d DN=%.2f NEXT=%ld\n",
               i++, a.mbr, a.uk_dnevnice, a.next);
    }

    fclose(fd);
    fclose(fo);
}


void update_agregat(Agregat novi)
{
    FILE *f = fopen(DATA_FILE, "rb+");
    Agregat blok[F1];

    while (fread(blok, sizeof(Agregat), F1, f)) {
        for (int i = 0; i < F1; i++) {
            if (blok[i].mbr == novi.mbr) {

                strcpy(blok[i].ime, novi.ime);
                strcpy(blok[i].prezime, novi.prezime);
                blok[i].plata = novi.plata;
                blok[i].premija = novi.premija;

                fseek(f, -(long)sizeof(Agregat)*F1, SEEK_CUR);
                fwrite(blok, sizeof(Agregat), F1, f);

                upisi_log(novi.mbr, "UPDATE_AGREGAT",1);

                fclose(f);
                return;
            }
        }
    }

    fclose(f);
}

void azuriraj_agregat_isplata(Isplata i)
{
    FILE *f = fopen(DATA_FILE, "rb+");
    Agregat blok[F1];

    while (fread(blok, sizeof(Agregat), F1, f)) {
        for (int j = 0; j < F1; j++) {

            if (blok[j].mbr == i.mbr) {

                if (strcmp(i.razlog, "BONUS") == 0)
                    blok[j].uk_bonus += i.iznos;

                if (strcmp(i.razlog, "DNEVNICA") == 0)
                    blok[j].uk_dnevnice += i.iznos;

                fseek(f, -(long)sizeof(Agregat)*F1, SEEK_CUR);
                fwrite(blok, sizeof(Agregat), F1, f);

                upisi_log(i.mbr, "UPDATE_ISPLATA",1);

                fclose(f);
                return;
            }
        }
    }

    fclose(f);
}

// cisti se datoteka, brisu se logicki obrisani slogovi, ako je status=1
void reorganizuj_datoteku()
{
    FILE *f = fopen(DATA_FILE, "rb");
    FILE *temp = fopen("temp.bin", "wb");

    if (!f || !temp) {
        printf("Greska reorganizacija.\n");
        return;
    }

    Agregat svi[1000]; // dovoljno za projekat
    int n = 0;

    Agregat blok[F1];

    // 1. ucitaj sve iz primarne
    while (fread(blok, sizeof(Agregat), F1, f) == F1) {
        for (int i = 0; i < F1; i++) {
            if (blok[i].status == 0 && blok[i].mbr > 0) {
                svi[n++] = blok[i];
            }
        }
    }

    fclose(f);

    // 2. ucitaj overflow
    FILE *fo = fopen(OVERFLOW_FILE, "rb");
    if (fo) {
        Agregat a;
        while (fread(&a, sizeof(Agregat), 1, fo)) {
            if (a.status == 0) {
                svi[n++] = a;
            }
        }
        fclose(fo);
    }

    // 3. sortiranje po MBR
    for (int i = 0; i < n-1; i++) {
        for (int j = i+1; j < n; j++) {
            if (svi[i].mbr > svi[j].mbr) {
                Agregat tmp = svi[i];
                svi[i] = svi[j];
                svi[j] = tmp;
            }
        }
    }

    // 4. ponovno formiranje primarne zone
    Agregat blok2[F1];
    int idx = 0;

    for (int i = 0; i < n; i++) {

        svi[i].next = -1;

        blok2[idx++] = svi[i];

        if (idx == F1) {
            fwrite(blok2, sizeof(Agregat), F1, temp);
            idx = 0;
        }
    }

    if (idx > 0)
        fwrite(blok2, sizeof(Agregat), idx, temp);

    fclose(temp);

    // 5. zameni staru datoteku
    remove(DATA_FILE);
    rename("temp.bin", DATA_FILE);

    // 6. reset overflow
    FILE *clear = fopen(OVERFLOW_FILE, "wb");
    fclose(clear);

    // 7. ponovo napravi indeks
    formiraj_indeks();

    printf("Reorganizacija zavrsena.\n");
}


static void dodaj_u_novi_blok(FILE *f, Agregat *blok, int *idx, Agregat a)
{
    blok[*idx] = a;
    (*idx)++;

    if (*idx == F1) {
        fwrite(blok, sizeof(Agregat), F1, f);
        *idx = 0;
        memset(blok, 0, sizeof(Agregat) * F1);
    }
}
