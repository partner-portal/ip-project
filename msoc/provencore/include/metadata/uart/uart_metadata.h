#ifndef UART_BUILD_UART_H_INCLUDED
#define UART_BUILD_UART_H_INCLUDED

#include <stddef.h>

static const char *uart_build_uart_metadata_keys[] = {
    "gpd.ta.dataSize",
    "pnr.driver.physbuff.nrpages",
    "pnr.driver.physbuff.cached",
};

static const char *uart_build_uart_metadata_values[] = {
    "0",
    "0",
    "1",
};

#define UART_BUILD_UART_METADATA_NR ((size_t)3)

#endif /* UART_BUILD_UART_H_INCLUDED */
