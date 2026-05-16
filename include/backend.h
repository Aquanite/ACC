#ifndef ACC_BACKEND_H
#define ACC_BACKEND_H

#include <libstellar/stellar.h>
#include <parser.h>

enum_t {
    BACKEND_ARM64,
    BACKEND_X86_64,
} backend_type_t;

struct_t {
    parser_t* parser;
    string in_file;
    string out_file;
    bool verbose;
} compilation_unit_t;

struct_t {
    backend_type_t type;
    string name;
} backend_t;

backend_t* query_backends(string name);
bool run_backend(backend_t* backend, compilation_unit_t* unit);

#endif // ACC_BACKEND_H
