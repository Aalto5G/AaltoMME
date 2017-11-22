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
 * @file   S11_User.c
 * @Author Vicent Ferrer
 * @date   June, 2015
 * @brief  S11 User
 *
 */

#include "S11_User.h"
#include "S11_FSMConfig.h"
#include "MME_S11.h"
#include "logmgr.h"
#include "EMMCtx.h"
#include "EPS_Session_priv.h"
#include "ESM_BearerContext.h"

#include <string.h>
#include <netinet/in.h>
#include <glib.h>

#include "gtp.h"

typedef struct{
    guint32            seq;
    int                fd;
    union gtp_packet   oMsg;
    guint32            oMsglen;
    union gtp_packet   iMsg;
    guint32            iMsglen;
    union gtpie_member *ie[GTPIE_SIZE];
}S11_TrxnT;

typedef struct{
    gpointer           s11;      /**< s11 stack handler*/
    S11_State          *state;   /**< s11 state for this user*/
    uint32_t           lTEID;    /**< local control TEID*/
    uint32_t           rTEID;    /**< remote control TEID*/
    struct sockaddr    rAddr;    /**<Peer IP address, IPv4 or IPv6*/
    socklen_t          rAddrLen; /**<Peer Socket length returned by recvfrom*/
    struct fteid_t     s5s8;     /**< F-TEID PGW S5/S8 (Control Plane)*/
    EMMCtx             emm;
    EPS_Session        session;
    Subscription       subs;     /**< Subscription information*/
    uint8_t            cause;
    void               (*cb) (gpointer);
    gpointer           args;
    GHashTable         *trxns; /**< Transactions by sequence number*/
    S11_TrxnT          *active_trxn;
}S11_user_t;

#define PARSE_ERROR parse_error()

GQuark
parse_error (void)
{
  return g_quark_from_static_string ("gtpv2-parse-error");
}

/* Trxn functions*/
static S11_TrxnT *s11uTrxn_new(guint32 seq){
    S11_TrxnT *trxn =  g_new(S11_TrxnT, 1);
    trxn->seq = seq;
    return trxn;
}

static void s11uTrxn_destroy(void *t){
    S11_TrxnT *trxn = (S11_TrxnT *)t;
    g_free(trxn);
}

/* User functions*/
gpointer s11u_newUser(gpointer s11, EMMCtx emm, EPS_Session s){
    S11_user_t *self = g_new0(S11_user_t, 1);
    self->lTEID   = newTeid();
    self->rTEID   = 0;
    self->emm     = emm;
    self->session = s;
    self->subs    = emmCtx_getSubscription(emm);
    self->s11     = s11;

    self->trxns = g_hash_table_new_full( g_int_hash,
                                         g_int_equal,
                                         NULL,
                                         (GDestroyNotify)s11uTrxn_destroy);

    /*Get SGW addr*/
    emmCtx_getSGW(emm, &self->rAddr, &self->rAddrLen);

    /*Initial state noCtx*/
    s11changeState(self, noCtx);
    log_msg(LOG_INFO, 0, "Created S11 session");
    return self;
}

void s11u_freeUser(gpointer u){
    S11_user_t *self = (S11_user_t*)u;
    S11_unrefSession(self->s11, &self->rAddr, self->rAddrLen);
    log_msg(LOG_INFO, 0, "Removing S11 session");
    g_hash_table_destroy(self->trxns);
    g_free(self);
}

static void s11u_newTrxn(S11_user_t *self){
     S11_TrxnT *t = s11uTrxn_new(getNextSeq(self->s11));
     g_hash_table_insert(self->trxns, &t->seq, t);
     self->active_trxn = t;
}

static gboolean s11u_hasPendingResp(S11_user_t *self, guint32 seq, S11_TrxnT **t){
    return g_hash_table_lookup_extended(self->trxns, &seq, NULL, (void**)t);
}

