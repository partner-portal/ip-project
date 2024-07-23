#ifndef LWIP_IP6_ADDR_PUBLIC_H
#define LWIP_IP6_ADDR_PUBLIC_H
#include <stdint.h>
#include "lwipopts_public.h"
/** This is the aligned version of ip6_addr_t,
    used as local variable, on the stack, etc. */
struct ip6_addr {
  uint32_t addr[4];
#if LWIP_IPV6_SCOPES
  uint8_t zone;
#endif /* LWIP_IPV6_SCOPES */
};

/** IPv6 address */
typedef struct ip6_addr ip6_addr_t;
#endif /* LWIP_IP6_ADDR_PUBLIC_H */
