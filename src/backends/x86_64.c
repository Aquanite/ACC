#include <backend.h>
#include <version.h>

#define QUADWORD 64
#define DOUBLEWORD 32
#define WORD 16
#define BYTE 8

struct_t {
    int index;
    bool is_occupied;
} register_slot_t;

typedef size_t stack_slot_t;

struct_t {
    bool is_register;
    int reg;
    stack_slot_t stack;
    size_t bytes_needed;
} slot_t;

static const int usable_registers[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };
static const string quadword_registers[] = { "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15" };
static const string doubleword_registers[] = { "eax", "ebx", "ecx", "edx", "esi", "edi", "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d" };
static const string word_registers[] = { "ax", "bx", "cx", "dx", "si", "di", "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w" };
static const string byte_registers[] = { "al", "bl", "cl", "dl", "sil", "dil", "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b" };
static const int return_register = 0;

static FILE* backend_out = NULL;
static compilation_unit_t* g_unit;

static register_slot_t register_pool[] = {
    {0,  false},
    {1,  false},
    {2,  false},
    {3,  false},
    {4,  false},
    {5,  false},
    {6,  false},
    {7,  false},
    {8,  false},
    {9,  false},
    {10, false},
    {11, false},
    {12, false},
    {13, false},
};

static stack_slot_t stack_slots = 0;

void emit_x86_64_function(function_t* func);
void emit_x86_64_return(statement_t* stmt);

__attribute__((noreturn))
void x86_64_error(compilation_unit_t* code, string str, size_t line, size_t column, string item) {
    NULLCHECK(code);
    NULLCHECK(str);

    fprintf(stderr, 
            "%s:%zu:%zu: \033[1;31mcodegen error:\033[0m %s\n"
            "  %zu  |  %s\n", 
            code->in_file, line, column, str,
            line, item);

    int digits = COUNTDIGITSU(line) + 4; // Account for spaces
    for (int i = 0; i < digits; i++)
        putc(' ', stderr);
    
    fprintf(stderr, "|  \033[1;31m^");

    size_t len = strlen(item) - 1;

    for (size_t i = 0; i < len; i++)
        putc('~', stderr);

    puts("\033[0m");
    
    exit(-1); 
}

void x86_64_assert(compilation_unit_t* code, int condition, string str, size_t line, size_t column, string item) {
    if (condition) 
        return;

    x86_64_error(code, str, line, column, item);
}

int total_registers(void) {
    return (int)(sizeof(register_pool) / sizeof(register_pool[0]));
}

void x86_64_reg_test(int reg) {
    char buf[30];

    if (reg >= total_registers() || 
        reg < 0) {
        snprintf(buf, sizeof(buf), "register: %d", reg);
        x86_64_error(g_unit, "unknown register", 0, 0, buf);
    }

    return;
}

string get_register_by_size(int reg, int width) {
    char buf[30];

    x86_64_reg_test(reg);

    switch (width) {
        case QUADWORD: return quadword_registers[reg];
        case DOUBLEWORD: return doubleword_registers[reg];
        case WORD: return word_registers[reg];
        case BYTE: return byte_registers[reg];
    }


    snprintf(buf, sizeof(buf), "width: %d", width);
    x86_64_error(g_unit, "unknown register size", 0, 0, buf);
}

int checkout_register(void) {
    for (int i = 0; i < total_registers(); i++) {
        register_slot_t reg = register_pool[i];

        if (reg.is_occupied)
            continue;
        
        reg.is_occupied = true;
        return reg.index;
    }

    return -1;
}

void checkin_register(int reg) {
    x86_64_reg_test(reg);

    if (!register_pool[reg].is_occupied) {
        fprintf(stderr, "CODEGEN FATAL: non-occupied register checked-in: %s", get_register_by_size(register_pool[reg].index, QUADWORD));
        exit(-1);
    }

    register_pool[reg].is_occupied = false;
    return;
}

bool is_available_register(void) {
    for (int i = 0; i < total_registers(); i++) {
        register_slot_t reg = register_pool[i];
        if (reg.is_occupied)
            continue;
        
        return true; // Found one!!
    }

    return false; // None found
}

stack_slot_t checkout_stack(size_t byte_amount) {
    size_t old = stack_slots;
    stack_slots += byte_amount;
    return old;
}

void clear_stack(void) {
    stack_slots = 0;
}

stack_slot_t align_stack(void) {
    return (stack_slots + 7) & ~7;
}

slot_t checkout(size_t needed) {
    ZEROCHECK(needed);

    if (needed <= 8) {
        int reg = checkout_register();
        if (reg != -1) {
            return (slot_t) {
                .bytes_needed = needed,
                .is_register = true,
                .reg = reg,
                .stack = 0
            };
        }
    }

    return (slot_t) {
        .bytes_needed = needed,
        .is_register = false,
        .reg = -1,
        .stack = checkout_stack(needed)
    };
}

bool run_x86_64_backend(compilation_unit_t* unit) {
    (void)usable_registers;

    NULLCHECK(unit);

    g_unit = unit;

    if (unit->out_file && unit->out_file[0] != '\0') {
        backend_out = fopen(unit->out_file, "w");
        if (!backend_out) {
            perror("fopen");
            return false;
        }
    } else {
        backend_out = stdout;
    }

    fprintf(backend_out, "// ACC %s\n\n", ACC_VERSION);
    fprintf(backend_out, ".file \"%s\"\n", unit->in_file);
    fprintf(backend_out, ".ofile \"%s\"\n", unit->out_file);
    fprintf(backend_out, ".text\n\n");

    for (size_t i = 0; i < unit->parser->function_count; i++) {
        function_t func = unit->parser->functions[i];
        emit_x86_64_function(&func);
    }

    if (backend_out && backend_out != stdout) {
        fclose(backend_out);
    }
    backend_out = NULL;

    return true;
}

void emit_x86_64_return(statement_t* stmt) {
    if (!stmt) return;
    if (stmt->return_state.type == TOK_LITERAL) {
        fprintf(backend_out, "  mov %s, %lld\n", quadword_registers[return_register], stmt->return_state.int_type);
    }
    fprintf(backend_out, "  ret\n");
}


void emit_x86_64_function(function_t* func) {
    if (!func) return;
    fprintf(backend_out, "_%s:\n", func->name);
    for (size_t i = 0; i < func->body.statement_count; i++) {
        statement_t stmt = func->body.statments[i];
        if (stmt.type == STATE_RETURN) {
            emit_x86_64_return(&stmt);
        }
    }
}
