#include <libstellar/memory.h>

#ifndef STEL_NO_MEM
uint64_t _stel_ptr_allcs = 0;
_stel_memhdr* _stel_g_alloc = NULL;

void* _stel_alloc(size_t size) {
    _stel_memhdr* header = malloc(sizeof(_stel_memhdr) + size);
    if (!header)
        return NULL;

    header->size = size;
    header->next = _stel_g_alloc;
    header->prev = NULL;

    if (_stel_g_alloc) {
        _stel_g_alloc->prev = header;
    }

    _stel_g_alloc = header;
    _stel_ptr_allcs++;

    return header + 1;
}

void _stel_free(void* ptr) {
    NULLCHECK(ptr);

    _stel_memhdr* header = (_stel_memhdr*)ptr - 1;

    NULLCHECK(header);

    if (header->prev) header->prev->next = header->next;
    if (header->next) header->next->prev = header->prev;

    free(header);

    _stel_ptr_allcs--;
}

void _stel_right_allocations(void) {
    if (_stel_ptr_allcs != 0)
        STEL_ERROR("unfreed stellar allocations remain");

    return;
}
#endif // STEL_NO_MEM
