#include "radnik.h"
#include "utils.h"
#include "isplata.h"
#include <string.h>
#include <stdlib.h>

void kreiraj_praznu_serijsku_radnik_datoteku(const char* filename) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        printf("Greska pri kreiranju datoteke.\n");
        return;
    }

    Radnik blok[FAKTOR_BLOKIRANJA_RADNIK] = {0};
    blok[0].mbr = -1;   // EOF marker

    fwrite(blok, sizeof(Radnik), FAKTOR_BLOKIRANJA_RADNIK, f);
    fclose(f);

    printf("Datoteka \"%s\" uspesno kreirana.\n", filename);
}

void unos_radnika(const char* filename) {
    Radnik novi;

    printf("Unesite MBR: ");
    scanf("%d", &novi.mbr);
    ocisti_bafer();

    if (novi.mbr <= 0) {
        printf("MBR mora biti pozitivan.\n");
        return;
    }

    printf("Unesite ime: ");
    fgets(novi.ime, MAX_IME, stdin);
    novi.ime[strcspn(novi.ime, "\n")] = 0;

    printf("Unesite prezime: ");
    fgets(novi.prezime, MAX_PREZIME, stdin);
    novi.prezime[strcspn(novi.prezime, "\n")] = 0;

    if (!provera_ascii(novi.ime) || !provera_ascii(novi.prezime)) {
        printf("Dozvoljeni su samo ASCII karakteri.\n");
        return;
    }

    printf("Unesite platu: ");
    scanf("%f", &novi.plata);
    ocisti_bafer();

    printf("Unesite premiju: ");
    scanf("%f", &novi.premija);
    ocisti_bafer();

    FILE* f = fopen(filename, "rb+");
    if (!f) {
        printf("Datoteka ne postoji.\n");
        return;
    }

    Radnik blok[FAKTOR_BLOKIRANJA_RADNIK];
    int blok_broj = 0;

    while (fread(blok, sizeof(Radnik), FAKTOR_BLOKIRANJA_RADNIK, f)
           == FAKTOR_BLOKIRANJA_RADNIK) {

        for (int i = 0; i < FAKTOR_BLOKIRANJA_RADNIK; i++) {

            if (blok[i].mbr == novi.mbr) {
                printf("Radnik sa datim MBR vec postoji.\n");
                fclose(f);
                return;
            }

            if (blok[i].mbr == -1) {
                blok[i] = novi;

                if (i < FAKTOR_BLOKIRANJA_RADNIK - 1)
                    blok[i + 1].mbr = -1;

                fseek(f, blok_broj * sizeof(blok), SEEK_SET);
                fwrite(blok, sizeof(Radnik), FAKTOR_BLOKIRANJA_RADNIK, f);

                if (i == FAKTOR_BLOKIRANJA_RADNIK - 1) {
                    Radnik novi_blok[FAKTOR_BLOKIRANJA_RADNIK] = {0};
                    novi_blok[0].mbr = -1;
                    fwrite(novi_blok, sizeof(Radnik), FAKTOR_BLOKIRANJA_RADNIK, f);
                }

                printf("Radnik upisan (blok %d, pozicija %d).\n", blok_broj, i);
                fclose(f);
                return;
            }
        }
        blok_broj++;
    }

    fclose(f);
}

void prikaz_radnika_po_mbr(const char* filename, int mbr) {
    FILE* f = fopen(filename, "rb");
    if (!f) return;

    Radnik blok[FAKTOR_BLOKIRANJA_RADNIK];
    int blok_broj = 0;

    while (fread(blok, sizeof(Radnik), FAKTOR_BLOKIRANJA_RADNIK, f)
           == FAKTOR_BLOKIRANJA_RADNIK) {

        for (int i = 0; i < FAKTOR_BLOKIRANJA_RADNIK; i++) {

            if (blok[i].mbr == -1) {
                printf("Radnik nije pronadjen.\n");
                fclose(f);
                return;
            }

            if (blok[i].mbr == mbr) {
                printf("Ime: %s\nPrezime: %s\nPlata: %.2f\nPremija: %.2f\n",
                       blok[i].ime, blok[i].prezime,
                       blok[i].plata, blok[i].premija);
                printf("Blok: %d, Pozicija: %d\n", blok_broj, i);
                fclose(f);
                return;
            }
        }
        blok_broj++;
    }
    fclose(f);
}

int postoji_radnik(const char* filename, int mbr) {
    FILE* f = fopen(filename, "rb");
    if (!f) return 0;

    Radnik blok[FAKTOR_BLOKIRANJA_RADNIK];

    while (fread(blok, sizeof(Radnik), FAKTOR_BLOKIRANJA_RADNIK, f)
           == FAKTOR_BLOKIRANJA_RADNIK) {
        for (int i = 0; i < FAKTOR_BLOKIRANJA_RADNIK; i++)
            if (blok[i].mbr == mbr) {
                fclose(f);
                return 1;
            }
    }
    fclose(f);
    return 0;
}

