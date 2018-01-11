/* AaltoMME - Mobility Management Entity for LTE networks
 * Copyright (C) 2013 Vicent Ferrer Guash & Jesus Llorente Santos
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file   gtp.h
 * @author Vicent Ferrer
 * @date   March, 2013
 * @brief  GTPv2 encoding and decoding functions. Based on OpenGGSN.
 *
 */

#pragma once

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "gtpie.h"


#define GTP_DEBUG 0              /* Print debug information */

#define GTP_MODE_GGSN 1
#define GTP_MODE_SGSN 2
#define GTP_MODE_NO_BIND 3

#define GTP0_PORT	3386
#define GTP1C_PORT	2123
#define GTP2C_PORT      2123
#define GTP1U_PORT	2152
#define PACKET_MAX      8196

#define GTP_MAX  0xffff         /* TODO: Choose right number */
#define GTP0_HEADER_SIZE 20
#define GTP1_HEADER_SIZE_SHORT  8
#define GTP1_HEADER_SIZE_LONG  12
#define GTP2_MINIMUM_HEADER_SIZE                       8    /**< Size of GTPv2c minimun header       */
#define GTP2_SPECIFIC_HEADER_SIZE                      12   /**< Size of GTPv2c EPC specific header  */

#define SYSLOG_PRINTSIZE 255
#define ERRMSG_SIZE 255

#define RESTART_FILE "gsn_restart"
#define NAMESIZE 1024

/* GTP version 1 extension header type definitions. */
#define GTP_EXT_PDCP_PDU    0xC0 /* PDCP PDU Number */

/* GTP version 1 message type definitions. Also covers version 0 except *
 * for anonymous PDP context which was superceded in version 1 */

/* 0 For future use. */
#define GTP_ECHO_REQ          1 /* Echo Request */
#define GTP_ECHO_RSP          2 /* Echo Response */
#define GTP_NOT_SUPPORTED     3 /* Version Not Supported */
#define GTP_ALIVE_REQ         4 /* Node Alive Request */
#define GTP_ALIVE_RSP         5 /* Node Alive Response */
#define GTP_REDIR_REQ         6 /* Redirection Request */
#define GTP_REDIR_RSP         7 /* Redirection Response */
/* 8-15 For future use. */
#define GTP_CREATE_PDP_REQ   16 /* Create PDP Context Request */
#define GTP_CREATE_PDP_RSP   17 /* Create PDP Context Response */
#define GTP_UPDATE_PDP_REQ   18 /* Update PDP Context Request */
#define GTP_UPDATE_PDP_RSP   19 /* Update PDP Context Response */
#define GTP_DELETE_PDP_REQ   20 /* Delete PDP Context Request */
#define GTP_DELETE_PDP_RSP   21 /* Delete PDP Context Response */
/* 22-25 For future use. */ /* In version GTP 1 anonomous PDP context */
#define GTP_ERROR            26 /* Error Indication */
#define GTP_PDU_NOT_REQ      27 /* PDU Notification Request */
#define GTP_PDU_NOT_RSP      28 /* PDU Notification Response */
#define GTP_PDU_NOT_REJ_REQ  29 /* PDU Notification Reject Request */
#define GTP_PDU_NOT_REJ_RSP  30 /* PDU Notification Reject Response */
#define GTP_SUPP_EXT_HEADER  31 /* Supported Extension Headers Notification */
#define GTP_SND_ROUTE_REQ    32 /* Send Routeing Information for GPRS Request */
#define GTP_SND_ROUTE_RSP    33 /* Send Routeing Information for GPRS Response */
#define GTP_FAILURE_REQ      34 /* Failure Report Request */
#define GTP_FAILURE_RSP      35 /* Failure Report Response */
#define GTP_MS_PRESENT_REQ   36 /* Note MS GPRS Present Request */
#define GTP_MS_PRESENT_RSP   37 /* Note MS GPRS Present Response */
/* 38-47 For future use. */
#define GTP_IDEN_REQ         48 /* Identification Request */
#define GTP_IDEN_RSP         49 /* Identification Response */
#define GTP_SGSN_CONTEXT_REQ 50 /* SGSN Context Request */
#define GTP_SGSN_CONTEXT_RSP 51 /* SGSN Context Response */
#define GTP_SGSN_CONTEXT_ACK 52 /* SGSN Context Acknowledge */
#define GTP_FWD_RELOC_REQ    53 /* Forward Relocation Request */
#define GTP_FWD_RELOC_RSP    54 /* Forward Relocation Response */
#define GTP_FWD_RELOC_COMPL  55 /* Forward Relocation Complete */
#define GTP_RELOC_CANCEL_REQ 56 /* Relocation Cancel Request */
#define GTP_RELOC_CANCEL_RSP 57 /* Relocation Cancel Response */
#define GTP_FWD_SRNS         58 /* Forward SRNS Context */
#define GTP_FWD_RELOC_ACK    59 /* Forward Relocation Complete Acknowledge */
#define GTP_FWD_SRNS_ACK     60 /* Forward SRNS Context Acknowledge */
/* 61-239 For future use. */
#define GTP_DATA_TRAN_REQ   240 /* Data Record Transfer Request */
#define GTP_DATA_TRAN_RSP   241 /* Data Record Transfer Response */
/* 242-254 For future use. */
#define GTP_GPDU            255 /* G-PDU */




