#include "log.h"
#include <stdio.h>
#include <string.h>

static int global_id = 1;

void upisi_log(int mbr, const char* operacija, int pristupi)
{
    FILE *f = fopen("log.bin", "ab");
    if (!f) return;

    LogSlog l;

    l.id = global_id++;
    l.mbr = mbr;
    strcpy(l.operacija, operacija);
    l.broj_pristupa = pristupi;

    fwrite(&l, sizeof(LogSlog), 1, f);
    fclose(f);
}

void prikaz_loga()
{
    FILE *f = fopen("log.bin", "rb");
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
    FILE *f = fopen("log.bin", "rb");
    if (!f) return;

    LogSlog l;
    int sum = 0, cnt = 0;

    while (fread(&l, sizeof(LogSlog), 1, f)) {
        sum += l.broj_pristupa;
        cnt++;
    }

    if (cnt == 0) return;

    float avg = (float)sum / cnt;

    printf("Prosecni broj pristupa: %.2f\n", avg);

    if (avg > 3) {
        printf(">>> POTREBNA REORGANIZACIJA <<<\n");
    }

    fclose(f);
}
