/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and funded by EIT ICT labs.
 */

/**
 * @file   MME_Controller.c
 * @Author Robin Babujee Jerome
 * @Author Vicent Ferrer Guasch
 * @date November 2013
 * @brief MME Controller interface protocol state machine.
 *
 * This module implements the SDN Controller interface state machine. The states
 * are separated on STATE_* and TASK_MME_Controller___*. The firsts ones
 * implements the flow control and the second ones the processing
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "signals.h"
#include <syslog.h>
#include <errno.h>
#include <glib.h>
#include "logmgr.h"
#include "MME_Controller.h"
#include "MME.h"
#include "EMMCtx_iface.h"
#include "nodemgr.h"
#include "ESM_BearerContext.h"
#include "EPS_Session_priv.h"
#include "NAS_EMM.h"

#define CONTROLLER_PORT 12345
#define PACKET_MAX      8196

typedef struct{
    gpointer        mme;
    int             fd;
    struct sockaddr addr;
    size_t          addrlen;
}SDNCtrl_t;

void ctrl_accept(evutil_socket_t listener, short event, void *arg){

    uint32_t teid;

    SDNCtrl_t *self = (SDNCtrl_t *)arg;

    log_msg(LOG_DEBUG, 0, "New SDN Controller message recv");
}

gpointer sdnCtrl_init(gpointer mme){
    struct nodeinfo_t ctrlInfo;
    struct sockaddr_in *peer;

    SDNCtrl_t *self = g_new0(SDNCtrl_t, 1);

    self->mme = mme;
    self->fd =init_udp_srv(mme_getLocalAddress(self->mme), CONTROLLER_PORT);
    log_msg(LOG_INFO, 0, "Open SDN Controller server on file descriptor %d, "
            "port %d", self->fd, CONTROLLER_PORT);
    getNode(&ctrlInfo, CTRL, NULL);

    peer = (struct sockaddr_in *)&(self->addr);
    peer->sin_family = AF_INET;
    peer->sin_port = htons(CONTROLLER_PORT);
    peer->sin_addr.s_addr = ctrlInfo.addrv4.s_addr;
    self->addrlen = sizeof(struct sockaddr_in);

    return self;
}

void sdnCtrl_free(gpointer ctrl_h){
    SDNCtrl_t *self = (SDNCtrl_t*)ctrl_h;
    close(self->fd);
    g_free(self);
}

static void sdnCtrl_send(SDNCtrl_t *self, char* msg){
    size_t len = strlen(msg);
    if (sendto(self->fd, msg, len, 0,
               &(self->addr), self->addrlen) < 0) {
        log_errpack(LOG_ERR, errno, (struct sockaddr_in *)&(self->addr),
                    msg, len,
                    "Sendto(fd=%d, msg=%s) failed",
                    self->fd, msg);
    }
}


/* ======================================================================
 * TASK Implementations
 * ====================================================================== */
