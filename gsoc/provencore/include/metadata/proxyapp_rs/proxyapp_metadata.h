#ifndef PROXYAPP_RS_BUILD_PROXYAPP_H_INCLUDED
#define PROXYAPP_RS_BUILD_PROXYAPP_H_INCLUDED

#include <stddef.h>

static const char *proxyapp_rs_build_proxyapp_metadata_keys[] = {
    "gpd.ta.dataSize",
    "pnr.driver.physbuff.nrpages",
    "pnr.driver.physbuff.cached",
};

static const char *proxyapp_rs_build_proxyapp_metadata_values[] = {
    "0",
    "0",
    "0",
};

#define PROXYAPP_RS_BUILD_PROXYAPP_METADATA_NR ((size_t)3)

#endif /* PROXYAPP_RS_BUILD_PROXYAPP_H_INCLUDED */
