// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2016-2022, ProvenRun S.A.S
 */
/*
 * File: proxyclient.c
 * 2022-07-20: T.Ta   : Created for using pnc_session_xxxx() API.
 */

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <provencore.h>

#define DEBUG 0
#define PNC_SESSION_BUF_SIZE 1024000
#define ID_STATE_SIZE 8
#define TABLE_LEN_SIZE 8
#define TIMEOUT 2000
#define TIMEOUT_STAT 20000
#define REJ_PKT_CAPACITY 1
#define REJ_PKT_SIZE 128

#if defined(DEBUG) && DEBUG > 0
 #define DEBUG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##args)
#else
 #define DEBUG_PRINT(fmt, args...) /* Don't do anything in release builds */
#endif

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

typedef struct statFrames {
    uint32_t acc_from_eth;
    uint32_t acc_from_other;
    uint32_t sent_to_eth;
    uint32_t sent_to_other;
} statFrame;

typedef struct statRejs {
    uint32_t from_eth;
    char pkt_from_eth[REJ_PKT_CAPACITY][REJ_PKT_SIZE];
    uint32_t from_other;
    char pkt_from_other[REJ_PKT_CAPACITY][REJ_PKT_SIZE];
} statRej;

typedef struct statDPIs {
    statFrame udp;
    statFrame someip;
    statFrame tcp;
    statRej rej_udp;
    statRej rej_tcp;
    statRej rej_unknown;
} statDPI;

typedef struct sockets {
    uint8_t ip[4];
    uint16_t port;
    uint32_t uid;
} socket;

typedef struct statAPPs {
    socket auth_listen[64];
    socket abandon_listen[64];
    socket est_accept[64];
    socket est_connect[64];
    socket close_accept[64];
    socket close_connect[64];
    int auth_listen_ctr;
    int abandon_listen_ctr;
    int est_accept_ctr;
    int est_connect_ctr;
    int close_accept_ctr;
    int close_connect_ctr;
} statAPP;

void print_error(int error)
{
    int sys_err = errno;
    fprintf(stderr, "Failed: error=%d, errno=%d (%s)\n",
           error, sys_err, strerror(sys_err));
}

void print_help(char **argv) {
    printf("%s : RobustProxies commands interface\n", argv[0]);
    printf("Load filters:\n");
    printf("    %s push-table --data <path> [--dpi|--app] \n", argv[0]);
    printf("Get statistics:\n");
    printf("    %s stat [--dpi|--app]\n", argv[0]);
    printf("Help:\n");
    printf("    %s help\n", argv[0]);
}

void print_hint(char **argv) {
    printf("(Use '%s help' for the full list of commands)\n", argv[0]);
}

int open_session(pnc_session_t **session, void **pnc_buf, size_t *pnc_size, const char* app_name)
{
    int error;
    uint8_t zone = 0;
    if (!strcmp("proxy_zone_1", app_name) || !strcmp("proxyapp_zone_1", app_name)) {
        zone = 1;
    } else {
        zone = 2;
    }
    // request 128kB shared buffer 
    DEBUG_PRINT("Z%d App name=%s Creating session, Request %d shared buffer\n", zone, app_name, PNC_SESSION_BUF_SIZE);
    error = pnc_session_new( PNC_SESSION_BUF_SIZE, session);
    if (error) { return error; }

    DEBUG_PRINT("Z%d App name=%s Configuring session\n", zone, app_name);
    error = pnc_session_config_by_name(*session, app_name);
    if (error) { return error; }

    // Get shared buffer parameters - its address and the size.
    error = pnc_session_getinfo(*session, pnc_buf, pnc_size);
    if (error) { return error; }
    DEBUG_PRINT("Z%d App name=%s PNC shared buffer size : %ld \n", zone, app_name, *pnc_size);
    return 0;
}

int push_table(char * data, size_t data_len, const char* app_name) 
{
    int error;
    pnc_session_t *session = NULL;
    char *pnc_buf = NULL;
    size_t pnc_size = 0;
    size_t i;
    size_t ctr = 0;
    uint32_t response = 0;
    uint8_t zone = 0;
    if (!strcmp("proxy_zone_1", app_name) || !strcmp("proxyapp_zone_1", app_name)) {
        zone = 1;
    } else {
        zone = 2;
    }

    error = open_session(&session, (void **)&pnc_buf, &pnc_size, app_name);
    if (error) {
        print_error(error);
        return EXIT_FAILURE;
    }

    // Copy the input string to the shared area
    for (i = 0; i < data_len; i++)
        pnc_buf[ctr++] = data[i];
    DEBUG_PRINT("Z%d App name=%s Write %ld bytes in shared buffer\n", zone, app_name, ctr);

    error = pnc_session_send_request_and_wait_response(session, 42, TIMEOUT, &response);
    if (error) {
        print_error(error);
        return EXIT_FAILURE;
    }
    fprintf(stderr, "ZONE%d: return=%d (%s)\n", zone, response, strerror(response));

    pnc_session_destroy(session);

    return EXIT_SUCCESS;
}

