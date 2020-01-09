#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define PROCNUMBER 4
#define CONDIVISENUMBER 2
#define INDEXARGS 99

enum priorita {MAX,MED,MIN};
typedef int T;

void pausetta(void){
  struct timespec t;
  t.tv_sec = 0;
  t.tv_nsec = (rand()%10+1)*100000000;
  nanosleep(&t,NULL);
}

//Struttura condivisa
struct Array_t {

  sem_t full;
  sem_t mutex;
  sem_t privSem[ARRLENGHT];
  T array[ARRLENGHT];
  int numBlocked;
  /* stato del sistema */
}Array;

/* inizializzazione della struttura condivisa */
void init_Array(struct Array_t *a){
  /* semafori e contatori privati */
  sem_init(&a->full, 0, 0);
  sem_init(&a->mutex, 0, 1);
  for (int i=0; i< ARRLENGHT; i++) {
    sem_init(&a->privSem[i], 0, 1);
  }
  a->numBlocked =0;
}

void printArray(){
  fprintf(stderr,"[");
  for (int i=0; i< ARRLENGHT; i++) {
    fprintf(stderr,"%d ", Array.array[i]);
  }
  fprintf(stderr," ]\n");
}

void send(int number){
    sem_wait(&Array.privSem[number]);

      sem_wait(&Array.mutex); //Not necessary only for printing purposes
        fprintf(stderr,"Mittente: %d scrive\n", number);
        Array.array[number] = number;
        printArray();
      sem_post(&Array.mutex); //Not necessary only for printing purposes

      pausetta();

      sem_wait(&Array.mutex);
        Array.numBlocked++;
        if(Array.numBlocked == ARRLENGHT){
          sem_post(&Array.full);
        }
      sem_post(&Array.mutex);
}

void receive(){
  sem_wait(&Array.full);
    sem_wait(&Array.mutex);
      fprintf(stderr,"Ricevente: ha ricevuto\n");
      printArray();
      Array.numBlocked = 0;
      for (int i=0; i< ARRLENGHT; i++) {
        Array.array[i] = 99;
      }
      fprintf(stderr,"Reset Array:\n");
      printArray();
    sem_post(&Array.mutex);

    for (int i = 0; i<ARRLENGHT; i++) {
      sem_post(&Array.privSem[i]);
    }
}

void *bodyMittente(void *arg){
  int number = *((int *) arg);
  for (;;) {
    send(number);
  }
  return 0;
}

void *bodyRicevente(void *arg){
  for (;;) {
    receive();
  }
  return 0;
}

int main(){

  //------------------ VARIABLES AND THREAD ------------------
  pthread_attr_t myattr;
  pthread_t threadMittente, threadRicevente;
  int err;
  void *res;
  int num[INDEXARGS];

  //------------------ INITIALIZATION RANDOM GENERATOR ------------------
  srand(time(NULL));

  //------------------ INITIALIZATION STRUTTURA CONDIVISA ------------------
  init_Array(&Array);

  //------------------ THREAD ATTRIBUTE INITIALIZATION ------------------
  /* initializes the thread attribute */
  pthread_attr_init(&myattr);
  pthread_attr_setdetachstate(&myattr, PTHREAD_CREATE_DETACHED);

  //------------------ THREAD CREATION ------------------
  /* creation and activation of the new thread */
  for (int i=0; i < INDEXARGS; i++) {
    num[i] = i;
  }

  for (int i=0; i< ARRLENGHT; i++) {
    err = pthread_create(&threadMittente, &myattr, bodyMittente, (void*) (&num[i]));
    if(err) fprintf(stderr,"errore creazione threadCliente: %u \n", i);
  }

  err = pthread_create(&threadRicevente, &myattr, bodyRicevente, NULL);
  if(err) fprintf(stderr,"errore creazione thread Ricevente\n");


  //------------------ THREAD ATTRIBUTE DESTRUCTION ------------------
  pthread_attr_destroy(&myattr);

  sleep(5); //Sleep for 5 second after that kill all process

  return 0;
}

