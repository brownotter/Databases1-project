#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED

#define F_LOG 6
#define MAX_PROSEK 3
#define LOG_FILE "log.bin"

typedef struct {
    int id;
    int mbr;
    char operacija[16];
    int broj_pristupa;
} LogSlog;

void prikaz_loga();

static int ucitaj_poslednji_id();

void upisi_log(int mbr, const char* operacija, int pristupi);
void prosecni_pristupi();

void proveri_reorganizaciju();
void reorganizuj_datoteku_log();

#endif // LOG_H_INCLUDED
