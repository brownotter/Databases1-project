#include <stdio.h>
#include <string.h>

#include "radnik.h"
#include "isplata.h"
#include "utils.h"


int main() {

    //printf("Hello world!\n");
    //return 0;

    int glavni_izbor = 0;
    char radnik_fajl[64] = "radnik.bin";
    char isplata_fajl[64] = "isplata.bin";

    do {
        printf("\n=== GLAVNI MENI ===\n");
        printf("1. Rad sa radnicima\n");
        printf("2. Rad sa isplatama\n");
        printf("0. Izlaz\n");
        printf("Izbor: ");
        scanf("%d", &glavni_izbor);
        ocisti_bafer();

        if (glavni_izbor == 1) {
            int izbor = 0;
            do {
                printf("\n--- MENI RADNIK ---\n");
                printf("Aktivna datoteka: %s\n", radnik_fajl);
                printf("1. Izaberi naziv fajla\n");
                printf("2. Kreiraj praznu datoteku\n");
                printf("3. Unos radnika\n");
                printf("4. Prikaz radnika po MBR\n");
                printf("5. Radnici sa bonusima > 2000\n");
                printf("6. Izmena radnika\n");
                printf("7. Ispis svih radnika (debug)\n");
                printf("0. Nazad\n");
                printf("Izbor: ");
                scanf("%d", &izbor);
                ocisti_bafer();

                switch (izbor) {
                    case 1:
                        printf("Unesite naziv fajla: ");
                        fgets(radnik_fajl, sizeof(radnik_fajl), stdin);
                        radnik_fajl[strcspn(radnik_fajl, "\n")] = 0;
                        break;
                    case 2:
                        kreiraj_praznu_serijsku_radnik_datoteku(radnik_fajl);
                        break;
                    case 3:
                        unos_radnika(radnik_fajl);
                        break;
                    case 4: {
                        int mbr;
                        printf("Unesite MBR: ");
                        scanf("%d", &mbr);
                        ocisti_bafer();
                        prikaz_radnika_po_mbr(radnik_fajl, mbr);
                        break;
                    }
                    case 5:
                        radnici_sa_bonusima(radnik_fajl, isplata_fajl);
                        break;
                    case 6: {
                        int mbr;
                        printf("Unesite MBR: ");
                        scanf("%d", &mbr);
                        ocisti_bafer();
                        izmeni_radnika(radnik_fajl, mbr);
                        break;
                    }
                    case 7:
                        ispis_radnika(radnik_fajl);
                        break;
                }
            } while (izbor != 0);
        }

        else if (glavni_izbor == 2) {
            int izbor = 0;
            do {
                printf("\n--- MENI ISPLATA ---\n");
                printf("Aktivna datoteka: %s\n", isplata_fajl);
                printf("1. Izaberi naziv fajla\n");
                printf("2. Kreiraj praznu datoteku\n");
                printf("3. Unos isplate\n");
                printf("4. Ispis isplata (debug)\n");
                printf("0. Nazad\n");
                printf("Izbor: ");
                scanf("%d", &izbor);
                ocisti_bafer();

                switch (izbor) {
                    case 1:
                        printf("Unesite naziv fajla: ");
                        fgets(isplata_fajl, sizeof(isplata_fajl), stdin);
                        isplata_fajl[strcspn(isplata_fajl, "\n")] = 0;
                        break;
                    case 2:
                        kreiraj_praznu_serijsku_isplata_datoteku(isplata_fajl);
                        break;
                    case 3:
                        unos_isplate(isplata_fajl, radnik_fajl);
                        break;
                    case 4:
                        ispis_isplata(isplata_fajl);
                        break;
                }
            } while (izbor != 0);
        }

    } while (glavni_izbor != 0);

    printf("Izlaz iz programa.\n");
    return 0;
}

