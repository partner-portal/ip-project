#ifndef TOUPPER_BUILD_TOUPPER_H_INCLUDED
#define TOUPPER_BUILD_TOUPPER_H_INCLUDED

#include <stddef.h>

static const char *toupper_build_toupper_metadata_keys[] = {
    "gpd.ta.dataSize",
    "pnr.driver.physbuff.nrpages",
    "pnr.driver.physbuff.cached",
};

static const char *toupper_build_toupper_metadata_values[] = {
    "0",
    "0",
    "0",
};

#define TOUPPER_BUILD_TOUPPER_METADATA_NR ((size_t)3)

#endif /* TOUPPER_BUILD_TOUPPER_H_INCLUDED */
