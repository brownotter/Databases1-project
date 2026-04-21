#include "radnik.h"
#include "utils.h"
#include "isplata.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void meni_radnik() {
    int izbor;
    char naziv_fajla[64] = "radnici.bin";

    do {
        printf("\n--- MENI RADNIK ---\n");
        printf("Aktivna datoteka: %s\n", naziv_fajla);
        printf("1. Izaberi naziv fajla\n");
        printf("2. Kreiraj praznu datoteku (f=5)\n");
        printf("3. Unos radnika\n");
        printf("4. Prikaz po MBR\n");
        printf("5. Modifikacija radnika\n");
        printf("6. Ispis svih radnika\n");
        printf("7. Uslov: BONUS > 2000\n");
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
                kreiraj_praznu_datoteku_radnik(naziv_fajla);
                break;

            case 3:
                unos_radnika(naziv_fajla);
                break;

            case 4: {
                int mbr;
                printf("Unesite mbr: ");
                scanf("%d", &mbr);
                ocisti_bafer();
                prikaz_radnika(naziv_fajla, mbr);
                break;
            }

            case 5: {
                int mbr;
                printf("Unesite mbr: ");
                scanf("%d", &mbr);
                ocisti_bafer();
                modifikacija_radnika(naziv_fajla, mbr);
                break;
            }

            case 6:
                ispis_radnika(naziv_fajla);
                break;

            case 7: {
                char fajl_isplate[64];
                printf("Unesite ime fajla sa isplatama: ");
                fgets(fajl_isplate, sizeof(fajl_isplate), stdin);
                fajl_isplate[strcspn(fajl_isplate, "\n")] = 0;
                uslov_bonus(naziv_fajla, fajl_isplate);
                break;
            }

            case 0:
                printf("Nazad.\n");
                break;

            default:
                printf("Nepoznat izbor!\n");
        }

    } while (izbor != 0);
}


void unos_radnika(const char* filename) {
    FILE* f = fopen(filename, "rb+");
    if (!f) {
        printf("Greska pri otvaranju fajla!\n");
        return;
    }

    Radnik novi;

    //Unos novog radnika
    printf("Unesite mbr: ");
    scanf("%d", &novi.mbr); ocisti_bafer();

    printf("Ime: ");
    fgets(novi.ime, MAX_IME, stdin);
    novi.ime[strcspn(novi.ime, "\n")] = 0;

    printf("Prezime: ");
    fgets(novi.prezime, MAX_PREZIME, stdin);
    novi.prezime[strcspn(novi.prezime, "\n")] = 0;

    printf("Plata: ");
    scanf("%f", &novi.plata); ocisti_bafer();

    printf("Premija: ");
    scanf("%f", &novi.premija); ocisti_bafer();

    BlokRadnik blok;
    int blok_index = 0;

    while (fread(&blok, sizeof(BlokRadnik), 1, f) == 1) {

        for (int i = 0; i < FAKTOR_BLOKIRANJA_RADNIK; i++) {

            // provera da li postoji radnik sa takvim mbr
            if (blok.slogovi[i].mbr == novi.mbr) {
                printf("Radnik vec postoji!\n");
                fclose(f);
                return;
            }

            //  upis eof
            if (blok.slogovi[i].mbr == -1) {

                blok.slogovi[i] = novi;

                // postavi novi EOF
                if (i < FAKTOR_BLOKIRANJA_RADNIK - 1) {
                    blok.slogovi[i + 1].mbr = -1;
                }

                // vrati se nazad i upisi blok
                fseek(f, -sizeof(BlokRadnik), SEEK_CUR);
                fwrite(&blok, sizeof(BlokRadnik), 1, f);

                // ako postojii poslednji u bloku, dodaje se novi blok
                if (i == FAKTOR_BLOKIRANJA_RADNIK - 1) {
                    BlokRadnik novi_blok = {0};
                    novi_blok.slogovi[0].mbr = -1;
                    fwrite(&novi_blok, sizeof(BlokRadnik), 1, f);
                }

                printf("Upisan radnik (blok %d, pozicija %d)\n", blok_index, i);
                fclose(f);
                return;
            }
        }

        blok_index++;
    }

    printf("Greska: nije pronadjen EOF!\n");
    fclose(f);
}
void ispis_radnika(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        printf("Greska!\n");
        return;
    }

    BlokRadnik blok;
    int blok_index = 0;

    while (fread(&blok, sizeof(BlokRadnik), 1, f) == 1) {
        for (int i = 0; i < FAKTOR_BLOKIRANJA_RADNIK; i++) {

            if (blok.slogovi[i].mbr == -1) {
                fclose(f);
                return;
            }

            printf("[%d,%d] MBR:%d %s %s Plata:%.2f Premija:%.2f\n",
                   blok_index, i,
                   blok.slogovi[i].mbr,
                   blok.slogovi[i].ime,
                   blok.slogovi[i].prezime,
                   blok.slogovi[i].plata,
                   blok.slogovi[i].premija);
        }
        blok_index++;
    }

    fclose(f);
}