static gboolean validateSourceAddr(S11_user_t* self,
                                   const struct sockaddr *src,
                                   const socklen_t peerlen){
    return self->rAddrLen == peerlen && memcmp(&self->rAddr, src, self->rAddrLen) == 0;
}

void processMsg(gpointer u, const struct t_message *msg){
    S11_user_t *self = (S11_user_t*)u;
    S11_TrxnT *t = NULL;
    char addrStr[INET6_ADDRSTRLEN];

    if (s11u_hasPendingResp(self, msg->packet.gtp.gtp2l.h.seq, &t)){
        log_msg(LOG_DEBUG, 0, "Received pending S11 reply");
        self->active_trxn = t;
    }
    else{
        log_msg(LOG_DEBUG, 0, "Received new S11 request");
        self->active_trxn = s11uTrxn_new(msg->packet.gtp.gtp2l.h.seq);
        t = self->active_trxn;
    }

    t->iMsglen = msg->length;
    memcpy(&(t->iMsg), &(msg->packet.gtp), msg->length);

    if(!validateSourceAddr(self, &msg->peer, msg->peerlen)){
        log_msg(LOG_WARNING, 0, "S11 - Wrong S-GW source (%s)."
                "Ignoring packet", inet_ntop(msg->peer.sa_family,
                                             &((struct sockaddr_in*)&msg->peer)->sin_addr,
                                             addrStr,
                                             msg->peerlen));
        return;
    }

    self->state->processMsg(self);
}

void attach(gpointer session, void(*cb)(gpointer), gpointer args){
    S11_user_t *self = (S11_user_t*)session;
    self->cb = cb;
    self->args = args;
    self->state->attach(self);
}

void detach(gpointer session, void(*cb)(gpointer), gpointer args){
    S11_user_t *self = (S11_user_t*)session;
    self->cb = cb;
    self->args = args;
    self->state->detach(self);
}

void modBearer(gpointer session, void(*cb)(gpointer), gpointer args){
    S11_user_t *self = (S11_user_t*)session;
    self->cb = cb;
    self->args = args;
    self->state->modBearer(self);
}

void releaseAccess(gpointer session, void(*cb)(gpointer), gpointer args){
    S11_user_t *self = (S11_user_t*)session;
    self->cb = cb;
    self->args = args;
    self->state->releaseAccess(self);
}

int *s11u_getTEIDp(gpointer u){
    S11_user_t *self = (S11_user_t*)u;
    return &(self->lTEID);
}

void s11u_setState(gpointer u, S11_State *s){
    S11_user_t *self = (S11_user_t*)u;
    self->state = s;
}


static void s11__send(S11_user_t* self){
    GError *err = NULL;
    /*Packet header modifications*/
    self->active_trxn->oMsg.gtp2l.h.seq = self->active_trxn->seq;
    self->active_trxn->oMsg.gtp2l.h.tei = self->rTEID;

    s11_send(self->s11, &(self->active_trxn->oMsg), self->active_trxn->oMsglen,
             &(self->rAddr), self->rAddrLen, &err);
    if(err != NULL){
        log_msg(LOG_ERR, 0, "s11_send error");
    }
}

static void s11u_send(S11_user_t* self){
    s11__send(self);
}

static void s11_send_resp(S11_user_t* self){
    s11__send(self);
    s11uTrxn_destroy(self->active_trxn);
}

static gboolean isFirstSessionForSGW(S11_user_t* self){
    return S11_isFirstSession(self->s11, &self->rAddr, self->rAddrLen);
}

void returnControl(gpointer u){
    S11_user_t *self = (S11_user_t*)u;
    if(self->cb){
        self->cb(self->args);
    }
}

void returnControlAndRemoveSession(gpointer u){
    S11_user_t *self = (S11_user_t*)u;
    void(*cb)(gpointer);
    gpointer args;
    cb = self->cb;
    args = self->args;
    s11_deleteSession(self->s11, self);
    if(cb){
        cb(args);
    }
}

