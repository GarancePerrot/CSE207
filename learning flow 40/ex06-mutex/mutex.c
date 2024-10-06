/*
 * mutex.c
 *
 *  Created on: Mar 19, 2016
 *      Author: jiaziyi
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NTHREADS 50
void *increase_counter(void *);


int  counter = 0;
pthread_mutex_t mutex= PTHREAD_MUTEX_INITIALIZER;

int main()
{
	int i=0;

	pthread_t *thread_group = malloc(sizeof(pthread_t) * NTHREADS);

	//create 50 threads of increase_counter, each thread adding 1 to the counter
	for (i=0 ; i<NTHREADS;i++){
		if(pthread_create(&thread_group[i],NULL,increase_counter,NULL)){
			fprintf(stderr, "Error creating thread\n");
			return 1;
    	};
	}	

	//allows a calling thread to wait for the completion of another thread: wait for all to finish
	for (i=0 ; i<NTHREADS;i++){
		if(pthread_join(thread_group[i],NULL)){
			fprintf(stderr, "Error joining threads\n");
			return 1;
    	};
	}	

	pthread_mutex_destroy(&mutex); //free memory of mutex
	printf("\nFinal counter value: %d\n", counter);
	return 0;

}

void *increase_counter(void *arg)
{
	while(1){
		pthread_mutex_lock(&mutex);            // START critical region
		if (counter>=NTHREADS) {
			pthread_mutex_unlock(&mutex);
			pthread_exit(NULL);
		}
		counter++;
		pthread_mutex_unlock(&mutex);          // END critical region

		printf("Thread number %ld, working on counter. The current value is %d\n", (long)pthread_self(), counter);
    }
}