void ispis_radnika(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return;

    Radnik blok[FAKTOR_BLOKIRANJA_RADNIK];
    int b = 0;

    while (fread(blok, sizeof(Radnik), FAKTOR_BLOKIRANJA_RADNIK, f)
           == FAKTOR_BLOKIRANJA_RADNIK) {

        for (int i = 0; i < FAKTOR_BLOKIRANJA_RADNIK; i++) {
            if (blok[i].mbr == -1) {
                fclose(f);
                return;
            }
            printf("%d %s %s %.2f %.2f (blok %d, %d)\n",
                   blok[i].mbr, blok[i].ime, blok[i].prezime,
                   blok[i].plata, blok[i].premija, b, i);
        }
        b++;
    }
    fclose(f);
}

void radnici_sa_bonusima(const char* radnik_file, const char* isplata_file) {
    FILE* fr = fopen(radnik_file, "rb");
    FILE* fi = fopen(isplata_file, "rb");

    if (!fr || !fi) {
        printf("Greska pri otvaranju datoteka.\n");
        if (fr) fclose(fr);
        if (fi) fclose(fi);
        return;
    }

    Radnik blok_radnik[FAKTOR_BLOKIRANJA_RADNIK];
    Isplata blok_isplata[FAKTOR_BLOKIRANJA_ISPLATA];

    int blok_broj = 0;
    int pronadjen = 0;

    while (fread(blok_radnik, sizeof(Radnik),
                 FAKTOR_BLOKIRANJA_RADNIK, fr)
           == FAKTOR_BLOKIRANJA_RADNIK) {

        for (int i = 0; i < FAKTOR_BLOKIRANJA_RADNIK; i++) {

            if (blok_radnik[i].mbr == -1) {
                fclose(fr);
                fclose(fi);
                if (!pronadjen)
                    printf("Nema radnika koji ispunjavaju uslov.\n");
                return;
            }

            int ima_bonus = 0;

            /* OBAVEZNO: vracanje pokazivaca na pocetak isplata */
            fseek(fi, 0, SEEK_SET);

            while (fread(blok_isplata, sizeof(Isplata),
                         FAKTOR_BLOKIRANJA_ISPLATA, fi)
                   == FAKTOR_BLOKIRANJA_ISPLATA) {

                for (int j = 0; j < FAKTOR_BLOKIRANJA_ISPLATA; j++) {

                    if (blok_isplata[j].identifikator == -1)
                        break;

                    if (blok_isplata[j].mbr == blok_radnik[i].mbr &&
                        strcmp(blok_isplata[j].razlog, "BONUS") == 0 &&
                        blok_isplata[j].iznos > 2000) {

                        ima_bonus = 1;
                        break;
                    }
                }
                if (ima_bonus) break;
            }

            if (ima_bonus) {
                printf("---------------------------------------------\n");
                printf("MBR: %d\n", blok_radnik[i].mbr);
                printf("Ime: %s\n", blok_radnik[i].ime);
                printf("Prezime: %s\n", blok_radnik[i].prezime);
                printf("Plata: %.2f\n", blok_radnik[i].plata);
                printf("Premija: %.2f\n", blok_radnik[i].premija);
                printf("Blok: %d | Pozicija: %d\n", blok_broj, i);
                pronadjen = 1;
            }
        }
        blok_broj++;
    }

    if (!pronadjen)
        printf("Nema radnika koji ispunjavaju uslov.\n");

    fclose(fr);
    fclose(fi);
}

void izmeni_radnika(const char* filename, int mbr) {
    FILE* f = fopen(filename, "rb+");
    if (!f) {
        printf("Datoteka ne postoji.\n");
        return;
    }

    Radnik blok[FAKTOR_BLOKIRANJA_RADNIK];
    int blok_broj = 0;

    while (fread(blok, sizeof(Radnik),
                 FAKTOR_BLOKIRANJA_RADNIK, f)
           == FAKTOR_BLOKIRANJA_RADNIK) {

        for (int i = 0; i < FAKTOR_BLOKIRANJA_RADNIK; i++) {

            if (blok[i].mbr == -1) {
                printf("Radnik nije pronadjen.\n");
                fclose(f);
                return;
            }

            if (blok[i].mbr == mbr) {
                printf("Unesite novo ime: ");
                fgets(blok[i].ime, sizeof(blok[i].ime), stdin);
                blok[i].ime[strcspn(blok[i].ime, "\n")] = 0;

                printf("Unesite novo prezime: ");
                fgets(blok[i].prezime, sizeof(blok[i].prezime), stdin);
                blok[i].prezime[strcspn(blok[i].prezime, "\n")] = 0;

                printf("Unesite novu platu: ");
                scanf("%f", &blok[i].plata);
                ocisti_bafer();

                printf("Unesite novu premiju: ");
                scanf("%f", &blok[i].premija);
                ocisti_bafer();

                fseek(f, blok_broj * sizeof(blok), SEEK_SET);
                fwrite(blok, sizeof(Radnik),
                       FAKTOR_BLOKIRANJA_RADNIK, f);

                printf("Radnik uspesno izmenjen (blok %d, pozicija %d).\n",
                       blok_broj, i);
                fclose(f);
                return;
            }
        }
        blok_broj++;
    }

    printf("Radnik nije pronadjen.\n");
    fclose(f);
}


