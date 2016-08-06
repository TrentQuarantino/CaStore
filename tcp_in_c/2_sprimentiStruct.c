/*https://users.dimi.uniud.it/~ivan.scagnetto/LABSO3/lezione13.pdf
http://www.dis.uniroma1.it/~liberato/struct/liste/tiposequenza.shtml
http://www.dis.uniroma1.it/~liberato/struct/typestruct/strutture.shtml
http://ennebi.solira.org/c-lang/pag45.html*/
#include <stdio.h>
#include <stdlib.h>

struct autore {
    char nome[20];
    char cognome[20];
};


struct libro {
    char titolo[50];
    //char autore[20];
    struct autore autore;
    char editore[20];
    float prezzo;
};
void libro_inserisci (struct libro *self) {
    printf("titolo\n");
    fgets(self->titolo, 50, stdin);
    printf("autore nome\n");
    fgets(self->autore.nome, 20, stdin);
    printf("autore cognome\n");
    fgets(self->autore.cognome, 20, stdin);
    printf("editore\n");
    fgets(self->editore, 20, stdin);
    printf("prezzo\n");
    char buffer[10];
    fgets(buffer, 10, stdin);
    self->prezzo = strtof(buffer, NULL);
}

void libro_stampa (struct libro *self) {
    printf("\n%s %s %s %s %.2f\n", self->titolo, self->autore.nome, self->autore.cognome, self->editore, self->prezzo);
}

#define N_LIBRI 3
int main (int argc, char **argv) {
    struct libro libri[N_LIBRI];
    int i;
    for (i = 0; i < N_LIBRI; i++) {
        libro_inserisci(libri + i); //avanzo il pointer oppure &libri[i]

    }
    for (i = 0; i < N_LIBRI; i++) {
        libro_stampa(libri + i); //avanzo il pointer oppure &libri[i]

    }

    //lib1 = (struct libro *) malloc(sizeof(struct libro));

    //libro_inserisci(lib1);

    //libro_stampa(lib1);

    //free(lib1);

    return (0);
}

