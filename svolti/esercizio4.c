/*
    ESERCIZIO 4
    
    Scrivere un programma multi-thread che simuli il gioco della morra cinese. 
    In tale programma ci devonoessere 3 thread:
        - 2 thread simulano i giocatori;
        - 1 thread simula l'arbitro.
    
    Il thread arbitro ha il compito di:
        1. "dare il via" ai due thread giocatori;
        2. aspettare che ciascuno di essi faccia la propria mossa;
        3. controllare chi dei due ha vinto, e stampare a video il risultato;
        4. aspettare la pressione di un tasto da parte dell'utente;
        5. ricominciare dal punto 1.

    Ognuno dei due thread giocatori deve:
        1. aspettare il "via" da parte del thread arbitro;
        2. estrarre a caso la propria mossa;
        3. stampare a video la propria mossa;
        4. segnalare al thread arbitro di aver effettuato la mossa;
        5. tornare al punto 1.

    Per semplicita', assumere che la mossa sia codificata come un numero intero con le seguenti define:
        #define CARTA     0
        #define SASSO     1
        #define FORBICE   2
    
    e che esista un array di stringhe cosi' definito:
        char *nomi_mosse[3] = {"carta", "sasso", "forbice"};
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define READY   7
#define PLAY    8
#define SCORE   9

#define STONE   0
#define PAPER   1
#define SCISSOR 2

char *move_names[3] = { "✊", "🖐", "✌️" };

struct game_t {
    pthread_mutex_t mutex;
    pthread_cond_t priv_refree;
    pthread_cond_t priv_players;

    int active_players, blocked_players;
    int moves[2];
    int state;
} game;

void init_game(struct game_t *g) 
{
    pthread_mutexattr_t mutexattr;
    pthread_condattr_t condattr;

    pthread_mutexattr_init(&mutexattr);
    pthread_condattr_init(&condattr);

    pthread_mutex_init(&g->mutex, &mutexattr);
    pthread_cond_init(&g->priv_refree, &condattr);
    pthread_cond_init(&g->priv_players, &condattr);
    
    pthread_mutexattr_destroy(&mutexattr);
    pthread_condattr_destroy(&condattr);
 
    g->active_players = 0;
    g->blocked_players = 0;
    g->state = READY;
    
    g->moves[0] = -1;
    g->moves[1] = -1;

    srand((unsigned int) time(0));
}

void kick_off(struct game_t *g)
{
    pthread_mutex_lock(&g->mutex);

    printf("\nREADY?\n");
    sleep(1);
    printf("GO!\n ");
    sleep(1);

    g->state = PLAY;
    if (g->blocked_players > 0) {
        pthread_cond_broadcast(&g->priv_players);
    }

    pthread_mutex_unlock(&g->mutex);
}

void wait_moves(struct game_t *g)
{
    pthread_mutex_lock(&g->mutex);

    while (SCORE != g->state) {
        pthread_cond_wait(&g->priv_refree, &g->mutex);
    }

    pthread_mutex_unlock(&g->mutex);
}

void announce_winner(struct game_t *g)
{
    pthread_mutex_lock(&g->mutex);

    int winner = 0;

    switch (g->moves[0]) {
        case STONE:
            if (PAPER == g->moves[1]) {
                winner = 2;
            } else if (SCISSOR == g->moves[1]) {
                winner = 1;
            }
            break;

        case PAPER:
            if (STONE == g->moves[1]) {
                winner = 1;
            } else if (SCISSOR == g->moves[1]) {
                winner = 2;
            }
            break;

        case SCISSOR:
            if (STONE == g->moves[1]) {
                winner = 2;
            } else if (PAPER == g->moves[1]) {
                winner = 1;
            }
            break;
    }
    
    0 == winner ? printf("\n---------------\nTIE\n") : printf("\n--------------\nPLAYER %d WINS! \n", winner);
    
    getchar();
    g->active_players = 0;
    g->state = READY;
    
    g->moves[0] = -1;
    g->moves[1] = -1;


    pthread_mutex_unlock(&g->mutex);
}

void wait_kick_off(struct game_t *g, int player)
{
    pthread_mutex_lock(&g->mutex);

    while (PLAY != g->state || -1 != g->moves[player]) {
        g->blocked_players++;
        pthread_cond_wait(&g->priv_players, &g->mutex);
        g->blocked_players--;
    }

    g->active_players++;

    pthread_mutex_unlock(&g->mutex);
}

int play(int player)
{
    int move = rand() % 3;
    printf("\nPLAYER %d -> %s", player + 1, move_names[move]);

    return move;
}

void report_move(struct game_t *g, int player, int move)
{
    pthread_mutex_lock(&g->mutex);

    g->moves[player] = move;
    g->active_players--;
    
    if (0 == g->active_players) {
        g->state = SCORE;
        pthread_cond_signal(&g->priv_refree);
    }

    pthread_mutex_unlock(&g->mutex);
}

void delay()
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 10 + 1) * 1000000;
    nanosleep(&t, NULL);
}

void *refree()
{
    while (1) {
        kick_off(&game);
        wait_moves(&game);
        announce_winner(&game);
    }

    return 0;
}

void *player(void *arg)
{
    while (1) {
        wait_kick_off(&game, (int)arg);
        delay();
        int move = play((int)arg);
        report_move(&game, (int)arg, move);
    }

    return 0;
}

int main() 
{
    pthread_attr_t attr;
    pthread_t thread;

    init_game(&game);

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_create(&thread, &attr, refree, NULL);
    pthread_create(&thread, &attr, player, (void *)0);
    pthread_create(&thread, &attr, player, (void *)1);

    pthread_attr_destroy(&attr);
    sleep(15);

    return 0;
}
