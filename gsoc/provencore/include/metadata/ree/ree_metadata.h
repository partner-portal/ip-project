#ifndef REE_BUILD_REE_H_INCLUDED
#define REE_BUILD_REE_H_INCLUDED

#include <stddef.h>

static const char *ree_build_ree_metadata_keys[] = {
    "gpd.ta.dataSize",
    "pnr.driver.physbuff.nrpages",
    "pnr.driver.physbuff.cached",
};

static const char *ree_build_ree_metadata_values[] = {
    "0",
    "0",
    "1",
};

#define REE_BUILD_REE_METADATA_NR ((size_t)3)

#endif /* REE_BUILD_REE_H_INCLUDED */
