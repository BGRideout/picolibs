#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// see https://www.nongnu.org/lwip/2_1_x/group__lwip__opts.html for details

#define NO_SYS                      1
#define LWIP_SOCKET                 0
#define MEM_LIBC_MALLOC             0
#define MEM_ALIGNMENT               4
#define MEMP_NUM_TCP_SEG            64
#define MEMP_NUM_ARP_QUEUE          10
#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1
#define LWIP_RAW                    1
#define TCP_MSS                     1460
#ifdef USE_HTTPS
#if PICO_RP2350
#define MEM_SIZE                    32768
#define TCP_WND                     16384
#define	MEMP_NUM_PBUF               64
#define PBUF_POOL_SIZE              64
#define PBUF_POOL_BUFSIZE           (1500 + PBUF_LINK_ENCAPSULATION_HLEN + PBUF_LINK_HLEN + PBUF_IP_HLEN + PBUF_TRANSPORT_HLEN)
#else
#define MEM_SIZE                    16384
#define TCP_WND                     16384
#define PBUF_POOL_SIZE              32
#endif
#else
#define MEM_SIZE                    8192
#define TCP_WND                     (8 * TCP_MSS)
#define PBUF_POOL_SIZE              32
#endif
#define TCP_SND_BUF                 (8 * TCP_MSS)
#define TCP_SND_QUEUELEN            ((8 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))
#define LWIP_NETIF_STATUS_CALLBACK  1
#define LWIP_NETIF_EXT_STATUS_CALLBACK 1
#define LWIP_NETIF_LINK_CALLBACK    1
#define LWIP_NETIF_HOSTNAME         1
#define LWIP_NETCONN                0
#define MEM_STATS                   0
#define SYS_STATS                   0
#define MEMP_STATS                  0
#define LINK_STATS                  0
#define LWIP_CHKSUM_ALGORITHM       3
#define LWIP_DHCP                   1
#define LWIP_IPV4                   1
#define LWIP_TCP                    1
#define LWIP_UDP                    1
#define LWIP_DNS                    1
#define LWIP_TCP_KEEPALIVE          1
#define LWIP_NETIF_TX_SINGLE_PBUF   1
#define DHCP_DOES_ARP_CHECK         0
#define LWIP_DHCP_DOES_ACD_CHECK    0

#ifndef NDEBUG
#define LWIP_DEBUG                  1
#define LWIP_STATS                  1
#define LWIP_STATS_DISPLAY          1
#endif

#define ETHARP_DEBUG                LWIP_DBG_OFF
#define NETIF_DEBUG                 LWIP_DBG_OFF
#define PBUF_DEBUG                  LWIP_DBG_OFF
#define API_LIB_DEBUG               LWIP_DBG_OFF
#define API_MSG_DEBUG               LWIP_DBG_OFF
#define SOCKETS_DEBUG               LWIP_DBG_OFF
#define ICMP_DEBUG                  LWIP_DBG_OFF
#define INET_DEBUG                  LWIP_DBG_OFF
#define IP_DEBUG                    LWIP_DBG_OFF
#define IP_REASS_DEBUG              LWIP_DBG_OFF
#define RAW_DEBUG                   LWIP_DBG_OFF
#define MEM_DEBUG                   LWIP_DBG_ON
#define MEMP_DEBUG                  LWIP_DBG_ON
#define SYS_DEBUG                   LWIP_DBG_OFF
#define TCP_DEBUG                   LWIP_DBG_OFF
#define TCP_INPUT_DEBUG             LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG            LWIP_DBG_OFF
#define TCP_RTO_DEBUG               LWIP_DBG_OFF
#define TCP_CWND_DEBUG              LWIP_DBG_OFF
#define TCP_WND_DEBUG               LWIP_DBG_OFF
#define TCP_FR_DEBUG                LWIP_DBG_OFF
#define TCP_QLEN_DEBUG              LWIP_DBG_OFF
#define TCP_RST_DEBUG               LWIP_DBG_OFF
#define UDP_DEBUG                   LWIP_DBG_OFF
#define TCPIP_DEBUG                 LWIP_DBG_OFF
#define PPP_DEBUG                   LWIP_DBG_OFF
#define SLIP_DEBUG                  LWIP_DBG_OFF
#define DHCP_DEBUG                  LWIP_DBG_OFF

//  Additions for MDNS
#define LWIP_MDNS_RESPONDER         1
#define LWIP_IGMP                   1
#define LWIP_NUM_NETIF_CLIENT_DATA  1
#define MEMP_NUM_UDP_PCB            5
#define MDNS_DEBUG                  LWIP_DBG_OFF
#define MDNS_RESP_USENETIF_EXTCALLBACK 1

#define MEMP_NUM_SYS_TIMEOUT   (LWIP_NUM_SYS_TIMEOUT_INTERNAL + 16)

//  For TLS
#define LWIP_ALTCP                  1
#ifdef USE_HTTPS
#define LWIP_ALTCP_TLS              1
#define LWIP_ALTCP_TLS_MBEDTLS      1
#if PICO_RP2350
#define MEMP_NUM_TCP_PCB            24
#define MEMP_NUM_RAW_PCB            8
#else
#define MEMP_NUM_TCP_PCB            8
#endif
#define ALTCP_MBEDTLS_DEBUG         LWIP_DBG_ON
#define ALTCP_MBEDTLS_LIB_DEBUG     LWIP_DBG_ON
#endif

//  For SNTP
#define SNTP_SET_SYSTEM_TIME(sec)	sntp_set_system_time(sec)
#define SNTP_SERVER_DNS             1
#define SNTP_DEBUG                  LWIP_DBG_OFF

//  For close/open of listen ports
#define SO_REUSE                    1

#ifdef APP_ERR_LOGGER
#ifdef __cplusplus
extern "C"
{
#endif
    void APP_ERR_LOGGER(const char *fmt, ...);
#ifdef __cplusplus
}
#endif
#define LWIP_PLATFORM_DIAG(x)       do {APP_ERR_LOGGER x;} while(0)
#define LWIP_PLATFORM_ASSERT(x)     do {APP_ERR_LOGGER("Assertion \"%s\" failed at line %d in %s\n", \
                                     x, __LINE__, __FILE__); fflush(NULL); abort();} while(0)
#endif

#endif /* __LWIPOPTS_H__ */
