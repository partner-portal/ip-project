#ifndef PROXY_RS_BUILD_PROXY_H_INCLUDED
#define PROXY_RS_BUILD_PROXY_H_INCLUDED

#include <stddef.h>

static const char *proxy_rs_build_proxy_metadata_keys[] = {
    "gpd.ta.dataSize",
    "pnr.driver.physbuff.nrpages",
    "pnr.driver.physbuff.cached",
};

static const char *proxy_rs_build_proxy_metadata_values[] = {
    "0",
    "0",
    "0",
};

#define PROXY_RS_BUILD_PROXY_METADATA_NR ((size_t)3)

#endif /* PROXY_RS_BUILD_PROXY_H_INCLUDED */
