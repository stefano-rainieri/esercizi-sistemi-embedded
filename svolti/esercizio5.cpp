#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define INDEXARGS 99
#define NUMVOTANTI 13 //must be DISPARI

void pausetta(void){
  struct timespec t;
  t.tv_sec = 0;
  t.tv_nsec = (rand()%10+1)*10000000;
  nanosleep(&t,NULL);
}

//Struttura condivisa
struct urna_t {
  pthread_mutex_t mutex;
  pthread_cond_t maggioranzaVoti;
  int voti[2];
  int maggioranzaRaggiunta;
  int vincitore;
} urna;

/* inizializzazione della struttura condivisa */
void init_urna(struct urna_t *u){
  /* mutex and condition variables private */
  pthread_condattr_t cattr;
  pthread_mutexattr_t mutexatrr;

  // Attribute initialization
  pthread_mutexattr_init(&mutexatrr);
  pthread_condattr_init(&cattr);

  // mutex and condition variable initialization
  pthread_mutex_init(&u->mutex,&mutexatrr);
  pthread_cond_init(&u->maggioranzaVoti,&cattr);

  // Attribute destruction
  pthread_mutexattr_destroy(&mutexatrr);
  pthread_condattr_destroy(&cattr);

  /* stato del sistema */
  u->voti[0] = 0;
  u->voti[1] = 0;
  u->maggioranzaRaggiunta = 0;
  u->vincitore = 99;
}

void unlock_mutex(void *m){
  pthread_mutex_unlock((pthread_mutex_t *)m);
}

int checkMaggioranza(){
  if(urna.voti[0] >= (NUMVOTANTI/2+1)) return 0;
  if(urna.voti[1] >= (NUMVOTANTI/2+1)) return 1;
  return 2;
}

void vota(int voto, int procNumber){
  pausetta();
  pthread_mutex_lock(&urna.mutex);
    urna.voti[voto]++;
    fprintf(stderr,"Processo: %d Ha votato %d\n",procNumber, voto);
    if(checkMaggioranza() != 2){
      urna.maggioranzaRaggiunta = 1;
      pthread_cond_broadcast(&urna.maggioranzaVoti);
    }
  pthread_mutex_unlock(&urna.mutex);
}

int risultato(void){
  int vincitore;
  pthread_mutex_lock(&urna.mutex);
    while (urna.maggioranzaRaggiunta != 1) {
      pthread_cleanup_push(unlock_mutex,(void *)&urna.mutex);
      pthread_cond_wait(&urna.maggioranzaVoti,&urna.mutex);
      pthread_cleanup_pop(0);
    }

    if(urna.vincitore == 99){
      if(urna.voti[0] > urna.voti[1]) urna.vincitore = 0;
      else urna.vincitore = 1;
    }
    vincitore = urna.vincitore;
  pthread_mutex_unlock(&urna.mutex);

  return vincitore;
}

void *bodyVotante(void *arg){
  int procNumber = (*(int *)arg);
  int voto = rand()%2;
  vota(voto, procNumber);
  if(voto == risultato()) fprintf(stderr,"Processo: %d Ha vinto\n",procNumber);
  else fprintf(stderr,"Processo: %d Ha perso\n",procNumber);
  pthread_exit(0);
}

int main(){

  //------------------ VARIABLES AND THREAD ------------------
  pthread_attr_t myattr;
  pthread_t thread[NUMVOTANTI];
  int err;
  int num[INDEXARGS];

  //------------------ INITIALIZATION RANDOM GENERATOR ------------------
  srand(time(NULL));

  //------------------ INITIALIZATION STRUTTURA CONDIVISA ------------------
  init_urna(&urna);

  //------------------ THREAD ATTRIBUTE INITIALIZATION ------------------
  /* initializes the thread attribute */
  pthread_attr_init(&myattr);
  //pthread_attr_setdetachstate(&myattr, PTHREAD_CREATE_DETACHED);

  //------------------ THREAD CREATION ------------------
  for (int i=0; i < INDEXARGS; i++) {
    num[i] = i;
  }

  for (int i=0; i<NUMVOTANTI;i++ ) {
    pthread_create(&thread[i], &myattr, bodyVotante, (void*) (&num[i]));
  }

  //------------------ THREAD ATTRIBUTE DESTRUCTION ------------------
  pthread_attr_destroy(&myattr);

  //------------------ THREAD CANCELLATION ------------------
  for (int i=0; i<NUMVOTANTI;i++ ) {
    pthread_join(thread[i],NULL);
  }

  return 0;
}

