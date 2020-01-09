/**
    ESERCIZIO 2

    In un sistema organizzato secondo il modello a memoria comune viene definita una risorsa astratta R 
    sulla quale si può operare mediante 3 procedure identificate rispettivamente da ProcA, ProcB e Reset.

    - Le due procedure ProcA e ProcB, operano su variabili diverse della risorsa R e pertanto possono essere 
    eseguite concorrentemente tra loro senza generare interferenze.

    - La procedura Reset opera su tutte le variabilidi R e quindi deve essere eseguita in modo mutuamente 
    esclusivo sia con ProcA che con ProcB.

    1) Se i tre processi PA, PB e PR invocano rispettivamente le operazioni ProcA, ProcB e Reset, descrivere
    una tecnica che consenta ai processi PA e PB di eseguire le procedure da essi invocate senza vincoli 
    reciproci di mutua esclusione, garantendo invece la mutua esclusione con l'esecuzione della procedura 
    Reset invocata da PR.
    Nel risolvere il problema garantire la priorità alle esecuzioni di Reset rispetto a quelle di ProcA e ProcB.

    2) Qualora i processi che invocano le procedure ProcA e ProcB siano
    piu` di due (PA1,......PAn e PB1,....,PBm) riscrivere la soluzione
    garantendo anche la mutua esclusione tra due o piu` attivazioni di
    ProcA e tra due o piu` attivazioni di ProcB.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

// #define USA_SEM;
#define USA_COND;

#ifdef USA_SEM
sem_t mutex;
sem_t priv_ab;
sem_t priv_r;

int active_ab, active_r;
int blocked_ab, blocked_r;

void init() {
    srand((unsigned int)time(0));

    sem_init(&mutex, 0, 1);
    sem_init(&priv_ab, 0, 0);
    sem_init(&priv_r, 0, 0);

    active_ab = active_r = 0;
    blocked_ab = blocked_r = 0;
}

void start_proc_a(void) {
    sem_wait(&mutex);

    if (0 == blocked_r && 0 == active_r) {
        active_ab++;
        sem_post(&priv_ab);
    } else {
        blocked_ab++;
    }

    sem_post(&mutex);
    sem_wait(&priv_ab);
}

void end_proc_a(void) {
    sem_wait(&mutex);

    active_ab--;
    if (blocked_r > 0 && !active_ab) {
        active_r++;
        blocked_r--;
        sem_post(&priv_r);
    }

    sem_post(&mutex);
}

void start_proc_b(void) {
    start_proc_a();
}

void end_proc_b(void) {
    end_proc_a();
}

void start_reset(void) {
    sem_wait(&mutex);

    if (0 == active_ab) {
        active_r++;
        sem_post(&priv_r);
    } else {
        blocked_r++;
    }

    sem_post(&mutex);
    sem_wait(&priv_r);
}

void end_reset(void) {
    sem_wait(&mutex);

    active_r--;
    while (blocked_ab > 0) {
        active_ab++;
        blocked_ab--;
        sem_post(&priv_ab);
    }

    sem_post(&mutex);
}
#endif

#ifdef USA_COND
pthread_mutex_t mutex;
pthread_cond_t cond_ab, cond_reset;

int active_ab, active_reset;
int blocked_ab, blocked_reset;

void init() {
    pthread_mutexattr_t mutexattr;
    pthread_condattr_t condattr;

    pthread_mutexattr_init(&mutexattr);
    pthread_condattr_init(&condattr);

    pthread_mutex_init(&mutex, &mutexattr);
    pthread_cond_init(&cond_ab, &condattr);
    pthread_cond_init(&cond_reset, &condattr);

    pthread_mutexattr_destroy(&mutexattr);
    pthread_condattr_destroy(&condattr);

    active_ab = active_reset = 0;
    blocked_ab = blocked_reset = 0;
}

void start_proc_a() {
    pthread_mutex_lock(&mutex);

    while (active_reset || blocked_reset) {
        blocked_ab++;
        pthread_cond_wait(&cond_ab, &mutex);
        blocked_ab--;
    }

    active_ab++;

    pthread_mutex_unlock(&mutex);
}

void end_proc_a() {
    pthread_mutex_lock(&mutex);

    active_ab--;
    if (0 == active_ab && blocked_reset > 0) {
        pthread_cond_signal(&cond_reset);
    }

    pthread_mutex_unlock(&mutex);
}

void start_proc_b() {
    start_proc_a();
}

void end_proc_b() {
    end_proc_a();
}

void start_reset() {
    pthread_mutex_lock(&mutex);

    while (active_ab) {
        blocked_reset++;
        pthread_cond_wait(&cond_reset, &mutex);
        blocked_reset--;
    }

    active_reset++;

    pthread_mutex_unlock(&mutex);
}

void end_reset() {
    pthread_mutex_lock(&mutex);

    active_reset--;
    if (blocked_ab > 0) {
        pthread_cond_broadcast(&cond_ab);
    }

    pthread_mutex_unlock(&mutex);
}
#endif

void delay(void) {
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 10 + 1) * 1000000;
    nanosleep(&t, NULL);
}

void print(char *c) {
    int i, j;

    printf("[");
    for (i = 0; i < 50; i++) {
        printf(c);
        for (j = 0; j < 1000000; j++);
    }
    printf("]");
}

void proc_a(void) {
    print("+");
}

void proc_b(void) {
    print("-");
}

void reset(void) {
    print(".");
}

void *PA() {
    while (1) {
        printf("A");
        start_proc_a();
        proc_a();
        end_proc_a();
        printf("a");
    }

    return 0;
}

void *PB() {
    while (1) {
        printf("B");
        start_proc_b();
        proc_b();
        end_proc_b();
        printf("b");
    }

    return 0;
}

void *PR() {
    while (1) {
        printf("R");
        start_reset();
        reset();
        end_reset();
        printf("r");
        delay();
    }

    return 0;
}

int main() {
    pthread_attr_t attr;
    pthread_t thread;

    init();

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_create(&thread, &attr, PA, NULL);
    pthread_create(&thread, &attr, PB, NULL);
    pthread_create(&thread, &attr, PR, NULL);

    pthread_attr_destroy(&attr);

    sleep(10);

    return 0;
}
