/* negozio di barbieri 
* soluione con condition variable
* f.l.
*/
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>

#define SHAVING_ITERATIONS 1000
#define PAYING_ITERATIONS 100
#define CLIENTI 15
#define SEDIE 3
#define DIVANI 4

struct negozio_t{
	int barbieri;
	int divano;
	int cassiere;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
}negozio;

void negozio_init (struct negozio_t *negozio)
{
	negozio->cassiere = 0;
	int i;
	negozio->barbieri = 0;
	negozio->divano = 0;
	pthread_mutexattr_t ma;
	pthread_condattr_t ca;
	
	pthread_mutexattr_init(&ma);
	pthread_mutex_init(&negozio->mutex, &ma);
	pthread_mutexattr_destroy(&ma);
	
	pthread_condattr_init(&ca);
	pthread_cond_init(&negozio->cond, &ca);
	pthread_condattr_destroy(&ca);
		
}


void *client( void *arg)
{
	
	int i;
	int cliente = *(int*)arg;
	pthread_mutex_lock(&negozio.mutex);

	// posso entrare? 
	while(1)
	{
		if(negozio.divano <= DIVANI) // 
		{
			// mi siedo
			negozio.divano ++;
			printf("%d: mi siedo sul divano\n",cliente);
			break;
		}
		else
		{
			// aspetto
			printf("%d: non c'e' posto sul divano, aspetto fuori\n",cliente);
			pthread_cond_wait(&negozio.cond, &negozio.mutex);
		}
	}
		pthread_mutex_unlock(&negozio.mutex);
	pthread_mutex_lock(&negozio.mutex);

	// tocca a me?
	while(1)
	{
		if(negozio.barbieri <= SEDIE) // 
		{
			// mi siedo
			printf("%d: tocca a me, mi alzo dal divano\n",cliente);
			negozio.barbieri ++;
			negozio.divano --;
			pthread_cond_broadcast(&negozio.cond);// segnalo a tutti che si è liberato un posto sul divano
			for(i = 0; i<SHAVING_ITERATIONS; i++ ); // mi fanno la barba
			// ho finito -> devo andare a pagare
			negozio.barbieri --;
			printf("%d: ho finito di fare la barba, vado a pagare\n",cliente);

			pthread_cond_broadcast(&negozio.cond);
			break;
		}
		else
		{
			// aspetto
			printf("%d: sto aspettando seduto sul divano\n",cliente);

			pthread_cond_wait(&negozio.cond, &negozio.mutex);
		}
	}
		pthread_mutex_unlock(&negozio.mutex);
	pthread_mutex_lock(&negozio.mutex);

	// posso andare a pagare?
	while(1)
	{
		if(negozio.cassiere == 0) // 
		{
			// mi siedo
			negozio.cassiere ++;
			printf("%d: sto pagando\n",cliente);

			for(i = 0; i<PAYING_ITERATIONS; i++ ); // sto pagando...
			// ho finito -> esco
			negozio.cassiere --;
			printf("%d: ho finito di pagare, esco! ciao!!!\n",cliente);
			pthread_cond_broadcast(&negozio.cond);
			break;

		}
		else
		{
			// aspetto
			printf("%d: sto aspettando che si liberi il cassiere\n",cliente);

			pthread_cond_wait(&negozio.cond, &negozio.mutex);
		}
	}
	pthread_mutex_unlock(&negozio.mutex);
	return NULL;
	
}
int main()
{
	
	pthread_t t[CLIENTI];
    pthread_attr_t ta;
    int err; 
    int i;
    negozio_init(&negozio);
    pthread_attr_init(&ta);
    int p[CLIENTI];
    for ( i = 0; i< CLIENTI; i++)
    {
        p[i] = i;
        err = pthread_create(&t[i], &ta, client, &p[i]);
        if(err != 0 )
        {
            printf("errore nella creazione dei thread\n");
            exit(-1);
        }

    }
for ( i = 0; i< CLIENTI; i++)
    {
        pthread_join(t[i],NULL);
    }
    pthread_attr_destroy(&ta);
	
	printf("\n");
	return 0;
	
}
