/*

Esercizio 6

In questo compito verrà affrontato il celebre problema dei filosofi a tavola, che può essere così schematizzato:

Un certo numero N di filosofi siede intorno ad un tavolo circolare al cui centro c'è un piatto di spaghetti e su cui sono disposte N forchette (in modo che ogni filosofo ne abbia una alla sua destra e una alla sua sinistra). Ogni filosofo si comporta nel seguente modo:
	- Trascorre il suo tempo pensando e mangiando.
	- Dopo una fase di riflessione passa a una di nutrizione
	- Per mangiare acquisisce prima la forchetta destra, poi quella a sinistra
	- Una volta che ha finito di mangiare rimette le forchette a posto

Il candidato:
	- modelli le forchette come risorse condivise, associando quindi un semaforo a ciascuna forchetta, ed 		  ogni filosofo come un thread e ne scriva quindi il relativo codice
	- modelli le fasi di pensiero e nutrizione come dei cicli for a vuoto di lunghezza definita dalla 	    macro DELAY
	- definisca il numero di filosofi e anche di forchette usando la macro NUM_FILOSOFI
 	- si sincronizzi con al fine di tutti i thread

Si tenga presente che è stato dimostrato che, per evitare situazioni di deadlock, uno dei filosofi deve invertitre lìordine di acquisizione delle forchette (quindi prima la sinistra e poi la destra).
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <semaphore.h>

#define DELAY 100000000
#define N 5

void DELAYS(void);

void DELAYS(void){
 int i;
 for(i=0; i<DELAY; i++);
}

//struttura condivisa

struct gestore_t{
 pthread_mutex_t m;
 pthread_cond_t c;
 sem_t st[N];
 int index;
}g;

void init_system(){
 int i; 
 //dichiaro attributi mutex e cond var
 pthread_mutexattr_t mt;
 pthread_condattr_t ct;

 //inizializzo semafori
 for(i=0; i<N; i++){
   sem_init(&g.st[i], 0, 1);
 }
 
 g.index = 0;

 //inizializzo attributi mutex e cond var
 pthread_mutexattr_init(&mt);
 pthread_condattr_init(&ct);

 //inizializzo mutex e cond var
 pthread_mutex_init(&g.m, &mt);
 pthread_cond_init(&g.c, &ct);

 //distruggo attributi
 pthread_mutexattr_destroy(&mt);
 pthread_condattr_destroy(&ct);
 
}
 
void *filosofo(void *args){
 int fsx, fdx, filcurr, tmp;
 filcurr = *(int *)args;
 fdx = filcurr;

 if(filcurr == 0){ // se il filosofo corrente è il primo
   fsx = N - 1; // allora prende la forchetta sinistra (4), non la -1
 }else{
   fsx = filcurr - 1; // altrimenti normale
 }

 // se il filosofo è l'ultimo, inverto dx <-> sx per evitare deadlock!!
 if(filcurr == (N - 1)) 
 {
   tmp = fdx;
   fdx = fsx;
   fsx = tmp;
 }
 
 for(;;){
  printf("Filosofo %d --> PENSA.\n", filcurr);
  DELAYS();
  sem_wait(&g.st[fsx]); //blocco il semaforo per la forchetta di sinistra
  sem_wait(&g.st[fdx]); //blocco il semaforo per la forchetta di destra
  printf("Filosofo %d --> MANGIA.\n", filcurr);
  DELAYS();
  sem_post(&g.st[fsx]); //sblocco il semaforo per la forchetta di sinistra
  sem_post(&g.st[fdx]); //sblocco il semaforo per la forchetta di destra
  
  g.index++; // filosofi che mangiano al momento

  pthread_mutex_lock(&g.m); // blocco 
  printf("Filosofo %d --> ATTENDE.\n", filcurr);
  while(g.index < N){
    pthread_cond_wait(&g.c, &g.m); // i thread aspettano 
  }
  
  pthread_cond_broadcast(&g.c); //sblocco tutta la coda
  pthread_mutex_unlock(&g.m); //sblocco

  g.index = 0; //resetto
  printf("Filosofo %d --> RIPARTE.\n", filcurr);
 }
}


int main(){
 int i, nfil[N];
 pthread_t p[N]; //un thread per ogni filosofo !!
 pthread_attr_t a;

 //init di sistema?
 init_system();
 // f. pausetta ?

 pthread_attr_init(&a);
 pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

 for(i=0; i<N; i++){
   nfil[i] = i;
   pthread_create(&p[i], &a, filosofo, (void *)&nfil[i]);
 }

 pthread_attr_destroy(&a);

 sleep(3);

 return 0;
} 




