void parseIEs(gpointer u){
    S11_user_t *self = (S11_user_t*)u;
    S11_TrxnT *t = self->active_trxn;
    gtp2ie_decap(t->ie, &(t->iMsg), t->iMsglen);
}

const int getMsgType(const gpointer u){
    S11_user_t *self = (S11_user_t*)u;
    S11_TrxnT *t = self->active_trxn;
    return t->iMsg.gtp2l.h.type;
}

void dl_data_not(gpointer u){
    S11_user_t *self = (S11_user_t*)u;
    S11_paging(self->s11, self->emm);
}

void sendCreateSessionReq(gpointer u){
    S11_user_t *self = (S11_user_t*)u;
    struct fteid_t  fteid;
    struct qos_t    qos;
    union gtpie_member ie[26], ie_bearer_ctx[3];
    int hlen, a;
    uint32_t ielen, ienum=0, ul, dl;
    uint64_t ul_64, dl_64;
    uint8_t bytefield[30], *tmp;
    uint8_t pco[0xff+2];
    gsize pco_len=0;
    ESM_BearerContext bearer;
    struct sockaddr pgw = {0};
    socklen_t pgwLen = 0;

    log_msg(LOG_DEBUG, 0, "Enter");
    memset(ie, 0, sizeof(union gtpie_member)*14);
    memset(ie_bearer_ctx, 0, sizeof(union gtpie_member)*3);
    /*  Send Create Context Request to SGW*/

    memset(&qos, 0, sizeof(struct qos_t));
    subs_cpyQoS_GTP(self->subs, &qos);

    s11u_newTrxn(self);
    self->active_trxn->oMsglen = get_default_gtp(2, GTP2_CREATE_SESSION_REQ, &(self->active_trxn->oMsg));

    /*IMSI*/
    ie[ienum].tliv.i=0;
    ie[ienum].tliv.t=GTPV2C_IE_IMSI;
    dec2tbcd(ie[ienum].tliv.v, &ielen, emmCtx_getIMSI(self->emm));
    ie[ienum].tliv.l=hton16(ielen);
    ienum++;
    /*MSISDN*/
    ie[ienum].tliv.i=0;
    ie[ienum].tliv.t=GTPV2C_IE_MSISDN;
    dec2tbcd(ie[ienum].tliv.v, &ielen, emmCtx_getMSISDN(self->emm));
    ie[ienum].tliv.l=hton16(ielen);
    ienum++;
    /*MEI*/
    /* ie[ienum].tliv.i   =0; */
    /* ie[ienum].tliv.t=GTPV2C_IE   _MEI; */
    /* dec2tbcd(ie[ienum].tliv.v, &ielen, subs_getIMEISV(self-   >subs)); */
    /* ie[ienum].tliv.l=hton16(ielen); */
    /* ienum++; */

    /*Serving Network*/
    ie[ienum].tliv.i=0;
    ie[ienum].tliv.l=hton16(3);
    ie[ienum].tliv.t=GTPV2C_IE_SERVING_NETWORK;
    memcpy(ie[ienum].tliv.v, emmCtx_getServingNetwork_TBCD(self->emm), 3);
    ienum++;

    /*RAT type*/
    ie[ienum].tliv.i=0;
    ie[ienum].tliv.l=hton16(1);
    ie[ienum].tliv.t=GTPV2C_IE_RAT_TYPE;
    ie[ienum].tliv.v[0]=6;                  /*Type 6= EUTRAN*/
    ienum++;

    /*F-TEID*/
    ie[ienum].tliv.i=0;
    ie[ienum].tliv.l=hton16(9);
    ie[ienum].tliv.t=GTPV2C_IE_FTEID;
    fteid.ipv4=1;
    fteid.ipv6=0;
    fteid.iface= hton8(S11_MME);
    fteid.teid = hton32(self->lTEID);
    inet_pton(AF_INET,
              s11_getLocalAddress(self->s11),
              &(fteid.addr.addrv4));
    ie[ienum].tliv.l=hton16(FTEID_IP4_SIZE);
    memcpy(ie[ienum].tliv.v, &fteid, FTEID_IP4_SIZE);
    ienum++;
    /*F-TEID PGW S5/S8 Address for Control Plane or PMIP */
    ie[ienum].tliv.i=1;
    ie[ienum].tliv.l=hton16(FTEID_IP4_SIZE);
    ie[ienum].tliv.t=GTPV2C_IE_FTEID;
    emmCtx_getPGW(self->emm, &pgw, &pgwLen);
    gtp_socktofeid(&fteid, S5S8C_PGW, 0, &pgw, pgwLen);

    memcpy(ie[ienum].tliv.v, &fteid, FTEID_IP4_SIZE);
    ienum++;
    /*APN*/
    ie[ienum].tliv.i=0;
    ie[ienum].tliv.l=hton16(subs_getAPNlen(self->subs));
    ie[ienum].tliv.t=GTPV2C_IE_APN;
    sprintf(ie[ienum].tliv.v,
            subs_getAPN(self->subs),
            subs_getAPNlen(self->subs));
    ienum++;

    /*Selection Mode*/
    ie[ienum].tliv.i=0;
    ie[ienum].tliv.l=hton16(1);
    ie[ienum].tliv.t=GTPV2C_IE_SELECTION_MODE;
    bytefield[0]=0x01; /* Selection Mode*/
    memcpy(ie[ienum].tliv.v, bytefield, 1);
    ienum++;

    /*PDN type*/
    ie[ienum].tliv.i=0;
    ie[ienum].tliv.l=hton16(1);
    ie[ienum].tliv.t=GTPV2C_IE_PDN_TYPE;
    bytefield[0]=subs_getPDNType(self->subs); /* PDN type IPv4*/
    memcpy(ie[ienum].tliv.v, bytefield, 1);
    ienum++;

    /*PAA*/
    ie[ienum].tliv.i=0;
    ie[ienum].tliv.l=hton16(5);
    ie[ienum].tliv.t=GTPV2C_IE_PAA;
    bytefield[0]=0x01;  /*PDN Type  IPv4 */
    memset(bytefield+1, 0, 4);   /*IP = 0.0.0.0*/
    memcpy(ie[ienum].tliv.v, bytefield, 5);
    ienum++;

    /*APN restriction*/
    ie[ienum].tliv.i=0;
    ie[ienum].tliv.l=hton16(1);
    ie[ienum].tliv.t=GTPV2C_IE_APN_RESTRICTION;
    bytefield[0]=0x00; /* APN restriction*/
    memcpy(ie[ienum].tliv.v, bytefield, 1);
    ienum++;

    /*APN-AMBR*/
    ie[ienum].tliv.i=0;
    ie[ienum].tliv.l=hton16(8);
    ie[ienum].tliv.t=GTPV2C_IE_AMBR;
    subs_getUEAMBR(self->subs, &ul_64, &dl_64);
    ul = htonl((uint32_t)ul_64/1000);
    dl = htonl((uint32_t)dl_64/1000);
    memcpy(ie[ienum].tliv.v, &ul, 4);
    memcpy(ie[ienum].tliv.v+4, &dl, 4);
    ienum++;

    /*Protocol Configuration Options*/
    if(ePSsession_getPCO(self->session, pco, &pco_len)){
        ie[ienum].tliv.i=0;
        ie[ienum].tliv.l=hton16(pco_len);
        ie[ienum].tliv.t=GTPV2C_IE_PCO;
        memcpy(ie[ienum].tliv.v, pco, pco_len);
        ienum++;
    }
    /*Bearer contex*/
    bearer = ePSsession_getDefaultBearer(self->session);
        /*EPS Bearer ID */
        ie_bearer_ctx[0].tliv.i=0;
        ie_bearer_ctx[0].tliv.l=hton16(1);
        ie_bearer_ctx[0].tliv.t=GTPV2C_IE_EBI;
        /*EBI = 5,  EBI > 4, see 3GPP TS 24.007 11.2.3.1.5  EPS bearer identity */
        ie_bearer_ctx[0].tliv.v[0]=esm_bc_getEBI(bearer);
        /* Bearer QoS */
        ie_bearer_ctx[1].tliv.i=0;
        ie_bearer_ctx[1].tliv.l=hton16(sizeof(struct qos_t));
        ie_bearer_ctx[1].tliv.t=GTPV2C_IE_BEARER_LEVEL_QOS;
        memcpy(ie_bearer_ctx[1].tliv.v, &qos, sizeof(struct qos_t));
        /*EPS Bearer TFT */
        /*ie_bearer_ctx[2].tliv.i=0;
        ie_bearer_ctx[2].tliv.l=hton16(3);
        ie_bearer_ctx[2].tliv.t=GTPV2C_IE_BEARER_TFT;
        bytefield[0]=0x01;
        bytefield[1]=0x01;
        bytefield[2]=0x01;
        memcpy(ie_bearer_ctx[2].tliv.v, bytefield, 3);
    gtp2ie_encaps_group(GTPV2C_IE_BEARER_CONTEXT, 0, &ie[12], ie_bearer_ctx, 3);*/
    gtp2ie_encaps_group(GTPV2C_IE_BEARER_CONTEXT, 0, &ie[ienum],
                        ie_bearer_ctx, 2);
    ienum++;

    /* Recovery IE*/
    if(isFirstSessionForSGW(self)){
        ie[ienum].tliv.i=0;
        ie[ienum].tliv.l=hton16(1);
        ie[ienum].tliv.t=GTPV2C_IE_RECOVERY;
        ie[ienum].tliv.v[0]= getRestartCounter(self->s11);
        ienum++;
    }

    gtp2ie_encaps(ie, ienum, &(self->active_trxn->oMsg), &(self->active_trxn->oMsglen));
    s11u_send(self);
}

