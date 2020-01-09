#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define NUMPRODUCER 5
#define NUMCONSUMER 5
#define CALENGHT 10
#define INDEXARGS 99

enum priorita {MAX,MED,MIN};

void pausetta(void){
  struct timespec t;
  t.tv_sec = 0;
  t.tv_nsec = (rand()%10+1)*10000000;
  nanosleep(&t,NULL);
}

//Struttura condivisa
struct circularArray_t {
  int array[CALENGHT];
  int head, tail;
  sem_t full, empty;
  sem_t mutex;
  /* stato del sistema */
} circularArray;

/* inizializzazione della struttura condivisa */
void init_circularArray(struct circularArray_t *ca){
  /* semafori e contatori privati */
  sem_init(&ca->full, 0, CALENGHT);
  sem_init(&ca->empty, 0, 0);
  /* mutua esclusione */
  sem_init(&ca->mutex, 0, 1);
  /* stato del sistema */
  ca->head = ca->tail = 0;

  for (int i=0;i<CALENGHT;i++) {
    ca->array[i] = 0;
  }
}

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
}

void insert(int number){
    sem_wait(&circularArray.full);
      sem_wait(&circularArray.mutex);

        circularArray.array[circularArray.head] = number; //Inserting number in array
        circularArray.head = (circularArray.head + 1) % CALENGHT;
        fprintf(stderr,"\nProducer: %d inserisce %d", number, number);
        printCA();

      sem_post(&circularArray.mutex);
    sem_post(&circularArray.empty);
}

void extract(int number, int* extracted){
  sem_wait(&circularArray.empty);
     sem_wait(&circularArray.mutex);

      *extracted = circularArray.array[circularArray.tail]; //Extracting number in array
      circularArray.tail = (circularArray.tail + 1) % CALENGHT;
      fprintf(stderr,"\nConsumer: %d preleva %d", number, *extracted);
      printCA();

    sem_post(&circularArray.mutex);
  sem_post(&circularArray.full);
}

void *bodyProducer(void *arg){
  int number = *((int *) arg);
  for (;;) {
    insert(number);
    pausetta();
  }
  return 0;
}

void *bodyConsumer(void *arg){
  int number = *((int *) arg);
  int extracted = 0;
  for (;;) {
    extract(number, &extracted);
    pausetta();
  }
  return 0;
}

int main(){

  //------------------ VARIABLES AND THREAD ------------------
  pthread_attr_t myattr;
  pthread_t threadConsumer;
  pthread_t threadProducer;
  int err;
  int num[INDEXARGS];

  //------------------ INITIALIZATION RANDOM GENERATOR ------------------
  srand(time(NULL));

  //------------------ INITIALIZATION STRUTTURA CONDIVISA ------------------
  init_circularArray(&circularArray);

  //------------------ THREAD ATTRIBUTE INITIALIZATION ------------------
  /* initializes the thread attribute */
  pthread_attr_init(&myattr);
  pthread_attr_setdetachstate(&myattr, PTHREAD_CREATE_DETACHED);

  //------------------ THREAD CREATION ------------------
  /* creation and activation of the new thread */
  for (int i=0; i < INDEXARGS; i++) {
    num[i] = i;
  }

  for (int i=0; i<NUMCONSUMER; i++) {
    err = pthread_create(&threadConsumer, &myattr, bodyConsumer, (void*) (&num[i]));
    if(err) fprintf(stderr,"errore creazione threadConsumer: %u \n", i);
  }
  for (int i=0; i<NUMPRODUCER; i++) {
    err = pthread_create(&threadProducer, &myattr, bodyProducer, (void*) (&num[i]));
    if(err) fprintf(stderr,"errore creazione threadProducer: %u \n", i);
  }

  //------------------ THREAD ATTRIBUTE DESTRUCTION ------------------
  pthread_attr_destroy(&myattr);

  //------------------ THREAD CANCELLATION ------------------
  sleep(5); //Sleep 5 seconds before stop everithings

  return 0;
}