static void TASK_MME_Controller___userAttach(SDNCtrl_t* self, gpointer emm){

    /* This function is used to send the message to the SDN controller that a
     * new user has attached to the eNB. For this, it creates a structure of
     * type ctrl_message which contains the IP of the SGW, GTP tunnel Ids etc.
     */

    uint32_t tunnelId;
    uint8_t packet_str[PACKET_MAX], tmp_str[PACKET_MAX];
    uint8_t  straddr[INET6_ADDRSTRLEN];
    struct in_addr ipv4enb, ipv4gw, ipv4addr;
    uint8_t ip_enb[INET_ADDRSTRLEN], ip_sgw[INET_ADDRSTRLEN];
    guint64 imsi;
    GList *bearers, *sessions;
    GList *first, *first_session;
    ESM_BearerContext bearer;
    EPS_Session session;
    struct fteid_t s1u_eNB, s1u_sgw;
    gsize fteid_size=0;

    const uint8_t *packet_pattern = "{"
        "\"version\": \"%d\","
        "\"msg_type\": \"attach\","
        "\"msg\": {"
            "\"plmn\": \"%.3llu%.2llu\","
            "\"ue_msisdn\": \"%llu\","
            "%s"
            /*"\"ue_ipv4addr\": \"10.10.255.254\","*/
            /*"\"ue_ipv6addr\": \"2001:0db8:85a3:0042:1000:8a2e:0370:7334\","*/
            "\"gtp_spgw_ipv4addr\": \"%s\","
            "\"gtp_teid_ul\": \"%u\","
            "\"gtp_enb_ipv4addr\": \"%s\","
            "\"gtp_teid_dl\": \"%u\","
            "\"gtp_qci\": \"%u\""
        "},"
        "\"notes\": \"Attach of an UE to the network\"}";

    log_msg(LOG_DEBUG, 0, "Enter");

    /* ======================================================================
     * Setting values in the packet
     * ====================================================================== */

    emm_getEPSSessions(emm, &sessions);
    first_session = g_list_first(sessions);
    session = (EPS_Session)(first_session->data);
    emm_getBearers(emm, &bearers);
    first = g_list_first(bearers);
    bearer = (ESM_BearerContext)(first->data);

    switch(ePSsession_getPDNType(session)){
    case  1: /* IPv4*/
        sprintf(tmp_str, "\"ue_ipv4addr\": \"%s\",",
                ePSsession_getPDNAddrStr(session, straddr, INET6_ADDRSTRLEN));
        log_msg(LOG_DEBUG, 0, "UE: Only IPv4 address is present %s", straddr);
        break;
    case 2: /* IPv6*/
        sprintf(tmp_str, "\"ue_ipv6addr\": \"%s\",",
                ePSsession_getPDNAddrStr(session, straddr, INET6_ADDRSTRLEN));
        log_msg(LOG_DEBUG, 0, "UE: Only IPv6 address is present %s", straddr);
        break;
    case 3: /*IPv4v6*/
        /* sprintf(tmp_str, "\"ue_ipv4addr\": \"%s\",\"ue_ipv6addr\": \"%s\",", */
        /*         inet_ntoa(user->pAA.addr.ipv4), inet_ntoa(user->pAA.addr.ipv6)); */
        log_msg(LOG_DEBUG, 0, "UE: Both IPv4 & IPv6 addresses are present."
                " Not Implemented");
        break;
    }

    esm_bc_getS1uSGWfteid(bearer, &s1u_sgw, &fteid_size);
    esm_bc_getS1ueNBfteid(bearer, &s1u_eNB, &fteid_size);
    ipv4enb.s_addr = s1u_eNB.addr.addrv4;
    ipv4gw.s_addr = s1u_sgw.addr.addrv4;
    imsi = emmCtx_getIMSI(emm);

    snprintf(packet_str, PACKET_MAX, packet_pattern,
             1,                                             /* Version */
             imsi/1000000000000,                            /* MCC */
             (imsi/10000000000)%100,                        /* MNC */
             emmCtx_getMSISDN(emm),                         /* MSISDN */
             tmp_str,
             /* SPGW endpoint IP Address */
             inet_ntop(AF_INET, &ipv4gw, ip_sgw, INET_ADDRSTRLEN),
             ntohl(s1u_sgw.teid),                           /* UL TEID */
             /* eNB endpoint IP Address */
             inet_ntop(AF_INET, &ipv4enb, ip_enb, INET_ADDRSTRLEN),
             ntohl(s1u_eNB.teid),                           /* DL TEID */
             9                                              /* QCI */
             );

    /* log_msg(LOG_DEBUG, 0, "Uplink   Tunnel Id: %"PRIu32, */
    /*    ntohl(packet.uL_gtp_teid)); */
    /* log_msg(LOG_DEBUG, 0, "Downlink Tunnel Id: %"PRIu32, */
    /*    ntohl(packet.dL_gtp_teid)); */

    /* ======================================================================
     * Sending packet
     * ====================================================================== */
    sdnCtrl_send(self, packet_str);
    log_msg(LOG_DEBUG, 0, "Sent the new user attach message to SDN Controller");
}