void prikaz_radnika(const char* filename, int mbr) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        printf("Greska!\n");
        return;
    }

    BlokRadnik blok;
    int blok_index = 0;

    while (fread(&blok, sizeof(BlokRadnik), 1, f) == 1) {
        for (int i = 0; i < FAKTOR_BLOKIRANJA_RADNIK; i++) {

            if (blok.slogovi[i].mbr == -1) {
                printf("Nije pronadjen.\n");
                fclose(f);
                return;
            }

            if (blok.slogovi[i].mbr == mbr) {
                printf("Pronadjen radnik: [%d,%d]: %s %s Plata %.2f Premija %.2f\n",
                       blok_index, i,
                       blok.slogovi[i].ime,
                       blok.slogovi[i].prezime,
                       blok.slogovi[i].plata,
                       blok.slogovi[i].premija);
                fclose(f);
                return;
            }
        }
        blok_index++;
    }

    fclose(f);
}


void modifikacija_radnika(const char* filename, int mbr) {
    FILE* f = fopen(filename, "rb+");
    if (!f) {
        printf("Greska!\n");
        return;
    }

    BlokRadnik blok;

    while (fread(&blok, sizeof(BlokRadnik), 1, f) == 1) {
        for (int i = 0; i < FAKTOR_BLOKIRANJA_RADNIK; i++) {

            if (blok.slogovi[i].mbr == -1) {
                printf("Nije pronadjen.\n");
                fclose(f);
                return;
            }

            if (blok.slogovi[i].mbr == mbr) {

                printf("Novo ime: ");
                fgets(blok.slogovi[i].ime, MAX_IME, stdin);
                blok.slogovi[i].ime[strcspn(blok.slogovi[i].ime, "\n")] = 0;

                printf("Novo prezime: ");
                fgets(blok.slogovi[i].prezime, MAX_PREZIME, stdin);
                blok.slogovi[i].prezime[strcspn(blok.slogovi[i].prezime, "\n")] = 0;

                printf("Nova plata: ");
                scanf("%f", &blok.slogovi[i].plata);
                ocisti_bafer();

                printf("Nova premija: ");
                scanf("%f", &blok.slogovi[i].premija);
                ocisti_bafer();

                fseek(f, -sizeof(BlokRadnik), SEEK_CUR);
                fwrite(&blok, sizeof(BlokRadnik), 1, f);

                printf("Izmenjeno.\n");
                fclose(f);
                return;
            }
        }
    }

    fclose(f);
}



void uslov_bonus(const char* radnici_fajl, const char* isplate_fajl) {

    FILE* fr = fopen(radnici_fajl, "rb");
    FILE* fi = fopen(isplate_fajl, "rb");

    if (!fr || !fi) {
        printf("Greska pri otvaranju fajlova!\n");
        return;
    }

    BlokRadnik blok_r;
    BlokIsplata blok_i;

    int blok_index = 0;
    int pronadjeno = 0;

    while (fread(&blok_r, sizeof(BlokRadnik), 1, fr) == 1) {

        for (int i = 0; i < FAKTOR_BLOKIRANJA_RADNIK; i++) {

            if (blok_r.slogovi[i].mbr == -1) {
                if (!pronadjeno)
                    printf("Nema rezultata.\n");
                fclose(fr);
                fclose(fi);
                return;
            }

            // mora imati premiju
            if (blok_r.slogovi[i].premija > 0) {

                int nasao_bonus = 0;

                // RESET FAJLA ISPLATA
                rewind(fi);

                while (fread(&blok_i, sizeof(BlokIsplata), 1, fi) == 1) {

                    for (int j = 0; j < FAKTOR_BLOKIRANJA_ISPLATA; j++) {

                        if (blok_i.slogovi[j].identifikator == -1)
                            break;

                        if (
                            blok_i.slogovi[j].mbr == blok_r.slogovi[i].mbr &&
                            strcmp(blok_i.slogovi[j].razlog, "BONUS") == 0 &&
                            blok_i.slogovi[j].iznos > 2000
                        ) {
                            nasao_bonus = 1;
                        }
                    }
                }

                if (nasao_bonus) {
                    printf("-------------------------------------------------\n");
                    printf("[%d,%d] %s %s\n",
                           blok_index, i,
                           blok_r.slogovi[i].ime,
                           blok_r.slogovi[i].prezime);

                    printf("Plata: %.2f Premija: %.2f\n",
                           blok_r.slogovi[i].plata,
                           blok_r.slogovi[i].premija);

                    pronadjeno++;
                }
            }
        }

        blok_index++;
    }

    if (!pronadjeno)
        printf("Nema rezultata.\n");

    fclose(fr);
    fclose(fi);
}

void kreiraj_praznu_datoteku_radnik(const char* filename) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        printf("Greska!\n");
        return;
    }

    BlokRadnik blok = {0};
    blok.slogovi[0].mbr = -1;

    fwrite(&blok, sizeof(BlokRadnik), 1, f);
    fclose(f);

    printf("Kreiran fajl radnika.\n");
}

