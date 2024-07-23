#ifndef PUBLIC_LWIPOPTS_H
#define PUBLIC_LWIPOPTS_H

#define PNC_MONOTHREAD 1
//Weird

#define LWIP_IPV6_MLD              0
#define LWIP_IGMP                  0//LWIP_IPV4
#define LWIP_MULTICAST_TX_OPTIONS  0
#if defined(CONFIG_LWIP_IPV4) && defined(CONFIG_LWIP_IPV6)
#define LWIP_IPV4                  1
#define LWIP_IPV6                  1
#elif defined(CONFIG_LWIP_IPV4)
#define LWIP_IPV4                  1
#define LWIP_IPV6                  0
#elif defined(CONFIG_LWIP_IPV6)
#define LWIP_IPV4                  0
#define LWIP_IPV6                  1
#else
#define LWIP_IPV4                  1
#define LWIP_IPV6                  1
#endif
#define LWIP_IPV6_SCOPES           1
/* ---------- TCP options ---------- */
#define LWIP_TCP                1
#define TCP_TTL                 255

#define LWIP_SOCKET_SELECT         1
#define LWIP_ALTCP              0//(LWIP_TCP)
#ifdef LWIP_HAVE_MBEDTLS
#define LWIP_ALTCP_TLS          (LWIP_TCP)
#define LWIP_ALTCP_TLS_MBEDTLS  (LWIP_TCP)
#endif

/* ---------- UDP options ---------- */
#define LWIP_UDP                1
#define LWIP_UDPLITE            LWIP_UDP
#define UDP_TTL                 255

#define LWIP_COMPAT_SOCKETS 1

/* ---------- RAW options ---------- */
#define LWIP_RAW                1

//TODO: normally defined in opt.h if not defined and set to 0
#define LWIP_SOCKET_OFFSET 0
#define NO_SYS                     0
#define LWIP_SOCKET                (NO_SYS==0)
#define LWIP_NETCONN               (NO_SYS==0)
#define LWIP_NETIF_API             (NO_SYS==0)
#endif /* PUBLIC LWIPOPTS */