static void TASK_MME_Controller___userDetach(SDNCtrl_t* self, gpointer emm){
    /*
     * This function is used to send the message to the SDN controller that an
     * existing user has detached from the eNB. For this, it creates a structure
     * of type ctrl_message which contains the IP of the SGW, GTP tunnel Ids etc
     */

    uint32_t tunnelId;
    uint8_t packet_str[PACKET_MAX], tmp_str[PACKET_MAX];
    uint8_t straddr[INET6_ADDRSTRLEN];
    struct in_addr ipv4enb, ipv4gw, ipv4addr;
    uint8_t ip_enb[INET_ADDRSTRLEN], ip_sgw[INET_ADDRSTRLEN];
    GList *bearers, *sessions;
    GList *first, *first_session;
    ESM_BearerContext bearer;
    EPS_Session session;
    struct fteid_t s1u_eNB, s1u_sgw;
    gsize fteid_size=0;
    guint64 imsi;

    const uint8_t *packet_pattern = "{"
        "\"version\": \"%d\","
        "\"msg_type\": \"detach\","
        "\"msg\": {"
            "\"plmn\": \"%.3llu%.2llu\","
            "\"ue_msisdn\": \"%llu\","
            "%s"
            /*"\"ue_ipv4addr\": \"10.10.255.254\","*/
            /*"\"ue_ipv6addr\": \"2001:0db8:85a3:0042:1000:8a2e:0370:7334\","*/
            "\"gtp_spgw_ipv4addr\": \"%s\","
            "\"gtp_teid_ul\": \"%u\","
            "\"gtp_enb_ipv4addr\": \"%s\","
            "\"gtp_teid_dl\": \"%u\","
            "\"gtp_qci\": \"%u\""
            "},"
        "\"notes\": \"Detach of an UE from the network\"}";

    log_msg(LOG_DEBUG, 0, "Enter");

    /* ======================================================================
     * Setting values in the packet
     * ====================================================================== */

    emm_getEPSSessions(emm, &sessions);
    first_session = g_list_first(sessions);
    session = (EPS_Session)(first_session->data);
    emm_getBearers(emm, &bearers);
    first = g_list_first(bearers);
    bearer = (ESM_BearerContext)(first->data);

    switch(ePSsession_getPDNType(session)){
    case  1: /* IPv4*/
        sprintf(tmp_str, "\"ue_ipv4addr\": \"%s\",",
                ePSsession_getPDNAddrStr(session, straddr, INET6_ADDRSTRLEN));
        log_msg(LOG_DEBUG, 0, "UE: Only IPv4 address is present %d", straddr);
        break;
    case 2: /* IPv6*/
        sprintf(tmp_str, "\"ue_ipv6addr\": \"%s\",",
                ePSsession_getPDNAddrStr(session, straddr, INET6_ADDRSTRLEN));
        log_msg(LOG_DEBUG, 0, "UE: Only IPv6 address is present %s", straddr);
        break;
    case 3: /*IPv4v6*/
        /* sprintf(tmp_str, "\"ue_ipv4addr\": \"%s\",\"ue_ipv6addr\": \"%s\",", */
        /*        inet_ntoa(user->pAA.addr.ipv4), inet_ntoa(user->pAA.addr.ipv6)); */
        log_msg(LOG_DEBUG, 0, "UE: Both Ipv4 & IPv6 addresses are present."
                " Not Implemented");
        break;
    }

    esm_bc_getS1uSGWfteid(bearer, &s1u_sgw, &fteid_size);
    esm_bc_getS1ueNBfteid(bearer, &s1u_eNB, &fteid_size);
    ipv4enb.s_addr = s1u_eNB.addr.addrv4;
    ipv4gw.s_addr = s1u_sgw.addr.addrv4;
    imsi = emmCtx_getIMSI(emm);

    sprintf(packet_str, packet_pattern,
            1,                                             /* Version*/
            imsi/1000000000000,                            /*MCC*/
            (imsi/10000000000)%100,                        /*MNC*/
            emmCtx_getMSISDN(emm),                         /*MSISDN*/
            tmp_str,
            /*SPGW endpoint IP Address*/
            inet_ntop(AF_INET, &ipv4gw, ip_sgw, INET_ADDRSTRLEN),
            ntohl(s1u_sgw.teid),                           /*UL TEID*/
            /*eNB endpoint IP Address*/
            inet_ntop(AF_INET, &ipv4enb, ip_enb, INET_ADDRSTRLEN),
            ntohl(s1u_eNB.teid),                           /*DL TEID*/
            9                                              /* QCI */
            );

    /* ======================================================================
     * Sending packet
     * ====================================================================== */
    sdnCtrl_send(self, packet_str);
    log_msg(LOG_DEBUG, 0, "Sent the user detach message to SDN Controller");
}