void sendModifyBearerReq(gpointer u){
    S11_user_t *self = (S11_user_t*)u;
    struct fteid_t  fteid;
    union gtpie_member ie[13], ie_bearer_ctx[3];
    int hlen, a;
    gsize fteid_size;
    ESM_BearerContext bearer;

    memset(ie, 0, sizeof(union gtpie_member)*13);
    memset(ie_bearer_ctx, 0, sizeof(union gtpie_member)*3);

    log_msg(LOG_DEBUG, 0, "Enter");
    /*  Send Create Context Request to SGW*/
    /******************************************************************************/

    s11u_newTrxn(self);
    self->active_trxn->oMsglen = get_default_gtp(2, GTP2_MODIFY_BEARER_REQ, &(self->active_trxn->oMsg));

    /*F-TEID*/
    ie[0].tliv.i=0;
    ie[0].tliv.t=GTPV2C_IE_FTEID;
    ie[0].tliv.l=hton16(FTEID_IP4_SIZE);
    fteid.ipv4=1;
    fteid.ipv6=0;
    fteid.iface= hton8(S11_MME);
    fteid.teid = hton32(self->lTEID);
    inet_pton(AF_INET,
              s11_getLocalAddress(self->s11),
              &(fteid.addr.addrv4));
    memcpy(ie[0].tliv.v, &fteid, FTEID_IP4_SIZE);

    /*Bearer contex*/
    bearer = ePSsession_getDefaultBearer(self->session);
        /*EPS Bearer ID */
        ie_bearer_ctx[0].tliv.i=0;
        ie_bearer_ctx[0].tliv.l=hton16(1);
        ie_bearer_ctx[0].tliv.t=GTPV2C_IE_EBI;
        ie_bearer_ctx[0].tliv.v[0]=esm_bc_getEBI(bearer);
        /* fteid S1-U eNB*/
        esm_bc_getS1ueNBfteid(bearer, &fteid, &fteid_size);
        ie_bearer_ctx[1].tliv.i=0;
        ie_bearer_ctx[1].tliv.t=GTPV2C_IE_FTEID;
        ie_bearer_ctx[1].tliv.l=hton16(fteid_size);
        memcpy(ie_bearer_ctx[1].tliv.v, &fteid, fteid_size);
    gtp2ie_encaps_group(GTPV2C_IE_BEARER_CONTEXT, 0, &ie[1], ie_bearer_ctx, 2);
    gtp2ie_encaps(ie, 2, &(self->active_trxn->oMsg), &(self->active_trxn->oMsglen));

    s11u_send(self);
}

