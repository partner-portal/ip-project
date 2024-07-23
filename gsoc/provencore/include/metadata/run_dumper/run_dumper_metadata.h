#ifndef RUN_DUMPER_BUILD_RUN_DUMPER_H_INCLUDED
#define RUN_DUMPER_BUILD_RUN_DUMPER_H_INCLUDED

#include <stddef.h>

static const char *run_dumper_build_run_dumper_metadata_keys[] = {
    "gpd.ta.dataSize",
    "pnr.driver.physbuff.nrpages",
    "pnr.driver.physbuff.cached",
};

static const char *run_dumper_build_run_dumper_metadata_values[] = {
    "0",
    "0",
    "1",
};

#define RUN_DUMPER_BUILD_RUN_DUMPER_METADATA_NR ((size_t)3)

#endif /* RUN_DUMPER_BUILD_RUN_DUMPER_H_INCLUDED */