static void TASK_MME_Controller___userHandover(SDNCtrl_t* self, gpointer emm){
    /*
     * This function is used to send two messages to the SDN controller that an
     *  existing user has moved from one eNB to another eNB. For this, it
     * creates a structure of type ctrl_message which contains the IP of the
     * SGW, GTP tunnel Ids etc. */

    uint32_t tunnelId;
    uint8_t packet_str[PACKET_MAX], tmp_str[PACKET_MAX];
    uint8_t straddr[INET6_ADDRSTRLEN];
    struct in_addr ipv4enb, ipv4enb_tgt, ipv4gw, ipv4addr;
    uint8_t ip_enb[INET_ADDRSTRLEN], ip_tgtenb[INET_ADDRSTRLEN];
    uint8_t ip_sgw[INET_ADDRSTRLEN];
    GList *bearers, *sessions;
    GList *first, *first_session;
    ESM_BearerContext bearer;
    EPS_Session session;
    struct fteid_t s1u_eNB, s1u_sgw, s1u_tgteNB;
    gsize fteid_size=0;
    guint64 imsi;

    const uint8_t *packet_pattern = "{"
        "\"version\": \"%d\","
        "\"msg_type\": \"handover\","
        "\"msg\": {"
            "\"plmn\": \"%.3llu%.2llu\","
            "\"ue_msisdn\": \"%llu\","
            "%s"
            /*"\"ue_ipv4addr\": \"10.10.255.254\","*/
            /*"\"ue_ipv6addr\": \"2001:0db8:85a3:0042:1000:8a2e:0370:7334\","*/
            "\"gtp_spgw_ipv4addr\": \"%s\","
            "\"gtp_teid_ul\": \"%u\","
            "\"gtp_src_enb_ipv4addr\": \"%s\","
            "\"gtp_src_teid_dl\": \"%u\","
            "\"gtp_tgt_enb_ipv4addr\": \"%s\","
            "\"gtp_tgt_teid_dl\": \"%u\","
            "\"gtp_qci\": \"%u\""
            "},"
        "\"notes\": \"Handover of an UE\"}";

    log_msg(LOG_DEBUG, 0, "Enter");

    /* ======================================================================
     * Setting values in the packet
     * ====================================================================== */

    emm_getEPSSessions(emm, &sessions);
    first_session = g_list_first(sessions);
    session = (EPS_Session)(first_session->data);
    emm_getBearers(emm, &bearers);
    first = g_list_first(bearers);
    bearer = (ESM_BearerContext)(first->data);

    switch(ePSsession_getPDNType(session)){
    case  1: /* IPv4*/
        sprintf(tmp_str, "\"ue_ipv4addr\": \"%s\",",
                ePSsession_getPDNAddrStr(session, straddr, INET6_ADDRSTRLEN));
        log_msg(LOG_DEBUG, 0, "UE: Only IPv4 address is present %d", straddr);
        break;
    case 2: /* IPv6*/
        sprintf(tmp_str, "\"ue_ipv6addr\": \"%s\",",
                ePSsession_getPDNAddrStr(session, straddr, INET6_ADDRSTRLEN));
        log_msg(LOG_DEBUG, 0, "UE: Only IPv6 address is present %s", straddr);
        break;
    case 3: /*IPv4v6*/
        /* sprintf(tmp_str, "\"ue_ipv4addr\": \"%s\",\"ue_ipv6addr\": \"%s\",", */
        /*        inet_ntoa(user->pAA.addr.ipv4), inet_ntoa(user->pAA.addr.ipv6)); */
        log_msg(LOG_DEBUG, 0, "UE: Both Ipv4 & IPv6 addresses are present."
                " Not Implemented");
        break;
    }

    esm_bc_getS1uSGWfteid(bearer, &s1u_sgw, &fteid_size);
    esm_bc_getS1ueNBfteid(bearer, &s1u_eNB, &fteid_size);
    /* HACK add target eNB*/
    esm_bc_getS1ueNBfteid(bearer, &s1u_tgteNB, &fteid_size);
    ipv4enb.s_addr = s1u_eNB.addr.addrv4;
    ipv4gw.s_addr = s1u_sgw.addr.addrv4;
    ipv4enb_tgt.s_addr = s1u_tgteNB.addr.addrv4;
    imsi = emmCtx_getIMSI(emm);

    sprintf(packet_str, packet_pattern,
            1,                                                 /* Version*/
            imsi/1000000000000,                                /* MCC*/
            (imsi/10000000000)%100,                            /* MNC*/
            emmCtx_getMSISDN(emm),                             /* MSISDN*/
            tmp_str,                                           /* UE IP*/
            /*SPGW endpoint IP Address*/
            inet_ntop(AF_INET, &ipv4gw, ip_sgw, INET_ADDRSTRLEN),
            ntohl(s1u_sgw.teid),              /* UL TEID*/
            /*SRC eNB endpoint IP Address*/
            inet_ntop(AF_INET, &ipv4enb, ip_enb, INET_ADDRSTRLEN),
            ntohl(s1u_tgteNB.teid),   /* SRC DL TEID*/
            /*TGT eNB endpoint IP Address*/
            inet_ntop(AF_INET, &ipv4enb_tgt, ip_tgtenb, INET_ADDRSTRLEN),
            ntohl(s1u_eNB.teid),                               /* TGT DL TEID*/
            9                                                  /* QCI*/
            );

    /* ======================================================================
     * Sending packet
     * ====================================================================== */
    sdnCtrl_send(self, packet_str);
    log_msg(LOG_DEBUG, 0, "Sent the user detach message to SDN Controller");
}

/* ======================================================================
 * SDN Controller API Implementation
 * ====================================================================== */

void Controller_newAttach(gpointer ctrl, gpointer emm){
    SDNCtrl_t* self = (SDNCtrl_t*)ctrl;
    log_msg(LOG_DEBUG, 0, "Entered the Controller_newAttach method");

    TASK_MME_Controller___userAttach(self, emm);
}

void Controller_newDetach(gpointer ctrl, gpointer emm){
    SDNCtrl_t* self = (SDNCtrl_t*)ctrl;
    log_msg(LOG_DEBUG, 0, "Entered the Controller_newDetach method");
    TASK_MME_Controller___userDetach(self, emm);
}

void Controller_newHandover(gpointer ctrl, gpointer emm){
    SDNCtrl_t* self = (SDNCtrl_t*)ctrl;
    log_msg(LOG_DEBUG, 0, "Entered the Controller_newHandover method");
    TASK_MME_Controller___userHandover(self, emm);
}