void sendDeleteSessionReq(gpointer u){
    S11_user_t *self = (S11_user_t*)u;

    union gtpie_member ie[13];
    guint ienum = 0;
    memset(ie, 0, sizeof(union gtpie_member)*13);

    /*  Send Delete Session Request to SGW*/
    /******************************************************************************/

    s11u_newTrxn(self);
    self->active_trxn->oMsglen = get_default_gtp(2, GTP2_DELETE_SESSION_REQ, &(self->active_trxn->oMsg));

    /*  EPS Bearer ID (EBI) to be removed*/
    ie[ienum].tliv.i=0;
    ie[ienum].tliv.l=hton16(1);
    ie[ienum].tliv.t=GTPV2C_IE_EBI;
    /*ie[ienum].tliv.v[0]=user.ebi; *//*Future*/
    ie[ienum].tliv.v[0]=0x05; /*EBI = 5,  EBI > 4, see 3GPP TS 24.007 11.2.3.1.5  EPS bearer identity */
    ienum++;

    /* User Location Information */

    /* Indication flgs*/
    ie[ienum].tliv.i=0;
    ie[ienum].tliv.l=hton16(4);
    ie[ienum].tliv.t=GTPV2C_IE_INDICATION;
    bzero(ie[ienum].tliv.v,4);
    ie[ienum].tliv.v[0]=0x08; /* OI flag*/
    ienum++;

    gtp2ie_encaps(ie, ienum, &(self->active_trxn->oMsg), &(self->active_trxn->oMsglen));

    s11u_send(self);
}

