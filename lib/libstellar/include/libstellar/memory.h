#ifndef LIBSTELLAR_MEMORY_H
#define LIBSTELLAR_MEMORY_H

#include <libstellar/core.h>

#ifndef STEL_NO_MEM
typedef struct _stel_memhdr {
    size_t size;
    struct _stel_memhdr* next;
    struct _stel_memhdr* prev;
} _stel_memhdr;

extern uint64_t _stel_ptr_allcs;
extern _stel_memhdr* _stel_g_alloc;

void* _stel_alloc(size_t size);
void _stel_free(void* ptr);
void _stel_right_allocations(void);

#define STALLOC _stel_alloc
#define STFREE _stel_free
#define STRIGHTALLOC _stel_right_allocations
#endif // STEL_NO_MEM

#endif // LIBSTELLAR_MEMORY_H
