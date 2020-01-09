/*
    ESERCIZIO 9
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define LETTURA   0
#define SCRITTURA 1   
#define N         5

typedef int T;

struct buffer_t {
    sem_t mutex;
    sem_t priv_send[N];
    sem_t priv_receive;

    T messaggio[N];
    int stato;
    int num_scritti;
} buffer;

void init_buffer(struct buffer_t *b)
{
    sem_init(&b->mutex, 0, 1);
    for (int i = 0; i < N; i++) {
        sem_init(&b->priv_send[i], 0, 0);
        b->messaggio[i] = -1;
    }
    sem_init(&b->priv_receive, 0, 0);

    b->num_scritti = 0;
    b->stato = SCRITTURA;

    srand((unsigned int)time(0));
}

void start_send(struct buffer_t *b, int num)
{
    sem_wait(&b->mutex);

    if (b->num_scritti < N && -1 == b->messaggio[num] && SCRITTURA == b->stato) {
        sem_post(&b->priv_send[num]);
    }

    sem_post(&b->mutex);
    sem_wait(&b->priv_send[num]);
}

void do_send(struct buffer_t *b, int num)
{
    b->messaggio[num] = rand() % 9;
    printf("DATO SCRITTO : [%d] -> %d\n", num, b->messaggio[num]);
}

void end_send(struct buffer_t *b)
{
    sem_wait(&b->mutex);

    b->num_scritti++;
    if (N == b->num_scritti) {
        b->stato = LETTURA;
        sem_post(&b->priv_receive);
    }

    sem_post(&b->mutex);
}

void start_receive(struct buffer_t *b)
{
    sem_wait(&b->mutex);

    if (LETTURA == b->stato && N == b->num_scritti) {
        sem_post(&b->priv_receive);
    }

    sem_post(&b->mutex);
    sem_wait(&b->priv_receive);
}

void do_receive(struct buffer_t *b)
{
    sleep(2);

    printf("\nMESSAGGIO LETTO -> ");
    for (int i = 0; i < N; i++) {
        printf("%d", b->messaggio[i]);
        b->messaggio[i] = -1;
    }
    printf("\n------------------------------");
    printf("\n\n");

    sleep(2);
}

void end_receive(struct buffer_t *b)
{
    sem_wait(&b->mutex);

    b->stato = SCRITTURA;
    b->num_scritti = 0;
    for (int i = 0; i < N; i++) {
        sem_post(&b->priv_send[i]);
    }

    sem_post(&b->mutex);
}

void send(int num)
{
    start_send(&buffer, num);
    do_send(&buffer, num);
    end_send(&buffer);
}

void receive()
{
    start_receive(&buffer);
    do_receive(&buffer);
    end_receive(&buffer);
}

void *M(void *arg)
{
    while (1) {
        send((int)arg);
    }
}

void *R(void *arg)
{
    while (1) {
        receive();
    }
}

int main()
{
    pthread_attr_t attr;
    pthread_t thread;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    init_buffer(&buffer);

    pthread_create(&thread, &attr, R, NULL);
    pthread_create(&thread, &attr, M, (void *)0);
    pthread_create(&thread, &attr, M, (void *)1);
    pthread_create(&thread, &attr, M, (void *)2);
    pthread_create(&thread, &attr, M, (void *)3);
    pthread_create(&thread, &attr, M, (void *)4);

    sleep(20);

    return 0;
}