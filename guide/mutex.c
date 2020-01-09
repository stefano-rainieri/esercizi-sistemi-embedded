#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

/**
 * STRUTTURA DATI
 * 
 * Condivisa e utilizzata per gestione concorrenza e sincronizzazione
 **/
struct my_resource {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} resource;

/**
 * INIZIALIZZAZIONE
 **/
void inizializza(struct my_resource *r) {
    pthread_mutexattr_t ma;
    pthread_condattr_t ca;

    pthread_mutexattr_init(&ma);
    pthread_condattr_init(&ca);

    pthread_mutex_init(&r->mutex, &ma);
    pthread_cond_init(&r->cond, &ca);

    pthread_mutexattr_destroy(&ma);
    pthread_condattr_destroy(&ca);
}

/**
 * PROLOGO
 **/
void prologo(struct my_resource *r) {
    pthread_mutex_lock(&r->mutex);

    while ("<NON POSSO ACCEDERE ALLA RISORSA>") {
        "<REGISTRO BLOCCAGGIO>";
        pthread_cond_wait(&r->cond, &r->mutex);
        "<DEREGISTRO BLOCCAGGIO>";
    }

    "<ALLOCO RISORSA>";

    pthread_mutex_unlock(&r->mutex);
}

/**
 * EPILOGO
 **/
void epilogo(struct my_resource *r) {
    pthread_mutex_lock(&r->mutex);

    "<DEALLOCO RISORSA>";
    
    if ("<POSSO SVEGLIARNE SOLO UNO>") {
        pthread_cond_signal(&r->cond);
    }

    if ("<POSSO SVEGLIARNE TANTI>") {
        pthread_cond_broadcast(&r->cond);
    }

    pthread_mutex_unlock(&r->mutex);
}

/**
 * THREAD
 * 
 * Utilizzo della risorsa protetto da prologo ed epilogo.
 **/
void *thread() {
    prologo(&resource);
    "UTILIZZO SAFE DELLA RISORSA";
    epilogo(&resource);

    return 0;
}

/**
 * MAIN
 * 
 * Thread principale che crea due thread concorrenti.
 **/
int main() {
    inizializza(&resource);

    pthread_attr_t a;
    pthread_t t;

    pthread_attr_init(&a);
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    pthread_create(&t, &a, thread, NULL);
    pthread_create(&t, &a, thread, NULL);

    pthread_attr_destroy(&a);
    sleep(10);

    return 0;
}