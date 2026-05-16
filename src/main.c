#include <libstellar/stellar.h>
#include <version.h>
#include <lexer.h>
#include <parser.h>
#include <backend.h>

void print_version(void) {
    printf("ACC - %s\n", ACC_VERSION);
    printf("Copyright (C) Aquanite 2026\n");
    printf("Written by AzureianGH\n");
    printf("This is free software; see the source for copying conditions.  There is NO\nwarranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
}

int main(int argc, string argv[]) {
    string src = "";
    string filein = "";
    string fileout = "a.out";
    string backend_name = "";
    if (argc > 1) {
        // modular arg parsing, support -o <output> <input> or <input> -o <output>
        for (int i = 1; i < argc; i++) {
            if (STRCMP(argv[i], "-o") && i + 1 < argc) {
                fileout = argv[i + 1];
                i++; // skip next arg since it's the output file
            } else if (STRCMP(argv[i], "--version")) {
                print_version();
                return 0;
            } else if (STRCMP(argv[i], "--help")) {
                printf("Usage: acc [options] <input>\n");
                printf("Options:\n");
                printf("  -o <output>   Specify output file (default: a.out)\n");
                printf("  -m <backend>  Select backend (arm64, x86_64)\n");
                printf("  --version     Show version information\n");
                printf("  --help        Show this help message\n");
                return 0;
            } else if (STRNCMP(argv[i], "-m", 2) && argv[i][2] != '\0') {
                backend_name = &argv[i][2];
            } else if (STRCMP(argv[i], "-m") && i + 1 < argc) {
                backend_name = argv[i + 1];
                i++;
            } else {
                filein = argv[i];
            }
        }
    }

    // read file
    if (filein[0] != '\0') {
        FILE* f = fopen(filein, "r");
        if (!f) {
            fprintf(stderr, "Could not open file: %s\n", filein);
            return -1;
        }

        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);

        src = malloc(fsize + 1);
        fread(src, 1, fsize, f);
        src[fsize] = '\0';

        fclose(f);
    } else {
        fprintf(stderr, "No input file provided.\n");
        return -1;
    }

    lexer_t* lexer = new_lexer(src, filein);

    token_list_t* tokens = run_lexer(lexer); (void)fetch_token(&tokens);

    parser_t* parser = new_parser(&tokens, &lexer);
    (void)run_parser(parser);

    compilation_unit_t unit = { .parser = parser, .in_file = filein, .out_file = fileout, .verbose = false };

    backend_t* backend = NULL;
    if (backend_name[0] != '\0') {
        backend = query_backends(backend_name);
        if (!backend) {
            fprintf(stderr, "Unknown backend: %s\n", backend_name);
            free(lexer);
            return -1;
        }
    } else {
#if defined(__aarch64__)
        backend = query_backends("arm64");
#else
        backend = query_backends("x86_64");
#endif
    }

    if (!run_backend(backend, &unit)) {
        fprintf(stderr, "Backend failed\n");
        free(lexer);
        return -1;
    }

    free(lexer);

    return 0;
}
