#include "log.h"
#include <stdio.h>
#include <string.h>

static int global_id = 1;
static int brojac_pristupa = 0;

void upisi_log(int mbr, const char* operacija)
{
    FILE *f = fopen(LOG_FILE, "ab");
    if (!f) return;

    LogSlog l;

    l.id = global_id++;
    l.mbr = mbr;
    strcpy(l.operacija, operacija);
    l.broj_pristupa = 1;   // svaki poziv = 1 pristup

    fwrite(&l, sizeof(LogSlog), 1, f);
    fclose(f);

    proveri_reorganizaciju();
}

void prikaz_loga()
{
    FILE *f = fopen(LOG_FILE, "rb");
    if (!f) {
        printf("Nema log fajla.\n");
        return;
    }

    LogSlog l;

    while (fread(&l, sizeof(LogSlog), 1, f)) {
        printf("ID:%d MBR:%d OP:%s PRISTUPI:%d\n",
               l.id, l.mbr, l.operacija, l.broj_pristupa);
    }

    fclose(f);
}

void prosecni_pristupi()
{
    FILE *f = fopen(LOG_FILE, "rb");
    if (!f) return;

    LogSlog l;
    int sum = 0, cnt = 0;

    while (fread(&l, sizeof(LogSlog), 1, f)) {
        sum += l.broj_pristupa;
        cnt++;
    }

    if (cnt == 0) return;

    printf("Prosecni broj pristupa: %.2f\n", (float)sum / cnt);

    fclose(f);
}

void proveri_reorganizaciju()
{
    FILE *f = fopen(LOG_FILE, "rb");
    if (!f) return;

    LogSlog l;
    int suma = 0, count = 0;

    while (fread(&l, sizeof(LogSlog), 1, f)) {

        /* filtriraj samo RADNIK operacije */
        if (strstr(l.operacija, "RADNIK") != NULL) {
            suma += l.broj_pristupa;
            count++;
        }
    }

    fclose(f);

    if (count == 0) return;

    float prosek = (float)suma / count;

    printf("Prosek pristupa: %.2f\n", prosek);

    if (prosek > MAX_PROSEK) {
        printf("POKRECE SE REORGANIZACIJA\n");
        reorganizuj_datoteku_log();
    }
}

void reorganizuj_datoteku_log()
{
    printf("REORGANIZACIJA POKRENUTA\n");

    FILE *old = fopen(LOG_FILE, "rb");
    if (!old) return;

    FILE *newf = fopen("log_tmp.bin", "wb");
    if (!newf) {
        fclose(old);
        return;
    }

    LogSlog l;

    while (fread(&l, sizeof(LogSlog), 1, old)) {
        // ovde možeš filter (npr. samo RADNIK)
        fwrite(&l, sizeof(LogSlog), 1, newf);
    }

    fclose(old);
    fclose(newf);

    remove(LOG_FILE);
    rename("log_tmp.bin", LOG_FILE);

    printf("Log reorganizovan bez gubitka podataka\n");
}

