/*
    ESERCIZIO 7
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define STATO_LIBERO    0
#define STATO_OCCUPATO  1

#define POSTI_BARBA     3
#define POSTI_DIVANO    4

struct divano_t {
    pthread_cond_t seduto;
    int posti_occupati;
};

struct barbiere_t {
    int posti_occupati;
};

struct cassiere_t {
    pthread_cond_t cassa;
    int stato;
};

struct negozio_t {
    pthread_mutex_t mutex;
    pthread_cond_t fuori;
    
    struct divano_t divano;
    struct barbiere_t barbiere;
    struct cassiere_t cassiere;
} negozio;

void init_negozio(struct negozio_t *n)
{
    pthread_mutexattr_t ma;
    pthread_condattr_t ca;

    pthread_mutexattr_init(&ma);
    pthread_condattr_init(&ca);

    pthread_mutex_init(&n->mutex, &ma);
    pthread_cond_init(&n->fuori, &ca);

    pthread_cond_init(&n->divano.seduto, &ca);
    pthread_cond_init(&n->cassiere.cassa, &ca);

    pthread_mutexattr_destroy(&ma);
    pthread_condattr_destroy(&ca);

    n->divano.posti_occupati = 0;
    n->barbiere.posti_occupati = 0;
    n->cassiere.stato = STATO_LIBERO;
}

void entra(struct negozio_t *n)
{
    pthread_mutex_lock(&n->mutex);

    while (POSTI_DIVANO >= n->divano.posti_occupati) {
        pthread_cond_wait(&n->fuori, &n->mutex);
    }

    pthread_mutex_unlock(&n->mutex);
}

void divano(struct negozio_t *n)
{
    pthread_mutex_lock(&n->mutex);

    while (POSTI_BARBA >= n->barbiere.posti_occupati) {
        n->divano.posti_occupati++;
        pthread_cond_wait(&n->divano.seduto, &n->mutex);
        n->divano.posti_occupati--;
    }

    n->barbiere.posti_occupati++;
    pthread_cond_signal(&n->fuori);

    pthread_mutex_unlock(&n->mutex);
}

void barba(struct negozio_t *n)
{
    sleep(rand() % 4);

    pthread_mutex_lock(&n->mutex);

    n->barbiere.posti_occupati--;

    pthread_mutex_unlock(&n->mutex);
}

void paga(struct negozio_t *n)
{
    pthread_mutex_lock(&n->mutex);

    while (STATO_OCCUPATO == n->cassiere.stato) {
        pthread_cond_wait(&n->cassiere.cassa, &n->mutex);
    }

    n->cassiere.stato = STATO_OCCUPATO;

    pthread_mutex_unlock(&n->mutex);
}

void esci(struct negozio_t *n)
{
    pthread_mutex_lock(&n->mutex);

    n->cassiere.stato = STATO_LIBERO;
    pthread_cond_signal(&n->cassiere.cassa);

    pthread_mutex_unlock(&n->mutex);
}

void *cliente()
{
    do {
        entra(&negozio);
        divano(&negozio);
        barba(&negozio);
        paga(&negozio);
        esci(&negozio);
    } while (1 == rand() % 2);

    return 0;
}

int main()
{
    pthread_attr_t attr;
    pthread_t thread;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    for (int i = 0; i < 20; i++) {
        pthread_create(&thread, &attr, cliente, NULL);
    }

    pthread_attr_destroy(&attr);
    sleep(40);

    return 0;
}