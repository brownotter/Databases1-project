#include "log.h"
#include <stdio.h>
#include <string.h>

static int ucitaj_poslednji_id()
{
    FILE *f = fopen(LOG_FILE, "rb");
    if (!f) return 0;

    LogSlog l;
    int max = 0;

    while (fread(&l, sizeof(LogSlog), 1, f)) {
        if (l.id > max)
            max = l.id;
    }

    fclose(f);
    return max;
}

void upisi_log(int mbr, const char* operacija, int pristupi)
{
    FILE *f = fopen(LOG_FILE, "ab");
    if (!f) return;

    static int global_id = 0;

    if (global_id == 0)
        global_id = ucitaj_poslednji_id() + 1;

    LogSlog l;

    l.id = global_id++;
    l.mbr = mbr;

    strncpy(l.operacija, operacija, sizeof(l.operacija));
    l.operacija[sizeof(l.operacija) - 1] = '\0';

    l.broj_pristupa = pristupi;

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

    int sum = 0;
    int cnt = 0;

    int last_id = -1;

    while (fread(&l, sizeof(LogSlog), 1, f))
    {
        if (l.id != last_id)
        {
            sum += l.broj_pristupa;
            cnt++;
            last_id = l.id;
        }
    }

    fclose(f);

    if (cnt == 0) {
        printf("Nema podataka.\n");
        return;
    }

    printf("Prosecni broj pristupa: %.2f\n", (float)sum / cnt);
}

void proveri_reorganizaciju()
{
    FILE *f = fopen(LOG_FILE, "rb");
    if (!f) return;

    LogSlog l;
    int suma = 0, count = 0;

    while (fread(&l, sizeof(LogSlog), 1, f)) {

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

    FILE *tmp = fopen("log_tmp.bin", "wb");
    if (!tmp) {
        fclose(old);
        return;
    }

    LogSlog l;

    while (fread(&l, sizeof(LogSlog), 1, old)) {
        fwrite(&l, sizeof(LogSlog), 1, tmp);
    }

    fclose(old);
    fclose(tmp);

    remove(LOG_FILE);
    rename("log_tmp.bin", LOG_FILE);

    printf("Log reorganizovan\n");
}