void sendReleaseAccessBearersReq(gpointer u){
    S11_user_t *self = (S11_user_t*)u;

    union gtpie_member ie[13];
    memset(ie, 0, sizeof(union gtpie_member)*13);

    /*  Send Release Access Bearers Request to SGW*/
    /******************************************************************************/

    s11u_newTrxn(self);
    self->active_trxn->oMsglen = get_default_gtp(2, GTP2_RELEASE_ACCESS_BEARERS_REQ, &(self->active_trxn->oMsg));

    s11u_send(self);
}


void sendDownlinkDataNotificationAck(gpointer u){
    S11_user_t *self = (S11_user_t*)u;
    guint ienum = 0;
    union gtpie_member ie[8];
    memset(ie, 0, sizeof(union gtpie_member)*8);

    /*  Send Release Access Bearers Request to SGW*/
    /******************************************************************************/
    S11_TrxnT *t = self->active_trxn;

    t->oMsglen = get_default_gtp(2,  GTP2_DOWNLINK_DATA_NOTIFICATION_ACK, &(t->oMsg));

    /* Cause*/
    ie[ienum].tliv.i=0;
    ie[ienum].tliv.l=hton16(2);
    ie[ienum].tliv.t= GTPV2C_IE_CAUSE;
    bzero(ie[ienum].tliv.v,4);
    ie[ienum].tliv.v[0]=GTPV2C_CAUSE_REQUEST_ACCEPTED ;
    ie[ienum].tliv.v[1]=0; /* No Flags*/
    ienum++;

    gtp2ie_encaps(ie, ienum, &(t->oMsg), &(t->oMsglen));

    s11_send_resp(self);
}

const gboolean accepted(gpointer u){
    S11_user_t *self = (S11_user_t*)u;
    uint16_t vsize;
    uint8_t value[2];

    /* Cause*/
    gtp2ie_gettliv(self->active_trxn->ie, GTPV2C_IE_CAUSE, 0, value, &vsize);
    if(vsize=2){
        self->cause = value[0];
        return value[0]==GTPV2C_CAUSE_REQUEST_ACCEPTED;
    }else{
        log_msg(LOG_ERR, 0, "GTPv2 Cause IE Parse Error");
        return FALSE;
    }
}

