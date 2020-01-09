/* 
    Siano A, B, C, D ed E le procedure che un insieme di processi P1, P2,
    ..., PN possono invocare e che devono essere eseguite rispettando i
    seguenti vincoli di sincronizzazione:

    Sono possibili solo due sequenze di esecuzioni delle procedure,
    sequenze tra loro mutuamente esclusive:

    - la prima sequenza prevede che venga eseguita per prima la procedura
            A. a cui puo' seguire esclusivamente l'esecuzione di una o
            piu' attivazioni concorrenti della procedura B;

    - la seconda sequenza e' costituita dall'esecuzione della procedura C
            a cui puo' seguire esclusivamente l'esecuzione della
            procedura D, o in alternativa a D della procedura E.

    Una volta terminata una delle due sequenze una nuova sequenza puo'
    essere di nuovo iniziata.

    Utilizzando il meccanismo dei semafori, realizzare le funzioni StartA,
    EndA, StartB, EndB, .... , StartE, EndE che, invocate dai processi
    clienti P1, P2, ..., PN rispettivamente prima e dopo le corrispondenti
    procedure, garantiscano il rispetto dei precedenti vincoli. Nel
    risolvere il problema non e' richiesta la soluzione ad eventuali
    problemi di starvation.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define STATE_0   0
#define STATE_A   1
#define STATE_B   2
#define STATE_C   3
#define STATE_D_E 4
#define STATE_D   5
#define STATE_E   6

#define USA_COND
// #define USA_SEM

#ifdef USA_SEM
struct manager_t
{
    sem_t mutex;
    sem_t semaphore_a, semaphore_b, semaphore_c, semaphore_d, semaphore_e;
    
    int blocked_a, blocked_b, blocked_c, blocked_d, blocked_e, active_b;
    int state;
} manager;

void initialize_manager(struct manager_t *m)
{
    sem_init(&m->mutex, 0, 1);

    sem_init(&m->semaphore_a, 0, 0);
    sem_init(&m->semaphore_b, 0, 0);
    sem_init(&m->semaphore_c, 0, 0);
    sem_init(&m->semaphore_d, 0, 0);
    sem_init(&m->semaphore_e, 0, 0);
    
    m->blocked_a = 0;
    m->blocked_b = 0;
    m->blocked_c = 0;
    m->blocked_d = 0;
    m->blocked_e = 0;
    m->active_b = 0;

    m->state = STATE_0;
}

void start_A(struct manager_t *m)
{
    sem_wait(&m->mutex);

    if (STATE_0 == m->state) {
        m->state = STATE_A;
        sem_post(&m->semaphore_a);
    } else {
        m->blocked_a++;
    }

    sem_post(&m->mutex);
    sem_wait(&m->semaphore_a);
}

void end_A(struct manager_t *m)
{
    sem_wait(&m->mutex);

    m->state = STATE_B;
    while (m->blocked_b > 0)
    {
        m->blocked_b--;
        m->active_b++;
        sem_post(&m->semaphore_b);
    }
    
    sem_post(&m->mutex);
}

void start_B(struct manager_t *m)
{
    sem_wait(&m->mutex);

    if (STATE_B == m->state) {
        m->active_b++;
        sem_post(&m->semaphore_b);
    } else {
        m->blocked_b++;
    }

    sem_post(&m->mutex);
    sem_wait(&m->semaphore_b);
}

void end_B(struct manager_t *m)
{
    sem_wait(&m->mutex);

    m->active_b--;

    if (0 == m->active_b) {
        if (0 == m->blocked_a && 0 == m->blocked_c) {
            m->state = STATE_0;
        } else if (0 < m->blocked_a && 0 == m->blocked_c) {
            m->blocked_a--;
            m->state = STATE_A;
            sem_post(&m->semaphore_a);
        } else if (0 == m->blocked_a && 0 < m->blocked_c) {
            m->blocked_c--;
            m->state = STATE_C;
            sem_post(&m->semaphore_c);
        } else {
            if (0 == rand() % 2) {
                m->blocked_a--;
                m->state = STATE_A;
                sem_post(&m->semaphore_a);
            } else {
                m->blocked_c--;
                m->state = STATE_C;
                sem_post(&m->semaphore_c);
            }
        }
    }

    sem_post(&m->mutex);
}

void start_C(struct manager_t *m)
{
    sem_wait(&m->mutex);

    if (STATE_0 == m->state) {
        m->state = STATE_C;
        sem_post(&m->semaphore_c);
    } else {
        m->blocked_c++;
    }

    sem_post(&m->mutex);
    sem_wait(&m->semaphore_c);
}

void end_C(struct manager_t *m) 
{
    sem_wait(&m->mutex);
    
    if ((0 == m->blocked_d && 0 == m->blocked_e) || (0 < m->blocked_d && 0 < m->blocked_e)) {
        if (0 == rand() % 2) {
            m->state = STATE_D;
            sem_post(&m->semaphore_d);
        } else {
            m->state = STATE_E;
            sem_post(&m->semaphore_e);
        }
    } else if (0 < m->blocked_d) {
        m->state = STATE_D;
        m->blocked_d--;
        sem_post(&m->semaphore_d);
    } else {
        m->state = STATE_E;
        m->blocked_e--;
        sem_post(&m->semaphore_e);
    }

    sem_post(&m->mutex);
}

void start_D(struct manager_t *m)
{
    sem_wait(&m->mutex);

    if (STATE_D == m->state) {
        sem_post(&m->semaphore_d);
    } else {
        m->blocked_d++;
    }

    sem_post(&m->mutex);
    sem_wait(&m->semaphore_d);
}

void end_D(struct manager_t *m) {
    sem_wait(&m->mutex);

    if (0 == m->blocked_a && 0 == m->blocked_c) {
        m->state = STATE_0;
    } else if (0 < m->blocked_a && 0 == m->blocked_c) {
        m->state = STATE_A;
        m->blocked_a--;
        sem_post(&m->semaphore_a);
    } else if (0 == m->blocked_a && 0 < m->blocked_c) {
        m->state = STATE_C;
        m->blocked_c--;
        sem_post(&m->semaphore_c);
    } else {
        if (0 == rand() % 2) {
            m->state = STATE_A;
            m->blocked_a--;
            sem_post(&m->semaphore_a);
        } else {
            m->state = STATE_C;
            m->blocked_c--;
            sem_post(&m->semaphore_c);
        }
    }

    sem_post(&m->mutex);
}

void start_E(struct manager_t *m)
{
    sem_wait(&m->mutex);

    if (STATE_E == m->state) {
        sem_post(&m->semaphore_e);
    } else {
        m->blocked_e++;
    }

    sem_post(&m->mutex);
    sem_wait(&m->semaphore_e);
}

void end_E(struct manager_t *m)
{
    end_D(m);
}
#endif

#ifdef USA_COND
struct manager_t {
    pthread_mutex_t mutex;
    pthread_cond_t cond_a, cond_b, cond_c, cond_d, cond_e;

    int blocked_a, blocked_b, blocked_c, blocked_d, blocked_e;
    int active_b;

    int state;
} manager;

void initialize_manager(struct manager_t *m)
{
    pthread_mutexattr_t mattr;
    pthread_condattr_t cattr;

    pthread_mutexattr_init(&mattr);
    pthread_condattr_init(&cattr);

    pthread_mutex_init(&m->mutex, &mattr);
    pthread_cond_init(&m->cond_a, &cattr);
    pthread_cond_init(&m->cond_b, &cattr);
    pthread_cond_init(&m->cond_c, &cattr);
    pthread_cond_init(&m->cond_d, &cattr);
    pthread_cond_init(&m->cond_e, &cattr);

    pthread_mutexattr_destroy(&mattr);
    pthread_condattr_destroy(&cattr);

    m->blocked_a = m->blocked_b = m->blocked_c = m->blocked_d = m->blocked_e = 0;
    m->active_b = 0;

    m->state = STATE_0;
    srand(555);
}

void start_A(struct manager_t *m) 
{
    pthread_mutex_lock(&m->mutex);

    while (STATE_0 != m->state) {
        m->blocked_a++;
        pthread_cond_wait(&m->cond_a, &m->mutex);
        m->blocked_a--;
    }

    m->state = STATE_A;

    pthread_mutex_unlock(&m->mutex);
}

void end_A(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    m->state = STATE_B;
    if (m->blocked_b > 0) {
        pthread_cond_broadcast(&m->cond_b);
    }

    pthread_mutex_unlock(&m->mutex);
}

void start_B(struct manager_t *m) 
{
    pthread_mutex_lock(&m->mutex);

    while (STATE_B != m->state) {
        m->blocked_b++;
        pthread_cond_wait(&m->cond_b, &m->mutex);
        m->blocked_b--;
    }

    m->active_b++;

    pthread_mutex_unlock(&m->mutex);
}

void end_B(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    m->active_b--;
    if (0 == m->active_b) {

        m->state = STATE_0;
        if (m->blocked_a > 0) {
            pthread_cond_signal(&m->cond_a);
        } else if (m->blocked_c > 0) {
            pthread_cond_signal(&m->cond_c);
        }
    }

    pthread_mutex_unlock(&m->mutex);
}

void start_C(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    while (STATE_0 != m->state) {
        m->blocked_c++;
        pthread_cond_wait(&m->cond_c, &m->mutex);
        m->blocked_c--;
    }

    m->state = STATE_C;

    pthread_mutex_unlock(&m->mutex);
}

void end_C(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    m->state = STATE_D_E;
    if (m->blocked_d > 0) {
        pthread_cond_signal(&m->cond_d);
    } else if (m->blocked_e > 0) {
        pthread_cond_signal(&m->cond_e);
    }

    pthread_mutex_unlock(&m->mutex);
}

void start_D(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    while (STATE_D_E != m->state) {
        m->blocked_d++;
        pthread_cond_wait(&m->cond_d, &m->mutex);
        m->blocked_e--;
    }

    m->state = STATE_D;

    pthread_mutex_unlock(&m->mutex);
}

void end_D(struct manager_t *m) 
{
    pthread_mutex_lock(&m->mutex);

    m->state = STATE_0;
    if (m->blocked_a > 0) {
        pthread_cond_signal(&m->cond_a);
    } else if (m->blocked_c > 0) {
        pthread_cond_signal(&m->cond_c);
    }

    pthread_mutex_unlock(&m->mutex);
}

void start_E(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    while (STATE_D_E != m->state) {
        m->blocked_d++;
        pthread_cond_wait(&m->cond_d, &m->mutex);
        m->blocked_e--;
    }

    m->state = STATE_E;

    pthread_mutex_unlock(&m->mutex);
}

void end_E(struct manager_t *m)
{
    end_D(m);
}

#endif

void pausetta(void)
{
  struct timespec t;
  t.tv_sec = 0;
  t.tv_nsec = (rand() % 10 + 1) * 1000000;
  nanosleep(&t,NULL);
}

void *procedure_A() 
{
    while (1) {
        start_A(&manager);
        printf("  A");
        end_A(&manager);
        pausetta();
    }

    return 0;
}

void *procedure_B()
{
    while (1) {
        start_B(&manager);
        printf("B");
        end_B(&manager);
        pausetta();
    }

    return 0;
}

void *procedure_C()
{
    while (1) {
        start_C(&manager);
        printf("  C");
        end_C(&manager);
        pausetta();
    }

    return 0;
}

void *procedure_D()
{
    while (1) {
        start_D(&manager);
        printf("D");
        end_D(&manager);
        pausetta();
    }

    return 0;
}

void *procedure_E()
{
    while (1) {
        start_E(&manager);
        printf("E");
        end_E(&manager);
        pausetta();
    }

    return 0;
}

int main()
{
    pthread_attr_t attr;
    pthread_t thread;

    initialize_manager(&manager);

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_create(&thread, &attr, procedure_A, NULL);
    pthread_create(&thread, &attr, procedure_A, NULL);

    pthread_create(&thread, &attr, procedure_B, NULL);
    pthread_create(&thread, &attr, procedure_B, NULL);
    pthread_create(&thread, &attr, procedure_B, NULL);
    
    pthread_create(&thread, &attr, procedure_C, NULL);
    pthread_create(&thread, &attr, procedure_C, NULL);
    
    pthread_create(&thread, &attr, procedure_D, NULL);
    pthread_create(&thread, &attr, procedure_D, NULL);
    
    pthread_create(&thread, &attr, procedure_E, NULL);
    pthread_create(&thread, &attr, procedure_E, NULL);

    pthread_attr_destroy(&attr);
    sleep(10);

    return 0;
}