/* GTP version 2 message type definitions. from 29.274  v11.5.0 6.1-1*/
/* GTP Message Type Values */
#define GTP2_ECHO_REQ                                          1
#define GTP2_ECHO_RSP                                          2
#define GTP2_VERSION_NOT_SUPPORTED_IND                         3
#define GTP2_CREATE_SESSION_REQ                               32
#define GTP2_CREATE_SESSION_RSP                               33
#define GTP2_MODIFY_BEARER_REQ                                34
#define GTP2_MODIFY_BEARER_RSP                                35
#define GTP2_DELETE_SESSION_REQ                               36
#define GTP2_DELETE_SESSION_RSP                               37
#define GTP2_CHANGE_NOTIFICATION_REQ                          38
#define GTP2_CHANGE_NOTIFICATION_RSP                          39
#define GTP2_MODIFY_BEARER_CMD                                64
#define GTP2_MODIFY_BEARER_FAILURE_IND                        65
#define GTP2_DELETE_BEARER_CMD                                66
#define GTP2_DELETE_BEARER_FAILURE_IND                        67
#define GTP2_BEARER_RESOURCE_CMD                              68
#define GTP2_BEARER_RESOURCE_FAILURE_IND                      69
#define GTP2_DOWNLINK_DATA_NOTIFICATION_FAILURE_IND           70
#define GTP2_TRACE_SESSION_ACTIVATION                         71
#define GTP2_TRACE_SESSION_DEACTIVATION                       72
#define GTP2_STOP_PAGING_IND                                  73
#define GTP2_CREATE_BEARER_REQ                                95
#define GTP2_CREATE_BEARER_RSP                                96
#define GTP2_UPDATE_BEARER_REQ                                97
#define GTP2_UPDATE_BEARER_RSP                                98
#define GTP2_DELETE_BEARER_REQ                                99
#define GTP2_DELETE_BEARER_RSP                               100
#define GTP2_DELETE_PDN_CONNECTION_SET_REQ                   101
#define GTP2_DELETE_PDN_CONNECTION_SET_RSP                   102
#define GTP2_PGW_DOWNLINK_TRIGGERING_NTF                     103
#define GTP2_PGW_DOWNLINK_TRIGGERING_ACK                     104
#define GTP2_IDENTIFICATION_REQ                              128
#define GTP2_IDENTIFICATION_RSP                              129
#define GTP2_CONTEXT_REQ                                     130
#define GTP2_CONTEXT_RSP                                     131
#define GTP2_CONTEXT_ACK                                     132
#define GTP2_FORWARD_RELOCATION_REQ                          133
#define GTP2_FORWARD_RELOCATION_RSP                          134
#define GTP2_FORWARD_RELOCATION_COMPLETE_NTF                 135
#define GTP2_FORWARD_RELOCATION_COMPLETE_ACK                 136
#define GTP2_FORWARD_ACCESS_CONTEXT_NTF                      137
#define GTP2_FORWARD_ACCESS_CONTEXT_ACK                      138
#define GTP2_RELOCATION_CANCEL_REQ                           139
#define GTP2_RELOCATION_CANCEL_RSP                           140
#define GTP2_CONFIGURE_TRANSFER_TUNNEL                       141
#define GTP2_DETACH_NTF                                      149
#define GTP2_DETACH_ACK                                      150
#define GTP2_CS_PAGING_INDICATION                            151
#define GTP2_RAN_INFORMATION_RELAY                           152
#define GTP2_ALERT_MME_NTF                                   153
#define GTP2_ALERT_MME_ACK                                   154
#define GTP2_UE_ACTIVITY_NTF                                 155
#define GTP2_UE_ACTIVITY_ACK                                 156
#define GTP2_ISR_STATUS_INDICATION                           157
#define GTP2_CREATE_FORWARDING_TUNNEL_REQ                    160
#define GTP2_CREATE_FORWARDING_TUNNEL_RSP                    161
#define GTP2_SUSPEND_NTF                                     162
#define GTP2_SUSPEND_ACK                                     163
#define GTP2_RESUME_NTF                                      164
#define GTP2_RESUME_ACK                                      165
#define GTP2_CREATE_INDIRECT_DATA_FORWARDING_TUNNEL_REQ      166
#define GTP2_CREATE_INDIRECT_DATA_FORWARDING_TUNNEL_RSP      167
#define GTP2_DELETE_INDIRECT_DATA_FORWARDING_TUNNEL_REQ      168
#define GTP2_DELETE_INDIRECT_DATA_FORWARDING_TUNNEL_RSP      169
#define GTP2_RELEASE_ACCESS_BEARERS_REQ                      170
#define GTP2_RELEASE_ACCESS_BEARERS_RSP                      171
#define GTP2_DOWNLINK_DATA_NOTIFICATION                      176
#define GTP2_DOWNLINK_DATA_NOTIFICATION_ACK                  177
//#define GTP2_RESERVED                                        178
#define GTP2_UPDATE_PDN_CONNECTION_SET_REQ                   200
#define GTP2_UPDATE_PDN_CONNECTION_SET_RSP                   201
#define GTP2_MBMS_SESSION_START_REQ                          231
#define GTP2_MBMS_SESSION_START_RSP                          232
#define GTP2_MBMS_SESSION_UPDATE_REQ                         233
#define GTP2_MBMS_SESSION_UPDATE_RSP                         234
#define GTP2_MBMS_SESSION_STOP_REQ                           235
#define GTP2_MBMS_SESSION_STOP_RSP                           236
//#define GTP2_MSG_END                                         255