const int cause(gpointer u){
    S11_user_t *self = (S11_user_t*)u;
    uint16_t vsize;
    uint8_t value[2];

    gtp2ie_gettliv(self->active_trxn->ie, GTPV2C_IE_CAUSE, 0, value, &vsize);
    if(vsize=2){
        return value[0];
    }else{
        log_msg(LOG_ERR, 0, "GTPv2 Cause IE Parse Error");
        return FALSE;
    }
}

void parseCtxRsp(gpointer u, GError **err){
    S11_user_t *self = (S11_user_t*)u;
    union gtpie_member *bearerCtxGroupIE[GTPIE_SIZE];
    uint8_t value[GTP2IE_MAX], value_bc[GTP2IE_MAX], addr[INET6_ADDRSTRLEN], ebi;
    uint16_t vsize = 0;
    uint32_t numIE;
    struct fteid_t fteid;
    struct in_addr s1uaddr;
    ESM_BearerContext bearer;

    memset(bearerCtxGroupIE, 0, sizeof(union gtpie_member*)*GTPIE_SIZE);
    memset(value, 0, GTP2IE_MAX * sizeof(uint8_t));

    /* F-TEID S11 (SGW)*/
    gtp2ie_gettliv(self->active_trxn->ie, GTPV2C_IE_FTEID, 0, value, &vsize);
    if(value!= NULL && vsize>0){
        /* memcpy(&(self->user->s11), value, vsize); */
        s11u_setS11fteid(self, value);
        log_msg(LOG_DEBUG, 0, "S11 Sgw teid = %x into", hton32(self->rTEID));
    }

    /* F-TEID S5 /S8 (PGW)*/
    gtp2ie_gettliv(self->active_trxn->ie, GTPV2C_IE_FTEID, 1, value, &vsize);
    if(value!= NULL && vsize>0){
        memcpy(&(self->s5s8), value, vsize);
        log_msg(LOG_DEBUG, 0, "S5/S8 Pgw teid = %x into", hton32(self->s5s8.teid));
    }

    /* PDN Address Allocation - PAA*/
    gtp2ie_gettliv(self->active_trxn->ie, GTPV2C_IE_PAA, 0, value, &vsize);
    if(value!= NULL && vsize>0){
        ePSsession_setPDNAddress(self->session, value, vsize);
        log_msg(LOG_DEBUG, 0, "PDN Address Allocated %s for IMSI: %" PRIu64"",
                ePSsession_getPDNAddrStr(self->session, addr, INET6_ADDRSTRLEN),
                emmCtx_getIMSI(self->emm));
    }
    /* APN Restriction*/

    vsize=0;
    /* Protocol Configuration Options PCO*/
    gtp2ie_gettliv(self->active_trxn->ie, GTPV2C_IE_PCO, 0, value, &vsize);
    if(value!= NULL && vsize>0){
        ePSsession_setPCO(self->session, value, vsize);
    }

    /* Bearer Context*/
    bearer = ePSsession_getDefaultBearer(self->session);
    gtp2ie_gettliv(self->active_trxn->ie, GTPV2C_IE_BEARER_CONTEXT, 0, value, &vsize);
    if(value!= NULL && vsize>0){
        gtp2ie_decaps_group(bearerCtxGroupIE, &numIE, value, vsize);

        /* EPS Bearer ID*/
        gtp2ie_gettliv(bearerCtxGroupIE, GTPV2C_IE_EBI, 0, value_bc, &vsize);
        ebi = esm_bc_getEBI(bearer);
        if(ebi != *value_bc){
            log_msg(LOG_ERR, 0, "Wrong EPC Bearer ID %u != %u",
                    ebi, *value_bc);
            return;
        }

        /* F-TEID S1-U (SGW)*/
        gtp2ie_gettliv(bearerCtxGroupIE, GTPV2C_IE_FTEID, 0, value_bc, &vsize);
        if(value_bc!= NULL && vsize>0){
            esm_bc_setS1uSGWfteid(bearer, value_bc, vsize);
            //log_msg(LOG_DEBUG, 0, "S1-u Sgw teid = %x, ip = %s", hton32(self->user->ebearer[0].s1u_sgw.teid), inet_ntoa(s1uaddr));
        }

        /* F-TEID S5/S8-U(PGW) */
        gtp2ie_gettliv(bearerCtxGroupIE, GTPV2C_IE_FTEID, 2, value_bc, &vsize);
        if(value_bc!= NULL && vsize>0){
            esm_bc_setS5S8uPGWfteid(bearer, value_bc, vsize);
            //log_msg(LOG_DEBUG, 0, "S5/S8 Pgw teid = %x into", hton32(self->user->ebearer[0].s5s8u.teid));
        }else{

        }
    }
    /*Recovery*/
    gtp2ie_gettliv(self->active_trxn->ie,  GTPV2C_IE_RECOVERY, 0, value, &vsize);
    if(vsize>0) S11_checkPeerRestart(self->s11,
                                     &self->rAddr, self->rAddrLen,
                                     value[0],
                                     self);
}

