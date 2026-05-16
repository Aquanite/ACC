#include <backend.h>

extern bool run_arm64_backend(compilation_unit_t* unit);
extern bool run_x86_64_backend(compilation_unit_t* unit);

backend_type_t current_backend = 
#if defined(__aarch64__)
    BACKEND_ARM64;
#else
    BACKEND_X86_64;
#endif

backend_t backends[] = {
    { BACKEND_ARM64, "arm64" },
    { BACKEND_X86_64, "x86_64" },
};

backend_t* query_backends(string name) {
    for (size_t i = 0; i < sizeof(backends) / sizeof(backend_t); i++) {
        if (STRCMP(backends[i].name, name)) {
            return &backends[i];
        }
    }
    return NULL;
}

bool run_backend(backend_t *backend, compilation_unit_t *unit)
{
    NULLCHECK(unit);
    NULLCHECK(backend);

    if (backend->type == BACKEND_ARM64) {
        if (unit->verbose) printf("Running ARM64 backend on %s\n", unit->in_file);
        return run_arm64_backend(unit);
    } else if (backend->type == BACKEND_X86_64) {
        if (unit->verbose) printf("Running x86_64 backend on %s\n", unit->in_file);
        return run_x86_64_backend(unit);
    }

    fprintf(stderr, "Unsupported backend: %s\n", backend->name);
    return false;
}