struct gtp2_header_short { /*    Descriptions from 3GPP 29274 */
    uint8_t  flags;        /* 01 bitfield, with typical values */
                           /*    010..... Version: 2 */
                           /*    ...1.... Piggybacking flag (P) */
                           /*    ....1... TEID flag (T) */
                           /*    .....0.. Spare = 0 */
                           /*    ......0. Spare = 0 */
                           /*    .......0 Spare = 0 */
    uint8_t  type;         /* 02 Message type. */
    uint16_t length;       /* 03 Length seq(3)+1(spare)+IE length */
    uint32_t seq : 24;     /* 05 Sequence Number (3bytes)*/ /*Use of bit fields*/
    uint8_t  spare1;       /* 18 Spare */
}__attribute__((packed));

struct gtp2_header_long {  /*    Descriptions from 3GPP 29274 */
    uint8_t  flags;        /* 01 bitfield, with typical values */
                           /*    010..... Version: 2 */
                           /*    ...1.... Piggybacking flag (P) */
                           /*    ....1... TEID flag (T) */
                           /*    .....0.. Spare = 0 */
                           /*    ......0. Spare = 0 */
                           /*    .......0 Spare = 0 */
    uint8_t  type;         /* 02 Message type. */
    uint16_t length;       /* 03 Length tei(4)+seq(3)+1(spare)+IE length*/
    uint32_t tei;          /* 05 Tunnel Endpoint ID */
    uint32_t seq : 24 ;    /* 09 Sequence Number (3bytes)*/ /*Use of bit fields*/
    uint8_t  spare1;       /* 12 Spare */
}__attribute__((packed));

struct gtp2_packet_short {
    struct gtp2_header_short h;
    uint8_t p[GTP_MAX];
} __attribute__((packed));

struct gtp2_packet_long {
    struct gtp2_header_long h;
    uint8_t p[GTP_MAX];
} __attribute__((packed));

struct flags_t {
    uint8_t spare     :3;     /**< Spare bits*/
    uint8_t tf        :1;     /**< teid_flag*/
    uint8_t pf        :1;     /**< Piggybacking flag*/
    uint8_t version   :3;     /**< Version field*/
} __attribute__((packed));

union gtp_packet {
    uint8_t                   flags;
    struct flags_t            nflags;
    struct gtp2_packet_short  gtp2s;
    struct gtp2_packet_long   gtp2l;
} __attribute__((packed));

struct node_t{

};

/* External API functions */

extern const char* gtp_version();

extern int gtp2_recv(int sockfd, union gtp_packet *packet, size_t *len, struct sockaddr *src_addr, socklen_t *addrlen);

extern unsigned int get_default_gtp(int version, uint8_t type, void *packet);

extern int print_packet(void *packet, unsigned len);

/* Utils */

/**
 *@brief fill FEID struct
 *@param [out] f FEID to be filled
 *@param [in]  t interface type
 *@param [in]  t TEID
 *@param [in]  IP address
 *@param [in]  IP address length
 */
void gtp_socktofeid(struct fteid_t *f,
                    enum iface_type const t, uint32_t const teid,
                    struct sockaddr const *addr, socklen_t const addrLen);
