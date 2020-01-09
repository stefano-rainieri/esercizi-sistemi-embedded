#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

enum stati {STATOA,STATOB,STATOC};

void pausetta(void){
  struct timespec t;
  t.tv_sec = 0;
  t.tv_nsec = (rand()%10+1)*10000000;
  nanosleep(&t,NULL);
}

//Struttura condivisa
struct gestore_t {
  sem_t mutex;
  sem_t privReader, privWriter;
  int contBlocReader, contBlocWriter;
  int contActiveReader, contActiveWriter;

  /* stato del sistema */
  stati stato;
} gestore;

/* inizializzazione della struttura condivisa */
void init_gestore(struct gestore_t *g){
  /* mutex and condition variables private */
  sem_init(&g->mutex,0,1);
  sem_init(&g->privReader,0,0);
  sem_init(&g->privWriter,0,0);

  /* stato del sistema */
  g->contBlocReader = g->contBlocWriter = 0;
  g->contActiveReader = g->contActiveWriter = 0;
  g->stato = STATOA;
}

void *bodyReader(void *arg){
  char procName = (*(char *)arg);
  for (;;) {
    //-------Starting--------
    sem_wait(&gestore.mutex);
      if( gestore.contBlocWriter > 0 || gestore.contActiveWriter > 0){
        gestore.contBlocReader++;
      }
      else {
        gestore.contActiveReader++;
        sem_post(&gestore.privReader);
      }
    sem_post(&gestore.mutex);
    sem_wait(&gestore.privReader);
    //----------------------

    //-----Read buffer------
    fprintf(stderr,"Reader: %c legge\n",procName);
    //----------------------

    //-------Closing--------
    sem_wait(&gestore.mutex);
      gestore.contActiveReader--;
      if(gestore.contBlocWriter > 0 && gestore.contActiveReader == 0){
        gestore.contBlocWriter--;
        gestore.contActiveWriter++;
        sem_post(&gestore.privWriter);
      }
    sem_post(&gestore.mutex);
    //----------------------

    pausetta();
  }
  return 0;
}

void *bodyWriter(void *arg){
  char procName = (*(char *)arg);
  for (;;) {
    //-------Starting--------
    sem_wait(&gestore.mutex);
      if( gestore.contActiveReader > 0 || gestore.contActiveReader > 0){
        gestore.contBlocWriter++;
      }
      else {
        gestore.contActiveWriter++;
        sem_post(&gestore.privWriter);
      }
    sem_post(&gestore.mutex);
    sem_wait(&gestore.privWriter);
    //----------------------

    //-----Read buffer------
    fprintf(stderr,"Writer: %c scrive\n",procName);
    //----------------------

    //-------Closing--------
    sem_wait(&gestore.mutex);
      gestore.contActiveWriter--;
      if (gestore.contBlocReader > 0){
        while (gestore.contBlocReader > 0) {
          gestore.contBlocReader--;
          gestore.contActiveReader++;
          sem_post(&gestore.privReader);
        }
      }else if (gestore.contBlocWriter > 0) {
        gestore.contBlocWriter--;
        gestore.contActiveWriter++;
        sem_post(&gestore.privWriter);
      }
    sem_post(&gestore.mutex);
    //----------------------

    pausetta();
  }
  return 0;
}

int main(){

  //------------------ VARIABLES AND THREAD ------------------
  pthread_attr_t myattr;
  pthread_t thread;
  int err;

  //------------------ INITIALIZATION RANDOM GENERATOR ------------------
  srand(time(NULL));

  //------------------ INITIALIZATION STRUTTURA CONDIVISA ------------------
  init_gestore(&gestore);

  //------------------ THREAD ATTRIBUTE INITIALIZATION ------------------
  /* initializes the thread attribute */
  pthread_attr_init(&myattr);
  pthread_attr_setdetachstate(&myattr, PTHREAD_CREATE_DETACHED);

  //------------------ THREAD CREATION ------------------
  pthread_create(&thread, &myattr, bodyReader, (void *)"R");
  pthread_create(&thread, &myattr, bodyReader, (void *)"r");

  pthread_create(&thread, &myattr, bodyWriter, (void *)"W");
  pthread_create(&thread, &myattr, bodyWriter, (void *)"w");

  //------------------ THREAD ATTRIBUTE DESTRUCTION ------------------
  pthread_attr_destroy(&myattr);

  //------------------ THREAD CANCELLATION ------------------
  sleep(5); //Sleep 5 seconds before stop everithings

  return 0;
}