void parseModBearerRsp(gpointer u, GError **err){
    S11_user_t *self = (S11_user_t*)u;
    union gtpie_member *bearerCtxGroupIE[GTPIE_SIZE];
    uint8_t value[40];
    uint16_t vsize;
    uint32_t numIE;
    struct fteid_t fteid;
    ESM_BearerContext bearer;
    guint8 ebi;

    /*  TODO @Vicent Check message mandatory IE*/
    log_msg(LOG_DEBUG, 0, "Parsing Modify Bearer Req");

    /* Bearer Context*/
    gtp2ie_gettliv(self->active_trxn->ie, GTPV2C_IE_BEARER_CONTEXT, 0, value, &vsize);
    if(value!= NULL && vsize>0){
        gtp2ie_decaps_group(bearerCtxGroupIE, &numIE, value, vsize);

        /* EPS Bearer ID*/
        gtp2ie_gettliv(bearerCtxGroupIE, GTPV2C_IE_EBI, 0, value, &vsize);

        bearer = ePSsession_getDefaultBearer(self->session);
        ebi = esm_bc_getEBI(bearer);
        if(vsize != 1 && ebi != *value){
            g_set_error (err,
                         PARSE_ERROR,                 // error domain
                         1,            // error code
                         "EPC Bearer ID %u != %u received",
                         ebi, *value);
            return;
        }

        /* F-TEID S1-U (SGW)*/
        gtp2ie_gettliv(bearerCtxGroupIE, GTPV2C_IE_FTEID, 0, value, &vsize);
        if(value!= NULL && vsize>0){
            memcpy(&fteid, value, vsize);
            /* if(memcmp(&(self->user->ebearer[0].s1u_sgw), value, vsize) != 0){ */
            /*     g_set_error (err, */
            /*                  PARSE_ERROR,                 // error domain */
            /*                  1,            // error code */
            /*                  "S1-U SGW FEID not corresponds to the stored one"); */
            /*     return; */
            /* } */
        }
    }
}

void parseDelCtxRsp(gpointer u, GError **err){
    S11_user_t *self = (S11_user_t*)u;

    log_msg(LOG_DEBUG, 0, "Parsing Delete Session RSP.");

    /*  Delete node info*/
    self->rTEID = 0;
}


void s11u_setS11fteid(gpointer u, gpointer fteid_h){
    struct fteid_t* fteid = (struct fteid_t*) fteid_h;
    S11_user_t *self = (S11_user_t*)u;

    if(fteid->iface != S11S4_SGW)
        g_error("Unexpected interface type, S11S4_SGW expected");

    /*TODO validate addr*/

    self->rTEID = fteid->teid;
}