int get_stat_app(const char* app_name, statAPP *stat_app)
{
    int error;
    pnc_session_t * session = NULL;
    char *pnc_buf = NULL;
    size_t pnc_size = 0;
    size_t i, j;
    uint32_t response = 0;

    uint8_t zone_nb = 0;
    if (!strcmp("proxyapp_zone_1", app_name)) {
        zone_nb = 1;
    } else if (!strcmp("proxyapp_zone_2", app_name)) {
        zone_nb = 2;
    } else {
        fprintf(stderr, "ZONE%d: unknown ProxyApp App name=%s\n", zone_nb, app_name);
        return EXIT_FAILURE;
    }

    error = open_session(&session, (void **)&pnc_buf, &pnc_size, app_name);
    if (error) {
        print_error(error);
        return EXIT_FAILURE;
    }

    // Copy the input string to the shared area
    char type[] = "STAT";
    for (i = 0; i < 4; i++) {
        pnc_buf[i] = type[i];
    }
    
    memset(&pnc_buf[12], 0, 4 * sizeof(uint8_t));

    DEBUG_PRINT("ZONE%d: Shared buffer %s\n", zone_nb, pnc_buf);

    error = pnc_session_send_request_and_wait_response(session, 42, TIMEOUT_STAT, &response);
    if (error) {
        print_error(error);
        return EXIT_FAILURE;
    }
    DEBUG_PRINT("ZONE%d: return=%d (%s) ", zone_nb, response, strerror(response));

    if (response == 0) {
        size_t ctr = 12;
        size_t stat_size = (pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++];
        DEBUG_PRINT("STAT size: %ld bytes\n", stat_size);
        while (stat_size > 0) {
            /* HEAD */
            char head[8];
            for(int i = 0; i < 8; i++) {
                head[i] = pnc_buf[ctr];
                ctr++;
            }
            
            if (strncmp(head, "AUTHLSTN", 8) == 0) {

                /* LEN */
                stat_app->auth_listen_ctr = (pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++];
                /* BODY */
                for(int i = 0; i < stat_app->auth_listen_ctr; i++) {
                    stat_app->auth_listen[i].ip[0] = pnc_buf[ctr++];
                    stat_app->auth_listen[i].ip[1] = pnc_buf[ctr++];
                    stat_app->auth_listen[i].ip[2] = pnc_buf[ctr++];
                    stat_app->auth_listen[i].ip[3] = pnc_buf[ctr++];
                    stat_app->auth_listen[i].port = (pnc_buf[ctr++] << 8) | pnc_buf[ctr++];
                    fprintf(stderr, "Z%d %d.%d.%d.%d:%d listen active\n",
                        zone_nb,
                        stat_app->auth_listen[i].ip[0], 
                        stat_app->auth_listen[i].ip[1],
                        stat_app->auth_listen[i].ip[2],
                        stat_app->auth_listen[i].ip[3],
                        stat_app->auth_listen[i].port);
                }
                stat_size -= 12 + stat_app->auth_listen_ctr*6;
            } else if (strncmp(head, "ABDNLSTN", 8) == 0) {
                /* LEN */
                stat_app->abandon_listen_ctr = (pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++];
                /* BODY */
                for(int i = 0; i < stat_app->abandon_listen_ctr; i++) {
                    stat_app->abandon_listen[i].ip[0] = pnc_buf[ctr++];
                    stat_app->abandon_listen[i].ip[1] = pnc_buf[ctr++];
                    stat_app->abandon_listen[i].ip[2] = pnc_buf[ctr++];
                    stat_app->abandon_listen[i].ip[3] = pnc_buf[ctr++];
                    stat_app->abandon_listen[i].port = (pnc_buf[ctr++] << 8) | pnc_buf[ctr++];
                    fprintf(stderr, "Z%d %d.%d.%d.%d:%d listen closed\n",
                        zone_nb,
                        stat_app->abandon_listen[i].ip[0], 
                        stat_app->abandon_listen[i].ip[1],
                        stat_app->abandon_listen[i].ip[2],
                        stat_app->abandon_listen[i].ip[3],
                        stat_app->abandon_listen[i].port);
                }
                stat_size -= 12 + stat_app->abandon_listen_ctr*6;
            } else if (strncmp(head, "ESTDACPT", 8) == 0) {
                /* LEN */
                stat_app->est_accept_ctr = (pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++];
                /* BODY */
                for(int i = 0; i < stat_app->est_accept_ctr; i++) {
                    stat_app->est_accept[i].uid = (pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++];
                    stat_app->est_accept[i].ip[0] = pnc_buf[ctr++];
                    stat_app->est_accept[i].ip[1] = pnc_buf[ctr++];
                    stat_app->est_accept[i].ip[2] = pnc_buf[ctr++];
                    stat_app->est_accept[i].ip[3] = pnc_buf[ctr++];
                    stat_app->est_accept[i].port = (pnc_buf[ctr++] << 8) | pnc_buf[ctr++];
                    fprintf(stderr, "Z%d %d.%d.%d.%d:%d UID%d accept active\n",
                        zone_nb,
                        stat_app->est_accept[i].ip[0], 
                        stat_app->est_accept[i].ip[1],
                        stat_app->est_accept[i].ip[2],
                        stat_app->est_accept[i].ip[3],
                        stat_app->est_accept[i].port,
                        stat_app->est_accept[i].uid);
                }
                stat_size -= 12 + stat_app->est_accept_ctr*10;
            } else if (strncmp(head, "ESTDCONN", 8) == 0) {
                /* LEN */
                stat_app->est_connect_ctr = (pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++];
                /* BODY */
                for(int i = 0; i < stat_app->est_connect_ctr; i++) {
                    stat_app->est_connect[i].uid = (pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++];
                    stat_app->est_connect[i].ip[0] = pnc_buf[ctr++];
                    stat_app->est_connect[i].ip[1] = pnc_buf[ctr++];
                    stat_app->est_connect[i].ip[2] = pnc_buf[ctr++];
                    stat_app->est_connect[i].ip[3] = pnc_buf[ctr++];
                    stat_app->est_connect[i].port = (pnc_buf[ctr++] << 8) | pnc_buf[ctr++];
                    fprintf(stderr, "Z%d %d.%d.%d.%d:%d UID%d connect active\n",
                        zone_nb, 
                        stat_app->est_connect[i].ip[0], 
                        stat_app->est_connect[i].ip[1],
                        stat_app->est_connect[i].ip[2],
                        stat_app->est_connect[i].ip[3],
                        stat_app->est_connect[i].port,
                        stat_app->est_connect[i].uid);
                }
                stat_size -= 12 + stat_app->est_connect_ctr*10;
            } else if (strncmp(head, "CLSDACPT", 8) == 0) {
                /* LEN */
                stat_app->close_accept_ctr = (pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++];
                /* BODY */
                for(int i = 0; i < stat_app->close_accept_ctr; i++) {
                    stat_app->close_accept[i].uid = (pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++];
                    stat_app->close_accept[i].ip[0] = pnc_buf[ctr++];
                    stat_app->close_accept[i].ip[1] = pnc_buf[ctr++];
                    stat_app->close_accept[i].ip[2] = pnc_buf[ctr++];
                    stat_app->close_accept[i].ip[3] = pnc_buf[ctr++];
                    stat_app->close_accept[i].port = (pnc_buf[ctr++] << 8) | pnc_buf[ctr++];
                    fprintf(stderr, "Z%d %d.%d.%d.%d:%d UID%d accept closed\n",
                        zone_nb,
                        stat_app->close_accept[i].ip[0], 
                        stat_app->close_accept[i].ip[1],
                        stat_app->close_accept[i].ip[2],
                        stat_app->close_accept[i].ip[3],
                        stat_app->close_accept[i].port,
                        stat_app->close_accept[i].uid);
                }
                stat_size -= 12 + stat_app->close_accept_ctr*10;
            } else if (strncmp(head, "CLSDCONN", 8) == 0) {
                /* LEN */
                stat_app->close_connect_ctr = (pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++];
                /* BODY */
                for(int i = 0; i < stat_app->close_connect_ctr; i++) {
                    stat_app->close_connect[i].uid = (pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++];
                    stat_app->close_connect[i].ip[0] = pnc_buf[ctr++];
                    stat_app->close_connect[i].ip[1] = pnc_buf[ctr++];
                    stat_app->close_connect[i].ip[2] = pnc_buf[ctr++];
                    stat_app->close_connect[i].ip[3] = pnc_buf[ctr++];
                    stat_app->close_connect[i].port = (pnc_buf[ctr++] << 8) | pnc_buf[ctr++];
                    fprintf(stderr, "Z%d %d.%d.%d.%d:%d UID%d connect closed\n",
                        zone_nb,
                        stat_app->close_connect[i].ip[0], 
                        stat_app->close_connect[i].ip[1],
                        stat_app->close_connect[i].ip[2],
                        stat_app->close_connect[i].ip[3],
                        stat_app->close_connect[i].port,
                        stat_app->close_connect[i].uid);
                }
                stat_size -= 12 + stat_app->close_connect_ctr*10;
            } else {
                fprintf(stderr, "unknown HEAD %s\n", head);
                break;
            }     
        }
    }
    pnc_session_destroy(session);
    return EXIT_SUCCESS;
}

