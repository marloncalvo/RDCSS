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
#include <stdio.h>
#include "RDCSS.h"

#define N_THREADS 16
#define SECONDS 5.0

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
		//printf("count=%d\n",*((int*)(void*)old));

		// We need to allocate a new memory address, so that we do not change the old one.
		// Omitting this step causes `old` memory address to be updated. We only want to update
		// the address of the pointer that count points to, not the value of the pointed pointer.
		int *n_count = (int*)malloc(sizeof(int));
		*n_count = *((int*)(void*)old) + 1;

		// Notice d->o1 is 0, which is what we are treating as the flag being false.
		RdcssDescriptor *d = (RdcssDescriptor*)malloc(sizeof(RdcssDescriptor));
		d->a1 = &flag;
		d->o1 = 0;
		d->a2 = &count;
		d->o2 = old;
		d->n2 = (uintptr_t)(void*)n_count;

		rdcss(d);
	}

	return NULL;
}

int main(void) {
	int init_count = 0;

	count = (uintptr_t)(void*)&init_count;
	flag = 0;

	pthread_t threads[N_THREADS];
	for (int i = 0; i < N_THREADS; i++) {
		pthread_create( &threads[i], NULL, counter, NULL);
	}

	time_t start, end;
	time(&start);
	do time(&end); while(difftime(end, start) <= SECONDS);
	atomic_store(&flag,1); // @suppress("Type cannot be resolved")

	// Show that RDCSS still works after setting flag to false, but terminating
	// after that.
	time(&start);
	do time(&end); while(difftime(end, start) <= 5.0);

	atomic_store(&progress, 0);

	printf("%d\n", *((int*)(void*)count));
}
