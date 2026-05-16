#ifndef LIBSTELLAR_STRINGS_H
#define LIBSTELLAR_STRINGS_H

#include <libstellar/base.h>

#ifndef STEL_NO_STRING
#include <string.h>
#define _STELVS(x) #x
#define STRINGIFY(x) _STELVS(x)
#define STRCMP(x, y) (strcmp((x), (y)) == 0)
#define STRNCMP(x, y, z) (strncmp((x), (y), (z)) == 0)
typedef char *string;
#endif // STEL_NO_STRING

#endif // LIBSTELLAR_STRINGS_H
