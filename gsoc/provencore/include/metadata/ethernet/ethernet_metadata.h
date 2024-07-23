#ifndef ETHERNET_BUILD_ETHERNET_H_INCLUDED
#define ETHERNET_BUILD_ETHERNET_H_INCLUDED

#include <stddef.h>

static const char *ethernet_build_ethernet_metadata_keys[] = {
    "gpd.ta.dataSize",
    "pnr.driver.physbuff.nrpages",
    "pnr.driver.physbuff.cached",
};

static const char *ethernet_build_ethernet_metadata_values[] = {
    "0",
    "0",
    "0",
};

#define ETHERNET_BUILD_ETHERNET_METADATA_NR ((size_t)3)

#endif /* ETHERNET_BUILD_ETHERNET_H_INCLUDED */
