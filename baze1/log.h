#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED

#define F_LOG 6

typedef struct {
    int id;
    int mbr;
    char operacija[16];
    int broj_pristupa;
} LogSlog;

void prikaz_loga();
void upisi_log(int mbr, const char* operacija, int pristupi);
void prosecni_pristupi();

#endif // LOG_H_INCLUDED
