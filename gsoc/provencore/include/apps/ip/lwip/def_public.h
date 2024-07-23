#ifndef LWIP_HDR_DEF_PUBLIC_H
#define LWIP_HDR_DEF_PUBLIC_H

#include <stdint.h>

#ifndef lwip_htons
uint16_t lwip_htons(uint16_t x);
#endif
#define lwip_ntohs(x) lwip_htons(x)

#ifndef lwip_htonl
uint32_t lwip_htonl(uint32_t x);
#endif
#define lwip_ntohl(x) lwip_htonl(x)

/* These macros should be calculated by the preprocessor and are used
   with compile-time constants only (so that there is no little-endian
   overhead at runtime). */
#define PP_HTONS(x) ((uint16_t)((((x) & (uint16_t)0x00ffU) << 8) | (((x) & (uint16_t)0xff00U) >> 8)))
#define PP_NTOHS(x) PP_HTONS(x)
#define PP_HTONL(x) ((((x) & (uint32_t)0x000000ffUL) << 24) | \
                     (((x) & (uint32_t)0x0000ff00UL) <<  8) | \
                     (((x) & (uint32_t)0x00ff0000UL) >>  8) | \
                     (((x) & (uint32_t)0xff000000UL) >> 24))
#define PP_NTOHL(x) PP_HTONL(x)

/* Provide usual function names as macros for users, but this can be turned off */
#ifndef LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS
#define htons(x) PP_HTONS(x)
#define ntohs(x) PP_NTOHS(x)
#define htonl(x) PP_HTONL(x)
#define ntohl(x) PP_NTOHL(x)
#endif

#endif /* LWIP_HDR_DEF_PUBLIC_H */
