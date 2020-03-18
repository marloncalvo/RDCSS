/*
 * RDCSS.c
 *
 * This RDCSS implementation utilizes bit marking for determining
 * if the value in the data section is a descriptor or not.
 * As such, this only works on word sized variables.
 *
 * For this implementation, and with the aformentioned restriction,
 * I am only allowing uintptr_t variables, which can point to other
 * memory addresses, or values.
 *
 * NOTE: the data section must have the lowest bit never populated, else
 * this will fail.
 *
 * Author: Marlon Calvo
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdatomic.h>
#include "RDCSS.h"

uintptr_t CAS1(uintptr_t *a, uintptr_t o, uintptr_t n);
void complete(RdcssDescriptor *d);
int is_descriptor(uintptr_t ptr);

uintptr_t rdcss(RdcssDescriptor *d) {
	uintptr_t r;
	uintptr_t d_ptr = (((uintptr_t)(void*)d) | 1);
	do {
		r = CAS1(d->a2, d->o2, d_ptr);
		if (is_descriptor(r)) {
			complete((RdcssDescriptor*)(r&~1));
		}
	} while (is_descriptor(r));
	if(r == d->o2) complete(d);
	return r;
}

uintptr_t rdcss_read(uintptr_t *addr) {
	uintptr_t r;
	do {
		r = atomic_load(addr);
		if(is_descriptor(r)) {
			complete((RdcssDescriptor*)(r&~1));
		}
	} while(is_descriptor(r));
	return r;
}

void complete(RdcssDescriptor *d) {
	// I chose to utilize atomic load here,
	uintptr_t v = atomic_load(d->a1);
	uintptr_t d_ptr = (((uintptr_t)(void*)d) | 1);
	if (v == d->o1) {
		if(CAS1(d->a2,d_ptr,d->n2)==d_ptr){
			
		}
	}
	else {
		CAS1(d->a2,d_ptr,d->o2);
	}
}

uintptr_t CAS1(uintptr_t *a, uintptr_t o, uintptr_t n) {
	uintptr_t o_t = o;
	atomic_compare_exchange_strong(a, &o_t, n);
	return o_t;
}


int is_descriptor(uintptr_t ptr) {
	return ptr & 1;
}
