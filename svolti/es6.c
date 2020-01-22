// esercizio 6 --> filosofi


#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <stdio.h>

#define NUM_FILOSOFI 5 // numero di filosofi ( e quindi di forchette )
#define DELAY 1200


// utilizzo delle condition variable 



int i ;

struct mysem_t {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
    int forchette[NUM_FILOSOFI];
}mysem;

void mysem_init(struct mysem_t* m)
{
    pthread_mutexattr_t ma;
    pthread_condattr_t ca;
    for (i = 0 ; i < NUM_FILOSOFI; i++)
    {
        m->forchette[i] = 0; // 0 = libera
    }

    
    pthread_mutexattr_init(&ma);
    pthread_mutex_init(&m->mutex, &ma);
    pthread_mutexattr_destroy(&ma);

    pthread_condattr_init(&ca);
    pthread_cond_init(&m->cond, &ca);
    pthread_condattr_destroy(&ca);
}

void pensa(int f)
{
    printf("%d sta pensando\n",f);
    for(i = 0; i<DELAY; i++)
    {
        
    }
}
void mangia(int f)
{
    if(f==0) // il filosofo 0 prende prima sx poi destra
    {
        int fork1 = 0;
        while (1)
        {
            pthread_mutex_lock(&mysem.mutex);
            if(mysem.forchette[NUM_FILOSOFI-1] == 0)
            {
                printf("%d ha in mano la forchetta %d\n",f,NUM_FILOSOFI-1);
                // PRENDO FORCHETTA
                mysem.forchette[NUM_FILOSOFI-1] = 1;

                fork1 = 1;
            }
            else
            {
                // ASPETTO CHE QUALCUNO LASCI UNA FORCHETTA
                pthread_cond_wait(&mysem.cond, &mysem.mutex);
            }
            if(fork1 == 1 && mysem.forchette[0]== 0)
            {
                //POSSO PRENDERE LA SECONDA FORCHETTA
                mysem.forchette[0] = 1;
                printf("%d prende anche la forchetta e mangia\n",f,f);

                for(i = 0; i<NUM_FILOSOFI;i++)
                {

                }
                // POSSO LASCIARE LE FORCHETTE
                mysem.forchette[NUM_FILOSOFI-1] = 0;
                mysem.forchette[0] = 0;
                printf("%d lascia entrambe le forchette\n",f);
            }
            else
            {
                // LASCIO LA FORCHETTA PRESA
                fork1 = 0;
                mysem.forchette[NUM_FILOSOFI-1] = 0;
                pthread_cond_wait(&mysem.cond, &mysem.mutex);
                printf("%d lascia la forchetta %d ( non era disponibile la seconda)",NUM_FILOSOFI-1,f);
            }

             pthread_mutex_unlock(&mysem.mutex);
        }        
    }
    else // gli altri prendono prima dx e poi sx
    {
        int fork1 = 0;
        while (1)
        {
            pthread_mutex_lock(&mysem.mutex);
            if(mysem.forchette[f] == 0)
            {
                printf("%d ha in mano la forchetta %d\n",f,f);
                // PRENDO FORCHETTA
                mysem.forchette[f] = 1;
                fork1 = 1;
            }
            else
            {
                // ASPETTO CHE QUALCUNO LASCI UNA FORCHETTA
                pthread_cond_wait(&mysem.cond, &mysem.mutex);
            }
            if(fork1 == 1 && mysem.forchette[f-1]== 0)
            {
                //POSSO PRENDERE LA SECONDA FORCHETTA
                mysem.forchette[f-1] = 1;
                printf("%d prende anche la forchetta %d e mangio\n",f,f);
                for(i = 0; i<NUM_FILOSOFI;i++)
                {

                }
                // POSSO LASCIARE LE FORCHETTE
                mysem.forchette[f-1] = 0;
                mysem.forchette[f] = 0;
                printf("%d lascia entrambe le forchette\n",f);

            }
            else
            {
                // LASCIO LA FORCHETTA PRESA
                fork1 = 0;
                mysem.forchette[f] = 0;
                pthread_cond_wait(&mysem.cond, &mysem.mutex);
                printf("%d lascia la forchetta %d ( non era disponibile la seconda)",f,f);

            }

             pthread_mutex_unlock(&mysem.mutex);
        }        
    }
    


}

void *body ( void * arg)
{
    int filosofo = *(int*)arg;
    while (1)
    {
        pensa(filosofo);
        mangia(filosofo);
    }
}



int main () 
{
    pthread_t t[NUM_FILOSOFI];
    pthread_attr_t ta;
    int err; 
    
    mysem_init (&mysem);
    pthread_attr_init(&ta);
    int p[NUM_FILOSOFI];
    for ( i = 0; i< NUM_FILOSOFI; i++)
    {
        p[i] = i;
        err = pthread_create(&t[i], &ta, body, &p[i]);
        if(err != 0 )
        {
            printf("errore nella creazione dei thread\n");
            exit(-1);
        }
    }

    pthread_attr_destroy(&ta);
    
    printf("\n");   
	sleep(3); 
    return 0;
}