int get_stat_dpi(const char* app_name, statDPI *stat_dpi) 
{
    int error;
    pnc_session_t * session = NULL;
    uint8_t*pnc_buf = NULL;
    size_t pnc_size = 0;
    size_t i, j;
    uint32_t response = 0;

    //TODO FIXE APP NAME ?
    uint8_t zone_nb = 0;
    if (!strcmp("proxy_zone_1", app_name)) {
        zone_nb = 1;
    } else if (!strcmp("proxy_zone_2", app_name)) {
        zone_nb = 2;
    } else {
        fprintf(stderr, "ZONE%d: unknown Proxy name=%s\n", zone_nb, app_name);
        return EXIT_FAILURE;
    }

    error = open_session(&session, (void **)&pnc_buf, &pnc_size, app_name);
    if (error) {
        print_error(error);
        return EXIT_FAILURE;
    }

    // Copy the input string to the shared area
    char type[] = "STAT";
    for (i = 0; i < 5; i++) {
        pnc_buf[i] = type[i];
    }

    // Set input length filter to 0
    memset(&pnc_buf[12], 0, 4 * sizeof(uint8_t));

    DEBUG_PRINT("ZONE%d: Shared buffer %s\n", zone_nb, pnc_buf);

    error = pnc_session_send_request_and_wait_response(session, 42, TIMEOUT_STAT, &response);
    if (error) {
        print_error(error);
        return EXIT_FAILURE;
    }
    

    if (response == 0) {
        size_t ctr = 12;
        size_t stat_size = (pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++];
        stat_dpi->udp.acc_from_eth = (uint32_t) ((pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++]);
        stat_dpi->udp.acc_from_other = (uint32_t) ((pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++]);
        stat_dpi->udp.sent_to_eth = (uint32_t) ((pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++]);
        stat_dpi->udp.sent_to_other = (uint32_t) ((pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++]);
        stat_dpi->someip.acc_from_eth = (uint32_t) ((pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++]);
        stat_dpi->someip.acc_from_other = (uint32_t) ((pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++]);
        stat_dpi->someip.sent_to_eth = (uint32_t) ((pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++]);
        stat_dpi->someip.sent_to_other = (uint32_t) ((pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++]);
        stat_dpi->tcp.acc_from_eth = (uint32_t) ((pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++]);
        stat_dpi->tcp.acc_from_other = (uint32_t) ((pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++]);
        stat_dpi->tcp.sent_to_eth = (uint32_t) ((pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++]);
        stat_dpi->tcp.sent_to_other = (uint32_t) ((pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++]);
        stat_dpi->rej_udp.from_eth = (uint32_t) ((pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++]);
        stat_dpi->rej_udp.from_other = (uint32_t) ((pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++]);
        stat_dpi->rej_tcp.from_eth = (uint32_t) ((pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++]);
        stat_dpi->rej_tcp.from_other = (uint32_t) ((pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++]);
        stat_dpi->rej_unknown.from_eth = (uint32_t) ((pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++]);
        stat_dpi->rej_unknown.from_other = (uint32_t) ((pnc_buf[ctr++] << 24) | (pnc_buf[ctr++] << 16) | (pnc_buf[ctr++] << 8) | pnc_buf[ctr++]);
        for (i = 0; i < stat_dpi->rej_udp.from_eth; i++) {
            if (i >= REJ_PKT_CAPACITY) { break; }
            for (j = 0; j < 128; j++) {
                stat_dpi->rej_udp.pkt_from_eth[i][j] = pnc_buf[ctr++];
            }
        }
        for (i = 0; i < stat_dpi->rej_udp.from_other; i++) {
            if (i >= REJ_PKT_CAPACITY) { break; }
            for (j = 0; j < 128; j++) {
                stat_dpi->rej_udp.pkt_from_other[i][j] = pnc_buf[ctr++];
            }
        }
        for (i = 0; i < stat_dpi->rej_tcp.from_eth; i++) {
            if (i >= REJ_PKT_CAPACITY) { break; }
            for (j = 0; j < 128; j++) {
                stat_dpi->rej_tcp.pkt_from_eth[i][j] = pnc_buf[ctr++];
            }
        }
        for (i = 0; i < stat_dpi->rej_tcp.from_other; i++) {
            if (i >= REJ_PKT_CAPACITY) { break; }
            for (j = 0; j < 128; j++) {
                stat_dpi->rej_tcp.pkt_from_other[i][j] = pnc_buf[ctr++];
            }
        }
        for (i = 0; i < stat_dpi->rej_unknown.from_eth; i++) {
            if (i >= REJ_PKT_CAPACITY) { break; }
            for (j = 0; j < 128; j++) {
                stat_dpi->rej_unknown.pkt_from_eth[i][j] = pnc_buf[ctr++];
            }
        }
        for (i = 0; i < stat_dpi->rej_unknown.from_other; i++) {
            if (i >= REJ_PKT_CAPACITY) { break; }
            for (j = 0; j < 128; j++) {
                stat_dpi->rej_unknown.pkt_from_other[i][j] = pnc_buf[ctr++];
            }
        }
    }

    pnc_session_destroy(session);

    return EXIT_SUCCESS;
}

