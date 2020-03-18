/*
 * RDCSS_Test.c
 *
 * Tests RDCSS implementation, with word sized control and data section values.
 * uintptr_t is always word sized, and can point to any object (it is of maximal length).
 * Here, we are simply updating count to point to a new memory address, which stores a higher count.
 *
 * Flag is not a pointer (supposed to be bool), but it is stored in uintptr_t anyway.
 * This is to ensure that it is a word sized variable.
 *
 * Author: Marlon Calvo
 */

#include <stdatomic.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include "RDCSS.h"

#define N_THREADS 1
#define SECONDS 75

uintptr_t count; // This is treated as a pointer to a memory address.
uintptr_t flag; // This is treated as a bit for true / false.

int progress = 1; // just so we can tell threads to stop.

/*
 * This function is called by each thread, where it calls RDCSS to swap
 * count with count + 1 (data section), and checks flag to be 0 (control section).
 *
 * uintptr_t of value 0 is treated as (false) for Flag, and value 1 as (true).
 */
void* counter() {

	while(progress) {
		// Grab latest pointer from count. Needs to be rdcss_read because there may be a
		// descriptor at count.
		uintptr_t old = rdcss_read(&count);
		
		//printf("count=%ld\n",*((unsigned long long*)old));

		// We need to allocate a new memory address, so that we do not change the old one.
		// Omitting this step causes `old` memory address to be updated. We only want to update
		// the address of the pointer that count points to, not the value of the pointed pointer.
		unsigned long long *n_count_ptr = (unsigned long long*)malloc(sizeof(unsigned long long));
		*n_count_ptr = *((unsigned long long*)old) + 1;

		unsigned long long n_count = *n_count_ptr + 1;

		// Notice d->o1 is 0, which is what we are treating as the flag being false.
		RdcssDescriptor d;
		d.a1 = &flag;
		d.o1 = 0;
		d.a2 = &count;
		d.o2 = old;
		d.n2 = (uintptr_t)(void*)&n_count;

		rdcss(&d);
		free(n_count_ptr);
	}

	return NULL;
}

int main(int argc, char *argv[]) {

	int n_threads = N_THREADS;

	if(argc > 1) {
		int t_n_threads = atoi(argv[1]);
		if(t_n_threads > 0) {
			n_threads = t_n_threads;
		}
	}

	unsigned long long init_count = 0;
	count = (uintptr_t)(void*)&init_count;
	flag = 0;

	printf("Starting counter with %d thread(s).\n", n_threads);

	pthread_t threads[n_threads];
	for (int i = 0; i < n_threads; i++) {
		pthread_create( &threads[i], NULL, counter, NULL);
	}

	printf("...\n");

	// This is lazy, but accurate enough for our purposes.
	sleep(SECONDS);
	atomic_store(&flag,1);

	// Just to show that RDCSS will fail to update if the flag is false,
	// but allow for the program to end.
	sleep(3);
	atomic_store(&progress, 0);

	printf("Result=%ld\n", *((unsigned long long*)rdcss_read(&count)));
}
