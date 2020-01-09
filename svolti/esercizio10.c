/*
    ESERCIZIO 10
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define STATO_LIBERO    0
#define STATO_OCCUPATO  1

struct gestore_t {
    sem_t mutex;
    sem_t priv_q, priv_a, priv_b, priv_2;

    int stato_a, stato_b;
    int bloccati_q, bloccati_a, bloccati_b, bloccati_2;
} gestore;

void init_gestore(struct gestore_t *g)
{
    sem_init(&g->mutex, 0, 1);
    sem_init(&g->priv_q, 0, 0);
    sem_init(&g->priv_a, 0, 0);
    sem_init(&g->priv_b, 0, 0);
    sem_init(&g->priv_2, 0, 0);

    g->stato_a = g->stato_b = STATO_LIBERO;
    g->bloccati_q = g->bloccati_a = g->bloccati_b = g->bloccati_2 = 0;
}

int sveglia(struct gestore_t *g)
{
    if ((STATO_LIBERO == g->stato_a || STATO_LIBERO == g->stato_b) && g->bloccati_q > 0) {
        sem_post(&g->priv_q);

        return 1;
    } else if (STATO_LIBERO == g->stato_a && g->bloccati_a > 0) {
        sem_post(&g->priv_a);

        return 1;
    } else if (STATO_LIBERO == g->stato_b && g->bloccati_b > 0) {
        sem_post(&g->priv_b);

        return 1;
    } else if (STATO_LIBERO == g->stato_a && STATO_LIBERO == g->stato_b && g->bloccati_2 > 0) {
        sem_post(&g->bloccati_2);

        return 1;
    }

    return 0;
}

char start_q(struct gestore_t *g)
{
    sem_wait(&g->mutex);

    if (STATO_OCCUPATO == g->stato_a && STATO_OCCUPATO == g->stato_b) {
        g->bloccati_q++;
        sem_post(&g->mutex);
        sem_wait(&g->priv_q);
        g->bloccati_q--;
    }

    char risorsa;

    if (STATO_LIBERO == g->stato_a) {
        g->stato_a = STATO_OCCUPATO;
        risorsa = 'A';  
    } else {
        g->stato_b = STATO_OCCUPATO;
        risorsa = 'B';
    }

    sem_post(&g->mutex);

    return risorsa;
}

void end_q(struct gestore_t *g, char risorsa)
{
    sem_wait(&g->mutex);

    if ('A' == risorsa) {
        g->stato_a = STATO_LIBERO;
    } else {
        g->stato_b = STATO_LIBERO;
    }

    if (!sveglia(&g)) {
        sem_post(&g->mutex);
    }
}

void start_a(struct gestore_t *g)
{
    sem_wait(&g->mutex);

    if (STATO_OCCUPATO == g->stato_a) {
        g->bloccati_a++;
        sem_post(&g->mutex);
        sem_wait(&g->priv_a);
        g->bloccati_a--;
    }

    g->stato_a = STATO_OCCUPATO;

    sem_wait(&g->mutex);
}

void end_a(struct gestore_t *g)
{
    sem_wait(&g->mutex);

    g->stato_a = STATO_LIBERO;

    if (!sveglia(&g)) {
        sem_post(&g->mutex);
    }
}

void start_b(struct gestore_t *g)
{
    sem_wait(&g->mutex);

    if (STATO_OCCUPATO == g->stato_b) {
        g->bloccati_b++;
        sem_post(&g->mutex);
        sem_wait(&g->priv_b);
        g->bloccati_b--;
    }

    g->stato_b = STATO_OCCUPATO;

    sem_post(&g->mutex);
}

void end_b(struct gestore_t *g)
{
    sem_wait(&g->mutex);

    g->stato_b = STATO_LIBERO;

    if (!sveglia(&g)) {
        sem_post(&g->mutex);
    }
}

void start_2(struct gestore_t *g)
{
    sem_wait(&g->mutex);

    if (STATO_OCCUPATO == g->stato_a || STATO_OCCUPATO == g->stato_b) {
        g->bloccati_2++;
        sem_post(&g->mutex);
        sem_wait(&g->priv_2);
        g->bloccati_2--;
    }

    g->stato_a = STATO_OCCUPATO;
    g->stato_b = STATO_OCCUPATO;

    sem_post(&g->mutex);
}

void end_2(struct gestore_t *g)
{
    sem_wait(&g->mutex);

    g->stato_a = STATO_LIBERO;
    g->stato_b = STATO_LIBERO;

    if (!sveglia(&g)) {
        sem_post(&g->mutex);
    }
}

void ric_q(struct gestore_t *g)
{
    char risorsa = start_q(&g);
    printf("Q -> %c", risorsa);
    end_q(&g, risorsa);
}

void ric_a(struct gestore_t *g)
{
    start_a(&g);
    printf("A");
    end_a(&g);
}

void ric_b(struct gestore_t *g)
{
    start_b(&g);
    printf("B");
    end_b(&g);
}

void ric_2(struct gestore_t *g)
{
    start_2(&g);
    printf("2 -> AB");
    end_2(&g);
}

void delay()
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 10 + 1) * 1000000;
    nanosleep(&t, NULL);
}

void *P()
{
    int resource;

    while (1) {
        resource = rand() % 4;

        switch (resource) {
            case 0:
                ric_q(&gestore);
                break;
            
            case 1:
                ric_a(&gestore);
                break;

            case 2:
                ric_b(&gestore);
                break;

            case 3:
                ric_2(&gestore);
                break;
        }

        delay();    
    }

    return 0;
}

int main() 
{
    init_gestore(&gestore);

    pthread_attr_t attr;
    pthread_t thread;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_create(&thread, &attr, P, NULL);
    pthread_create(&thread, &attr, P, NULL);
    pthread_create(&thread, &attr, P, NULL);
    pthread_create(&thread, &attr, P, NULL);

    pthread_attr_destroy(&attr);
    sleep(10);
    
    return 0;
}
