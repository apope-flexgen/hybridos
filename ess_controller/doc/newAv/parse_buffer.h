#ifndef __PARSE_BUFFER_H
#define __PARSE_BUFFER_H

#include <stdlib.h>
typedef struct
{
    const unsigned char *content;
    size_t length;
    size_t offset;
    size_t depth; /* How deeply nested (in arrays/objects) is the input at the current offset. */
    //internal_hooks hooks;
} parse_buffer;
#endif