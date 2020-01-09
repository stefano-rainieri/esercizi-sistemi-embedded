#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

enum mosse {SASSO,CARTA,FORBICE,NULLO};

mosse calcolaMossa(){
  int num = static_cast<int>((rand() % 3));
  switch(num){
    case 0: return SASSO;
    case 1: return CARTA;
    case 2: return FORBICE;
    default: return SASSO;
  }
}

//Struttura condivisa
struct gestore_t {
  sem_t start;
  sem_t played;
  sem_t mutex;
  /* stato del sistema */
  mosse mossa1, mossa2;
  pthread_t *giocatore1, *giocatore2;
} gestore;

/* inizializzazione della struttura condivisa */
void init_gestore(struct gestore_t *g, pthread_t* giocatore1, pthread_t* giocatore2){
  /* mutua esclusione */
  sem_init(&g->start, 0, 0);
  /* semafori e contatori privati */
  sem_init(&g->played, 0, 2);
  sem_init(&g->mutex, 0, 1);
  /* stato del sistema */
  g->mossa1 = g->mossa2 = NULLO;
  g->giocatore1 = giocatore1;
  g->giocatore2 = giocatore2;
}

void *bodyGiocatore(void *arg){
  int number = (*(int*)arg);
  int ok = 0;
  while(true){
    sem_wait(&gestore.start);
    //Calcola figura
    sem_wait(&gestore.mutex);

    switch(number){
    case 1: if(gestore.mossa1 == NULLO){
               gestore.mossa1 = calcolaMossa();
               ok = 1;
               if(gestore.mossa1 == SASSO) fprintf(stderr,"Giocatore: %u, mossa: SASSO\n",number);
               else if(gestore.mossa1 == CARTA) fprintf(stderr,"Giocatore: %u, mossa: CARTA\n",number);
               else fprintf(stderr,"Giocatore: %u, mossa: FORBICE\n",number);
            }else{
              ok = 0;
              sem_post(&gestore.start);
            }
      break;
    case 2: if(gestore.mossa2 == NULLO){
            gestore.mossa2 = calcolaMossa();
            ok = 1;
            if(gestore.mossa2 == SASSO) fprintf(stderr,"Giocatore: %u, mossa: SASSO\n",number);
            else if(gestore.mossa2 == CARTA) fprintf(stderr,"Giocatore: %u, mossa: CARTA\n",number);
            else fprintf(stderr,"Giocatore: %u, mossa: FORBICE\n",number);
            }else{
              ok = 0;
              sem_post(&gestore.start);
            }
      break;
    }

    sem_post(&gestore.mutex);
    if(ok){
      sem_post(&gestore.played);
      ok = 0;
    }
  }
}

void *bodyArbitro(void *arg){
  while(true){

    sem_wait(&gestore.played);
    sem_wait(&gestore.played);

    sem_wait(&gestore.mutex);
    fprintf(stderr,"Arbitro decreta:  ");

    //Calcola vincitore
    if(gestore.mossa1 == gestore.mossa2){
      fprintf(stderr,"Pareggio\n\n");
    }
    else if(gestore.mossa1 == SASSO){
      if(gestore.mossa2 == FORBICE) fprintf(stderr,"Vincitore Giocatore 1\n\n");
      else if(gestore.mossa2 == CARTA) fprintf(stderr,"Vincitore Giocatore 2\n\n");
    }
    else if(gestore.mossa1 == CARTA){
      if(gestore.mossa2 == SASSO) fprintf(stderr,"Vincitore Giocatore 1\n\n");
      else if(gestore.mossa2 == FORBICE) fprintf(stderr,"Vincitore Giocatore 2\n\n");
    }
    else if(gestore.mossa1 == FORBICE){
      if(gestore.mossa2 == CARTA) fprintf(stderr,"Vincitore Giocatore 1\n\n");
      else if(gestore.mossa2 == SASSO) fprintf(stderr,"Vincitore Giocatore 2\n\n");
    }

    //Aggiorna comandi
    gestore.mossa1 = NULLO;
    gestore.mossa2 = NULLO;
    sem_post(&gestore.mutex);

    fprintf(stderr,"Vuoi continuare? --> (Y/N) ");
    char comando;
    scanf(" %c",&comando);//Aspetta comando utente

    if(comando == 'Y' || comando == 'y'){
      sem_post(&gestore.start);
      sem_post(&gestore.start);
    }else {
      pthread_cancel(*(gestore.giocatore1));
      pthread_cancel(*(gestore.giocatore2));
      return 0;
    }
  }
}


int main(){

  //------------------ VARIABLES ------------------
  pthread_attr_t myattr;
  pthread_t threadArbitro, threadGiocatore1, threadGiocatore2;
  unsigned long main_id;
  int err;
  void *returnvalue;

  //------------------ INITIALIZATION STRUTTURA CONDIVISA ------------------
  init_gestore(&gestore, &threadGiocatore1, &threadGiocatore2);

  //------------------ THREAD ATTRIBUTE INITIALIZATION ------------------
  /* initializes the thread attribute */
  pthread_attr_init(&myattr);

  srand(555);

  main_id = pthread_self();
  //puts("main: before pthread_create\n");
  printf("Main_id: %lu \n", main_id );

  //------------------ THREAD CREATION ------------------
  /* creation and activation of the new thread */
  int arg1 = 1, arg2 = 2;
  err = pthread_create(&threadArbitro, &myattr, bodyArbitro, NULL);
  if(err) printf("errore creazione threadA: %u \n", err);
  err = pthread_create(&threadGiocatore1, &myattr, bodyGiocatore, (void*) (&arg1) );
  if(err) printf("errore creazione threadB: %u \n", err);
  err = pthread_create(&threadGiocatore2, &myattr, bodyGiocatore, (void*) (&arg2) );
  if(err) printf("errore creazione threadReset: %u \n", err);

  //------------------ THREAD ATTRIBUTE DESTRUCTION ------------------
  /* the thread attribute is no more needed */
  pthread_attr_destroy(&myattr);

  //------------------ WAITHING THREAD TO JOIN ------------------
  /* wait the end of the thread we just created */
  pthread_join(threadArbitro, &returnvalue);
  pthread_join(threadGiocatore1, &returnvalue);
  pthread_join(threadGiocatore2, &returnvalue);

  return 0;
}

