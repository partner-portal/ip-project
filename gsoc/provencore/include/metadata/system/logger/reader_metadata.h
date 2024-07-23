#ifndef SYSTEM_LOGGER_BUILD_READER_H_INCLUDED
#define SYSTEM_LOGGER_BUILD_READER_H_INCLUDED

#include <stddef.h>

static const char *system_logger_build_reader_metadata_keys[] = {
    "gpd.ta.dataSize",
};

static const char *system_logger_build_reader_metadata_values[] = {
    "5120",
};

#define SYSTEM_LOGGER_BUILD_READER_METADATA_NR ((size_t)1)

#endif /* SYSTEM_LOGGER_BUILD_READER_H_INCLUDED */
