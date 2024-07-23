#ifndef IP_BUILD_IP_H_INCLUDED
#define IP_BUILD_IP_H_INCLUDED

#include <stddef.h>

static const char *ip_build_ip_metadata_keys[] = {
    "gpd.ta.dataSize",
    "pnr.driver.physbuff.nrpages",
    "pnr.driver.physbuff.cached",
};

static const char *ip_build_ip_metadata_values[] = {
    "5120",
    "0",
    "1",
};

#define IP_BUILD_IP_METADATA_NR ((size_t)3)

#endif /* IP_BUILD_IP_H_INCLUDED */
