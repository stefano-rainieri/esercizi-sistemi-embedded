/*
    ESERCIZIO 5

    In un programma multithread, ogni thread esegue il seguente codice:
    
    void *thread(void *arg)
    {
        int voto = rand()%2;
        vota(voto);
        if (voto == risultato()) 
            printf("Ho vinto!\n");
        else 
            printf("Ho perso!\n");
        
        pthread_exit(0);
    }
    
    cioe' ogni thread:
        -esprime un voto, che puo' essere 0 o 1, invocando la funzione vota(), la quale registra il voto in una struttura dati condivisa che per comodita' chiameremo "urna";
        -aspetta l'esito della votazione invocando la funzione risultato(), la quale controlla l'urna e ritorna 0 o 1a seconda che ci sia una maggioranza di voti 0 oppure di voti 1.
        -se l'esito della votazione e' uguale al proprio voto, stampa a video la stringa "Ho vinto", altrimenti stampa la stringa "Ho perso";
    
    Supponiamo che ci siano un numero dispari di threads nel sistema. 
    Il candidato deve implementare la struttura dati:
        struct {...} urna;
    e le funzioni:
        -void vota(int v);
        -int risultato(void);
    
    in modo che i thread si comportino come segue:
        Se l'esito della votazione non puo' ancora essere stabilito, la funzione risultato() deve bloccare il thread chiamante. 
        Non appena l'esito e' "sicuro" (ovvero almeno la meta' piu' uno dei threads ha votato 0, oppure almeno la meta' piu'
        uno dei threads ha votato 1) il thread viene sbloccato e la funzione risultato() ritorna l'esito della votazione. 
        I thread vengono sbloccati il piu' presto possibile, quindi anche prima che abbiano votato tutti. 

    Utilizzare i costrutti pthread_mutex_xxx e pthread_cond_xxx visti a lezione.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define N 21

struct urna_t {
    pthread_mutex_t mutex;
    pthread_cond_t priv;

    int num_0;
    int num_1;
} urna;

void init_urna(struct urna_t *u)
{
    pthread_mutexattr_t ma;
    pthread_condattr_t ca;

    pthread_mutexattr_init(&ma);
    pthread_condattr_init(&ca);

    pthread_mutex_init(&u->mutex, &ma);
    pthread_cond_init(&u->priv, &ca);

    pthread_mutexattr_destroy(&ma);
    pthread_condattr_destroy(&ca);

    u->num_0 = 0;
    u->num_1 = 0;

    srand((unsigned int) time(0));
}

void vota(struct urna_t *u, int voto)
{
    pthread_mutex_lock(&u->mutex);

    printf("voto: %d\n", voto);
    if (0 == voto) {
        u->num_0++;
    } else {
        u->num_1++;
    }

    pthread_cond_broadcast(&u->priv);

    pthread_mutex_unlock(&u->mutex);
}

int risultato(struct urna_t *u)
{
    pthread_mutex_lock(&u->mutex);

    while (u->num_0 <= N / 2 && u->num_1 <= N / 2) {
        pthread_cond_wait(&u->priv, &u->mutex);
    }

    pthread_mutex_unlock(&u->mutex);

    return u->num_0 > N / 2 ? 0 : 1;
}

void *thread(void *arg)
{
    int voto = rand() % 2;

    vota(&urna, voto);

    if (voto == risultato(&urna)) {
        printf("Ho vinto! %d 😇\n", voto);
    } else {
        printf("Ho perso! %d 👿\n", voto);
    }

    pthread_exit(0);
}

int main()
{
    pthread_attr_t a;
    pthread_t t;

    pthread_attr_init(&a);
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);
    
    for (int i = 0; i < N; i++) {
        pthread_create(&t, &a, thread, NULL);
    }

    pthread_attr_destroy(&a);

    sleep(2);

    return 0;
}