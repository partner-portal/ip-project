#ifndef DUMPER_BUILD_DUMPER_H_INCLUDED
#define DUMPER_BUILD_DUMPER_H_INCLUDED

#include <stddef.h>

static const char *dumper_build_dumper_metadata_keys[] = {
    "gpd.ta.dataSize",
    "pnr.driver.physbuff.nrpages",
    "pnr.driver.physbuff.cached",
};

static const char *dumper_build_dumper_metadata_values[] = {
    "0",
    "0",
    "1",
};

#define DUMPER_BUILD_DUMPER_METADATA_NR ((size_t)3)

#endif /* DUMPER_BUILD_DUMPER_H_INCLUDED */
