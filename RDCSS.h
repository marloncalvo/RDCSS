typedef struct RdcssDescriptor {
	uintptr_t *a1;
	uintptr_t  o1;
	uintptr_t *a2;
	uintptr_t  o2;
	uintptr_t  n2;
} RdcssDescriptor;

uintptr_t rdcss(RdcssDescriptor *d);
uintptr_t rdcss_read(uintptr_t *addr);
