#ifndef SYSTEM_LOGGER_BUILD_LOGGER_H_INCLUDED
#define SYSTEM_LOGGER_BUILD_LOGGER_H_INCLUDED

#include <stddef.h>

static const char *system_logger_build_logger_metadata_keys[] = {
    "gpd.ta.dataSize",
};

static const char *system_logger_build_logger_metadata_values[] = {
    "0",
};

#define SYSTEM_LOGGER_BUILD_LOGGER_METADATA_NR ((size_t)1)

#endif /* SYSTEM_LOGGER_BUILD_LOGGER_H_INCLUDED */
