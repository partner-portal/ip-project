#ifndef HELLO_BUILD_HELLO_H_INCLUDED
#define HELLO_BUILD_HELLO_H_INCLUDED

#include <stddef.h>

static const char *hello_build_hello_metadata_keys[] = {
    "gpd.ta.dataSize",
};

static const char *hello_build_hello_metadata_values[] = {
    "5120",
};

#define HELLO_BUILD_HELLO_METADATA_NR ((size_t)1)

#endif /* HELLO_BUILD_HELLO_H_INCLUDED */
