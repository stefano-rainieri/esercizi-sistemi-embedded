#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define NUMCLIENTI 20
#define NUMPOSTIDIVANO 4
#define NUMPOSTIBARBIERI 3
#define INDEXARGS 99

enum priorita {MAX,MED,MIN};

void pausetta(void){
  struct timespec t;
  t.tv_sec = 0;
  t.tv_nsec = (rand()%10+1)*100000000;
  nanosleep(&t,NULL);
}

//Struttura condivisa
struct circularArray_t {
  sem_t fullDivano, fullBarbieri;
  sem_t cassa;
  /* stato del sistema */
} circularArray;

/* inizializzazione della struttura condivisa */
void init_circularArray(struct circularArray_t *ca){
  /* semafori e contatori privati */
  sem_init(&ca->fullDivano, 0, NUMPOSTIDIVANO);
  sem_init(&ca->fullBarbieri, 0, NUMPOSTIBARBIERI);
  sem_init(&ca->cassa, 0, 1);
}
/*
void printCA(){
  fprintf(stderr,"\nCA: [");

  if(circularArray.head > circularArray.tail)
    for (int i=0; i< CALENGHT; i++) {
      if(i < circularArray.tail) fprintf(stderr," *");
      else if(i >= circularArray.head) fprintf(stderr," *");
      else fprintf(stderr," %d", circularArray.array[i]);
    }
  else if (circularArray.head < circularArray.tail)
    for (int i=0; i< CALENGHT; i++) {
      if(i < circularArray.tail && i >= circularArray.head) fprintf(stderr," *");
      else fprintf(stderr," %d", circularArray.array[i]);
    }
  else {
    int full;
    int empty;
    sem_getvalue(&circularArray.full,&full);
    sem_getvalue(&circularArray.empty,&empty);
    if(empty <= 0 ) //EMPTY ARRAY
      for (int i=0; i< CALENGHT; i++) {
        fprintf(stderr, " *");
      }
    else if(full <= 0 ) //FULL ARRAY
      for (int i=0; i< CALENGHT; i++) {
        fprintf(stderr, " %d",circularArray.array[i]);
      }
  }
  fprintf(stderr," ]\n");
}*/

void accediDivano(int number){
  sem_wait(&circularArray.fullDivano);
    int num = 0;
    sem_getvalue(&circularArray.fullDivano,&num);
    fprintf(stderr,"\nCliente: %d accede a divano, posti liberi: %d\n", number, num);
}

void accediBarbiere(int number){
  sem_wait(&circularArray.fullBarbieri);
  sem_post(&circularArray.fullDivano);
    int num=0;
    sem_getvalue(&circularArray.fullBarbieri,&num);
    fprintf(stderr,"\nCliente: %d accede al barbiere, barbieri liberi: %d\n", number, num);
    pausetta();
}

void accediCassa(int number){
  sem_wait(&circularArray.cassa);
  sem_post(&circularArray.fullBarbieri);
  pausetta();
  fprintf(stderr,"\nCliente: %d paga alla cassa", number);
  sem_post(&circularArray.cassa);
}

void *bodyCLiente(void *arg){
  int number = *((int *) arg);
  for (;;) {
    accediDivano(number);
    accediBarbiere(number);
    accediCassa(number);
    fprintf(stderr,"\nCliente: %d esce.\n", number);
    return 0;
  }
}

int main(){

  //------------------ VARIABLES AND THREAD ------------------
  pthread_attr_t myattr;
  pthread_t threadCliente;
  int err;
  void *res;
  int num[INDEXARGS];

  //------------------ INITIALIZATION RANDOM GENERATOR ------------------
  srand(time(NULL));

  //------------------ INITIALIZATION STRUTTURA CONDIVISA ------------------
  init_circularArray(&circularArray);

  //------------------ THREAD ATTRIBUTE INITIALIZATION ------------------
  /* initializes the thread attribute */
  pthread_attr_init(&myattr);
  //pthread_attr_setdetachstate(&myattr, PTHREAD_CREATE_DETACHED);

  //------------------ THREAD CREATION ------------------
  /* creation and activation of the new thread */
  for (int i=0; i < INDEXARGS; i++) {
    num[i] = i;
  }

  for (int i=0; i<NUMCLIENTI; i++) {
    err = pthread_create(&threadCliente, &myattr, bodyCLiente, (void*) (&num[i]));
    if(err) fprintf(stderr,"errore creazione threadCliente: %u \n", i);
  }

  //------------------ THREAD ATTRIBUTE DESTRUCTION ------------------
  pthread_attr_destroy(&myattr);

  //------------------ WAITING FOR THREAD TO JOIN ------------------
  for (int i=0; i<NUMCLIENTI; i++) {
    pthread_join(threadCliente, &res);
  }

  return 0;
}