void print_reject(int rej_count, char pkt[REJ_PKT_CAPACITY][REJ_PKT_SIZE]) {
    for (int i = 0; i < rej_count; i++) {
        if (i >= REJ_PKT_CAPACITY) { break; }
        for (int j = 0; j < 128; j++) {
            fprintf(stderr, "%02x ", pkt[i][j]);
            if ((j != 0) && ((j + 1) % 16 == 0)) {
                fprintf(stderr, "\n");
            }
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
}

void display_app(statAPP *stat_app_z1, statAPP *stat_app_z2) {
    int maxline = MAX(stat_app_z1->auth_listen_ctr, stat_app_z2->auth_listen_ctr);
    for (int i = 0; i < maxline; i++) {
        char col0[30] = "", col1[30] = "";
        if (i < stat_app_z1->auth_listen_ctr) {        
            sprintf(col0, "%d.%d.%d.%d:%d", 
                stat_app_z1->auth_listen[i].ip[0], 
                stat_app_z1->auth_listen[i].ip[1],
                stat_app_z1->auth_listen[i].ip[2],
                stat_app_z1->auth_listen[i].ip[3],
                stat_app_z1->auth_listen[i].port);
        }
        if (i < stat_app_z2->auth_listen_ctr) {        
            sprintf(col1, "%d.%d.%d.%d:%d", 
                stat_app_z2->auth_listen[i].ip[0], 
                stat_app_z2->auth_listen[i].ip[1],
                stat_app_z2->auth_listen[i].ip[2],
                stat_app_z2->auth_listen[i].ip[3],
                stat_app_z2->auth_listen[i].port);
        }
        fprintf(stderr, "AUTHLSTN| %30s  |%30s  |\n", col0, col1);
    }
    maxline = MAX(stat_app_z1->abandon_listen_ctr, stat_app_z2->abandon_listen_ctr);
    for (int i = 0; i < maxline; i++) {
        char col0[30] = "", col1[30] = "";
        if (i < stat_app_z1->abandon_listen_ctr) {        
            sprintf(col0, "%d.%d.%d.%d:%d",
                stat_app_z1->abandon_listen[i].ip[0],
                stat_app_z1->abandon_listen[i].ip[1],
                stat_app_z1->abandon_listen[i].ip[2],
                stat_app_z1->abandon_listen[i].ip[3],
                stat_app_z1->abandon_listen[i].port);
        }
        if (i < stat_app_z2->abandon_listen_ctr) {        
            sprintf(col1, "%d.%d.%d.%d:%d",
                stat_app_z2->abandon_listen[i].ip[0],
                stat_app_z2->abandon_listen[i].ip[1],
                stat_app_z2->abandon_listen[i].ip[2],
                stat_app_z2->abandon_listen[i].ip[3],
                stat_app_z2->abandon_listen[i].port);
        }
        fprintf(stderr, "ABDNLSTN| %30s  |%30s  |\n", col0, col1);
    }
    maxline = MAX(stat_app_z1->est_accept_ctr, stat_app_z2->est_accept_ctr);
    for (int i = 0; i < maxline; i++) {
        char col0[30] = "", col1[30] = "";
        if (i < stat_app_z1->est_accept_ctr) {        
            sprintf(col0, "%d %d.%d.%d.%d:%d",
                stat_app_z1->est_accept[i].uid, 
                stat_app_z1->est_accept[i].ip[0], 
                stat_app_z1->est_accept[i].ip[1],
                stat_app_z1->est_accept[i].ip[2],
                stat_app_z1->est_accept[i].ip[3],
                stat_app_z1->est_accept[i].port);
        }
        if (i < stat_app_z2->est_accept_ctr) {        
            sprintf(col1, "%d %d.%d.%d.%d:%d",
                stat_app_z2->est_accept[i].uid,
                stat_app_z2->est_accept[i].ip[0],
                stat_app_z2->est_accept[i].ip[1],
                stat_app_z2->est_accept[i].ip[2],
                stat_app_z2->est_accept[i].ip[3],
                stat_app_z2->est_accept[i].port);
        }
        fprintf(stderr, "ESTDACPT| %30s  |%30s  |\n", col0, col1);
    }
    maxline = MAX(stat_app_z1->est_connect_ctr, stat_app_z2->est_connect_ctr);
    for (int i = 0; i < maxline; i++) {
        char col0[30] = "", col1[30] = "";
        if (i < stat_app_z1->est_connect_ctr) {        
            sprintf(col0, "%d %d.%d.%d.%d:%d",
                stat_app_z1->est_connect[i].uid,
                stat_app_z1->est_connect[i].ip[0],
                stat_app_z1->est_connect[i].ip[1],
                stat_app_z1->est_connect[i].ip[2],
                stat_app_z1->est_connect[i].ip[3],
                stat_app_z1->est_connect[i].port);
        }
        if (i < stat_app_z2->est_connect_ctr) {        
            sprintf(col1, "%d %d.%d.%d.%d:%d",
                stat_app_z2->est_connect[i].uid,
                stat_app_z2->est_connect[i].ip[0],
                stat_app_z2->est_connect[i].ip[1],
                stat_app_z2->est_connect[i].ip[2],
                stat_app_z2->est_connect[i].ip[3],
                stat_app_z2->est_connect[i].port);
        }
        fprintf(stderr, "ESTDCONN| %30s  |%30s  |\n", col0, col1);
    }
    maxline = MAX(stat_app_z1->close_accept_ctr, stat_app_z2->close_accept_ctr);
    for (int i = 0; i < maxline; i++) {
        char col0[30] = "", col1[30] = "";
        if (i < stat_app_z1->close_accept_ctr) {        
            sprintf(col0, "%d %d.%d.%d.%d:%d",
                stat_app_z1->close_accept[i].uid,
                stat_app_z1->close_accept[i].ip[0],
                stat_app_z1->close_accept[i].ip[1],
                stat_app_z1->close_accept[i].ip[2],
                stat_app_z1->close_accept[i].ip[3],
                stat_app_z1->close_accept[i].port);
        }
        if (i < stat_app_z2->close_accept_ctr) {        
            sprintf(col1, "%d %d.%d.%d.%d:%d",
                stat_app_z2->close_accept[i].uid,
                stat_app_z2->close_accept[i].ip[0],
                stat_app_z2->close_accept[i].ip[1],
                stat_app_z2->close_accept[i].ip[2],
                stat_app_z2->close_accept[i].ip[3],
                stat_app_z2->close_accept[i].port);
        }
        fprintf(stderr, "CLSDACPT| %30s  |%30s  |\n", col0, col1);
    }
    maxline = MAX(stat_app_z1->close_connect_ctr, stat_app_z2->close_connect_ctr);
    for (int i = 0; i < maxline; i++) {
        char col0[30] = "", col1[30] = "";
        if (i < stat_app_z1->close_connect_ctr) {        
            sprintf(col0, "%d %d.%d.%d.%d:%d",
                stat_app_z1->close_connect[i].uid,
                stat_app_z1->close_connect[i].ip[0],
                stat_app_z1->close_connect[i].ip[1],
                stat_app_z1->close_connect[i].ip[2],
                stat_app_z1->close_connect[i].ip[3],
                stat_app_z1->close_connect[i].port);
        }
        if (i < stat_app_z2->close_connect_ctr) {        
            sprintf(col1, "%d %d.%d.%d.%d:%d",
                stat_app_z2->close_connect[i].uid,
                stat_app_z2->close_connect[i].ip[0],
                stat_app_z2->close_connect[i].ip[1],
                stat_app_z2->close_connect[i].ip[2],
                stat_app_z2->close_connect[i].ip[3],
                stat_app_z2->close_connect[i].port);
        }
        fprintf(stderr, "CLSDCONN| %30s  |%30s  |\n", col0, col1);
    }
}

void display_dpi(statDPI *stat_dpi_z1, statDPI *stat_dpi_z2) {
    fprintf(stderr, "____________________________________________________________________________\n");
    fprintf(stderr, "        |             ZONE 1              |               ZONE 2           |\n");
    fprintf(stderr, " -PASS- | acc_from_eth ->  sent_to_other -> acc_from_other ->  sent_to_eth |\n");
    fprintf(stderr, "    udp |   %10d  |     %10d  |     %10d  |   %10d |\n",
        stat_dpi_z1->udp.acc_from_eth,    stat_dpi_z1->udp.sent_to_other,    stat_dpi_z2->udp.acc_from_other,    stat_dpi_z2->udp.sent_to_eth);
    fprintf(stderr, " someip |   %10d  |     %10d  |     %10d  |   %10d |\n",
        stat_dpi_z1->someip.acc_from_eth, stat_dpi_z1->someip.sent_to_other, stat_dpi_z2->someip.acc_from_other, stat_dpi_z2->someip.sent_to_eth);
    fprintf(stderr, "    tcp |   %10d  |     %10d  |     %10d  |   %10d |\n",
        stat_dpi_z1->tcp.acc_from_eth,    stat_dpi_z1->tcp.sent_to_other,    stat_dpi_z2->tcp.acc_from_other,    stat_dpi_z2->tcp.sent_to_eth);
    fprintf(stderr, "        |  sent_to_eth <- acc_from_other <-  sent_to_other <- acc_from_eth |\n");
    fprintf(stderr, "    udp |   %10d  |     %10d  |     %10d  |   %10d |\n",
        stat_dpi_z1->udp.sent_to_eth,    stat_dpi_z1->udp.acc_from_other,    stat_dpi_z2->udp.sent_to_other,    stat_dpi_z2->udp.acc_from_eth);
    fprintf(stderr, " someip |   %10d  |     %10d  |     %10d  |   %10d |\n",
        stat_dpi_z1->someip.sent_to_eth, stat_dpi_z1->someip.acc_from_other, stat_dpi_z2->someip.sent_to_other, stat_dpi_z2->someip.acc_from_eth);
    fprintf(stderr, "    tcp |   %10d  |     %10d  |     %10d  |   %10d |\n",
        stat_dpi_z1->tcp.sent_to_eth,    stat_dpi_z1->tcp.acc_from_other,    stat_dpi_z2->tcp.sent_to_other,    stat_dpi_z2->tcp.acc_from_eth);
    fprintf(stderr, "___________________________________________________________________________|\n");
    fprintf(stderr, "-REJECT-|     from_eth  |     from_other  |     from_other  |    from_eth  |\n");
    fprintf(stderr, "    udp |   %10d  |     %10d  |     %10d  |  %10d  |\n",
        stat_dpi_z1->rej_udp.from_eth, stat_dpi_z1->rej_udp.from_other, stat_dpi_z2->rej_udp.from_other, stat_dpi_z2->rej_udp.from_eth);
    fprintf(stderr, "    tcp |   %10d  |     %10d  |     %10d  |  %10d  |\n",
        stat_dpi_z1->rej_tcp.from_eth, stat_dpi_z1->rej_tcp.from_other, stat_dpi_z2->rej_tcp.from_other, stat_dpi_z2->rej_tcp.from_eth);
    fprintf(stderr, "unknown |   %10d  |     %10d  |     %10d  |  %10d  |\n",
        stat_dpi_z1->rej_unknown.from_eth, stat_dpi_z1->rej_unknown.from_other, stat_dpi_z2->rej_unknown.from_other, stat_dpi_z2->rej_unknown.from_eth);
    fprintf(stderr, "___________________________________________________________________________|\n");
}

void display_reject(statDPI *stat_dpi_z1, statDPI *stat_dpi_z2) {
    fprintf(stderr, "Z1 REJECT UDP from_eth\n");
    print_reject(stat_dpi_z1->rej_udp.from_eth, stat_dpi_z1->rej_udp.pkt_from_eth);
    fprintf(stderr, "Z1 REJECT UDP from_other\n");
    print_reject(stat_dpi_z1->rej_udp.from_other, stat_dpi_z1->rej_udp.from_other);
    fprintf(stderr, "Z1 REJECT TCP from_eth\n");
    print_reject(stat_dpi_z1->rej_tcp.from_eth, stat_dpi_z1->rej_tcp.pkt_from_eth);
    fprintf(stderr, "Z1 REJECT TCP from_other\n");
    print_reject(stat_dpi_z1->rej_tcp.from_other, stat_dpi_z1->rej_tcp.from_other);
    fprintf(stderr, "Z1 REJECT Unknown from_eth\n");
    print_reject(stat_dpi_z1->rej_unknown.from_eth, stat_dpi_z1->rej_unknown.pkt_from_eth);
    fprintf(stderr, "Z1 REJECT Unknown from_other\n");
    print_reject(stat_dpi_z1->rej_unknown.from_other, stat_dpi_z1->rej_unknown.from_other);

    fprintf(stderr, "Z2 REJECT UDP from_eth\n");
    print_reject(stat_dpi_z2->rej_udp.from_eth, stat_dpi_z2->rej_udp.pkt_from_eth);
    fprintf(stderr, "Z2 REJECT UDP from_other\n");
    print_reject(stat_dpi_z2->rej_udp.from_other, stat_dpi_z2->rej_udp.from_other);
    fprintf(stderr, "Z2 REJECT TCP from_eth\n");
    print_reject(stat_dpi_z2->rej_tcp.from_eth, stat_dpi_z2->rej_tcp.pkt_from_eth);
    fprintf(stderr, "Z2 REJECT TCP from_other\n");
    print_reject(stat_dpi_z2->rej_tcp.from_other, stat_dpi_z2->rej_tcp.from_other);
    fprintf(stderr, "Z2 REJECT Unknown from_eth\n");
    print_reject(stat_dpi_z2->rej_unknown.from_eth, stat_dpi_z2->rej_unknown.pkt_from_eth);
    fprintf(stderr, "Z2 REJECT Unknown from_other\n");
    print_reject(stat_dpi_z2->rej_unknown.from_other, stat_dpi_z2->rej_unknown.from_other);
}

int get_data(char *path, char **buf, size_t *len) 
{
    int fd;
    struct stat st;
    ssize_t read_count;

    if ((fd = open(path, O_RDONLY)) == -1)
    {
        fprintf(stderr, "Cannot open file : %s\n", path);
        return EXIT_FAILURE;
    }

    if (fstat(fd, &st) == 0) {
        *len = st.st_size;
        DEBUG_PRINT("File size : %ld \n", *len);  
    } else {
        goto exit;
    }

    if ((*buf = malloc(*len)) == NULL) {
        fprintf(stderr, "Malloc failed!\n");
        goto exit;
    }

    read_count = read(fd, *buf, *len);
    DEBUG_PRINT("Read count : %ld\n", read_count);  
    DEBUG_PRINT("Data : %s\n", *buf);  

exit:
    close(fd);
    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    int func = -1;
    char *tbl_id = NULL;
    size_t tbl_len = 0;
    char *data = NULL;
    size_t data_len = 0;

    DEBUG_PRINT("Debugging is enabled. Debug level : %d \n", DEBUG);  

    if (argc <= 1) {
        print_help(argv);
        return EXIT_SUCCESS;
    }

    // Get function call
    if ((strcmp(argv[1], "push-table") == 0) && (argc >= 4)) {func = 0;}
    else if ((strcmp(argv[1], "stat") == 0) && (argc >= 2)) {func = 1;}
    else if (strcmp(argv[1], "help") == 0) { print_help(argv); return EXIT_SUCCESS; }
    else { 
        goto exit_failure;
    }

    
    fprintf(stdout,"START OF PROXYCLIENT\n");

    switch(func) {
        // function "push-table"
        case 0:
            // Get data
            if (strcmp(argv[2], "--data") == 0) {
                get_data(argv[3], &data, &data_len);
            } else {
                fprintf(stderr, "Missing --data <path>\n");
                goto exit_failure;
            }

            if ((argc == 5) && strcmp(argv[4], "--dpi") == 0) {
                push_table(data, data_len, "proxy_zone_1");
                push_table(data, data_len, "proxy_zone_2");
            } else if ((argc == 5) && strcmp(argv[4], "--app") == 0) {
                //push_table(data, data_len, "proxyapp_zone_1");
                //push_table(data, data_len, "proxyapp_zone_2");
            } else {
                push_table(data, data_len, "proxy_zone_1");
                push_table(data, data_len, "proxy_zone_2");
                //push_table(data, data_len, "proxyapp_zone_1");
                //push_table(data, data_len, "proxyapp_zone_2");
            }
            break;
        // function "stat"
        case 1:
            if (argc == 2) {
                statDPI stat_dpi_z1;
                statDPI stat_dpi_z2;
                statAPP stat_app_z1;
                statAPP stat_app_z2;
                get_stat_dpi("proxy_zone_1", &stat_dpi_z1);
                get_stat_dpi("proxy_zone_2", &stat_dpi_z2);
                display_dpi(&stat_dpi_z1, &stat_dpi_z2);
                //get_stat_app("proxyapp_zone_1", &stat_app_z1);
                //get_stat_app("proxyapp_zone_2", &stat_app_z2);
                //display_app(&stat_app_z1, &stat_app_z2);
            } else if ((argc == 3) && strcmp(argv[2], "--dpi") == 0) {
                statDPI stat_dpi_z1;
                statDPI stat_dpi_z2;
                get_stat_dpi("proxy_zone_1", &stat_dpi_z1);
                get_stat_dpi("proxy_zone_2", &stat_dpi_z2);
                display_dpi(&stat_dpi_z1, &stat_dpi_z2);
                display_reject(&stat_dpi_z1, &stat_dpi_z2);
            } else if ((argc == 3) && strcmp(argv[2], "--app") == 0) {
                statAPP stat_app_z1;
                statAPP stat_app_z2;
                //get_stat_app("proxyapp_zone_1", &stat_app_z1);
                //get_stat_app("proxyapp_zone_2", &stat_app_z2);
                //display_app(&stat_app_z1, &stat_app_z2);
            } else {
                fprintf(stderr, "Missing --dpi or --app\n");
                goto exit_failure;
            }
            break;
        default:
            goto exit_failure;
    }

    if(data) free(data);
    return EXIT_SUCCESS;

exit_failure:
    print_hint(argv);
    if(data) free(data);
    return EXIT_FAILURE;
}
