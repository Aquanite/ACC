#ifndef LIBSTELLAR_CHECKS_H
#define LIBSTELLAR_CHECKS_H

#include <libstellar/base.h>

#define STEL_ERROR(str)                                                                                 \
    do                                                                                                  \
    {                                                                                                   \
        fprintf(stderr, "%s Fatal Error | \"%s\" | %s:%d\n", STEL_PROGRAM, str, __FILE__, __LINE__); \
        exit(-1);                                                                                       \
    } while (0)

#define ISNULL(x) ((x) == NULL)

#define NULLCHECK(x)                   \
    do                                 \
    {                                  \
        if (ISNULL(x))                 \
            STEL_ERROR(#x " is NULL"); \
    } while (0)

#define ZEROCHECK(x)                   \
    do                                 \
    {                                  \
        if ((x) == 0)                  \
            STEL_ERROR(#x " is ZERO"); \
    } while (0)

#define COUNTDIGITS(n) (n == 0 ? 1 : (int)floor(log10(abs(n))) + 1)
#define COUNTDIGITSU(n) (n == 0 ? 1 : (int)floor(log10(n)) + 1)

#define ASSERT(c, fmt, ...)                    \
    do                                         \
    {                                          \
        if (!(c))                              \
        {                                      \
            fprintf(stderr, fmt, __VA_ARGS__); \
            exit(-1);                          \
        }                                      \
    } while (0)

#endif // LIBSTELLAR_CHECKS_H
