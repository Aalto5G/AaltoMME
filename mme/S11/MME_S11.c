/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   MME_S11.c
 * @Author Vicent Ferrer
 * @date   March, 2013
 * @brief  MME S11 interface protocol state machine.
 *
 * This module implements the S11 interface state machine. The states are separated on STATE_* and TASK_MME_S11___*
 * The firsts ones implements the flow control and the second ones the processing
 */

//#include "MME_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include "signals.h"

#include "MME_S11.h"
#include "logmgr.h"
#include "nodemgr.h"
#include "storagesys.h"
#include "gtpie.h"
#include "MME.h"

#include "S11_FSMConfig.h"

typedef enum{
    S11_new_user_test,
    S11_attach,
    S11_ModifyBearer,
    S11_detach,
    S11_createIndirectDataForwardingTunnel,
    S11_handler_ready,
    S11_handler_error,
    S11_releaseAccessBearers,
} S11_Signal;

typedef void (*S11_STATE)(S11_user_t u, S11_Signal sig);

typedef struct{
    gpointer    mme;   /**< mme handler*/
    int         fd;    /**< file descriptor of the s11 server*/
    GHashTable *users; /**< s11 users by TEID*/
}S11_t;

void s11_accept(evutil_socket_t listener, short event, void *arg);

//TO REMOVE
struct t_process *S11_handler_create(struct t_engine_data *engine, struct t_process *owner);
//
/* ======================================================================
 * TASK Prototypes
 * ====================================================================== */


static void TASK_MME_S11___CreateContextResp(S11_user_t u, S11_Signal sig);

static void TASK_MME_S11___removeCtx(S11_user_t u, S11_Signal sig);

static void TASK_MME_S11___newCtx(S11_user_t u, S11_Signal sig);

static void TASK_MME_S11___processHandler(S11_user_t u, S11_Signal sig);

static void TASK_MME_S11___Forge_ModifyBearerReqAttach(S11_user_t u, S11_Signal sig);

static uint8_t TASK_MME_S11___validate_ModifyBearerReq(S11_user_t u, S11_Signal sig);

static void TASK_MME_S11___Forge_CreateIndirectDataForwardingTunnel(S11_user_t u, S11_Signal sig);

static uint8_t TASK_MME_S11___Validate_createIndirectDataForwardingTunnelRsp(S11_user_t u, S11_Signal sig);

static void TASK_MME_S11___handler_error(S11_user_t u, S11_Signal sig);

/* ======================================================================
 * S11 FSM State Prototypes
 * ====================================================================== */

/** @brief Attach State
 *  @param [in] signal
 *  @return 0 to free signal & send stored signals on process or 1 to save signal on process signal queue.
 *
 *  This state is part of the attach procedure. See 3GPP TS 23.401 Figure 5.3.2.1-1
 *
 *  If there are active bearer contexts in the new MME for this particular UE (i.e.
 *  the UE re-attaches to the same MME without having properly detached before),
 *  the new MME deletes these bearer contexts by sending Delete Session Request (LBI) messages to
 *  the GWs involved. The GWs acknowledge with Delete Session Response (Cause) message.
 *  If a PCRF is deployed, the PDN GW employs an IP-CAN Session Termination procedure to indicate that
 *  resources have been released.
 */
static int STATE_attach(S11_user_t u, S11_Signal sig);

/* ======================================================================*/

static gpointer s11_newSession(S11_t *s11, struct user_ctx_t *user){
    gpointer u = s11u_newUser(user);
    g_hash_table_insert(s11->users, s11u_getTEIDp(u), u);
    return u;
}

void s11_deleteSession(gpointer s11_h, gpointer u){
	S11_t *self = (S11_t *) s11_h;
	g_hash_table_remove(self->users, s11u_getTEIDp(u));
}


gpointer s11_init(gpointer mme){
    S11_t *self = g_new(S11_t, 1);

    self->mme = mme;

    s11ConfigureFSM();

    /*Init S11 server*/
    self->fd =init_udp_srv(GTP2C_PORT);
    log_msg(LOG_INFO, 0, "Open S11 server on file descriptor %d, port %d",
            self->fd, GTP2C_PORT);

    mme_registerRead(self->mme, self->fd, s11_accept, self);

    self->users = g_hash_table_new_full( g_int_hash,
                                         g_int_equal,
                                         NULL,
                                         (GDestroyNotify)s11u_freeUser);
    return self;
}

void s11_free(gpointer s11_h){
    S11_t *self = (S11_t *) s11_h;

    g_hash_table_destroy(self->users);
    mme_deregisterRead(self->mme, self->fd);
    close(self->fd);
    s11DestroyFSM();
    g_free(self);
}

const int s11_fg(gpointer s11_h){
    S11_t *self = (S11_t *) s11_h;
    return self->fd;
}


void s11_accept(evutil_socket_t listener, short event, void *arg){

    uint32_t teid;
    struct SessionStruct_t *session;
    S11_Signal *output;
    struct t_message *msg;

    struct sockaddr_in peer;
    socklen_t peerlen;

    S11_t *self = (S11_t *) arg;
    S11_user_t * u;
    const char str[INET6_ADDRSTRLEN];

    msg = newMsg();
    msg->packet.gtp.flags=0x0;
    gtp2_recv(listener, &(msg->packet.gtp), &(msg->length),
              &peer, &peerlen);

    switch(peer->sin_family){
    case AF_INET:
	    inet_ntop(AF_INET, &(peer.sin_addr), msg->srcAddr, INET_ADDRSTRLEN);
    /* case AF_INET6: */
	/*     inet_ntop(AF_INET6, &(peer.sin6_addr), str, INET6_ADDRSTRLEN); */
    }

    msg->peer.fd = listener;

    if(msg->packet.gtp.gtp2s.h.type<4){
        /* TODO @Vicent:
           Manage echo request, echo response or version not suported*/
        log_msg(LOG_INFO, 0, "S11 recv echo request,"
                " echo response or version not suported msg");
        print_packet(&(msg->packet), msg->length);
        return;
    }

    teid = ntoh32(msg->packet.gtp.gtp2l.h.tei);

    if (! g_hash_table_lookup_extended(self->users, &teid, NULL,
                                       (gpointer*)&u)){
        log_errpack(LOG_INFO, 0, (struct sockaddr_in*)&(msg->peer.peerAddr),
                    &(msg->packet), msg->length,
                    "S11 received packet with unknown TEID (%#X),"
                    " ignoring packet", &teid);
        return;
    }

    if (s11u_hasPendingResp( u)){
        log_msg(LOG_DEBUG, 0, "Received pending S11 reply");
        processMsg(u, msg);
    }
    else{
        log_msg(LOG_DEBUG, 0, "Received new S11 request");
        processMsg(u, msg);
    }
    
    freeMsg(msg);
}

/* ======================================================================
 * S11 FSM State Implementations
 * ====================================================================== */

static int STATE_Handle_Recv_Msg(S11_user_t u, S11_Signal sig)
{

    INIT_TIME_MEASUREMENT_ENVIRONMENT

    S11_Signal *output;
    log_msg(LOG_DEBUG, 0, "Enter STATE_Handle_Recv_Msg.");

    switch(sig){
    case S11_attach:
        TASK_MME_S11___processHandler(signal);

        if(((struct t_message *)signal->data)->packet.gtp.gtp2l.h.type == GTP2_DELETE_SESSION_RSP){
            PROC->next_state = STATE_attach;
            signal_send(signal);
        }else{
            /*  Recover old process and old signal to continue the flow to original State Machine*/
            run_parent(signal);
        }
        break;
    case S11_ModifyBearer:
        MME_MEASURE_PROC_TIME
        log_msg(LOG_DEBUG, 0, "S11: Recv Modify Bearer Resp - time = %u us", SELF_ON_SIG->procTime);

        TASK_MME_S11___validate_ModifyBearerReq(signal);
        run_parent(signal);
        break;
    case S11_handler_ready:
        /*New request received*/
        log_msg(LOG_WARNING, 0, "New Request detected. Not implemented yet");
        break;
    case S11_handler_error:
        TASK_MME_S11___handler_error(signal);
        break;
    case S11_detach:
        if(((struct t_message *)signal->data)->packet.gtp.gtp2l.h.type == GTP2_DELETE_SESSION_RSP){
            TASK_MME_S11___processHandler(signal);
            run_parent(signal);
        }
        break;
    case S11_createIndirectDataForwardingTunnel:
        if(((struct t_message *)signal->data)->packet.gtp.gtp2l.h.type == GTP2_CREATE_INDIRECT_DATA_FORWARDING_TUNNEL_RSP){
            if(TASK_MME_S11___Validate_createIndirectDataForwardingTunnelRsp(signal) == 0){
                run_parent(signal);
            }else{
                /* ERROR*/
            }
        }
        break;
    case S11_releaseAccessBearers:
        run_parent(signal);

    default:
        break;
    }
    return 0;
}

static int STATE_Delete_User_Session(S11_user_t u, S11_Signal sig)
{

    INIT_TIME_MEASUREMENT_ENVIRONMENT

    log_msg(LOG_DEBUG, 0, "Enter STATE_Delete_User_Session.");

    TASK_MME_S11___removeCtx(signal);

    MME_MEASURE_PROC_TIME
    log_msg(LOG_DEBUG, 0, "S11: Sent Delete Session Request - time = %u us", SELF_ON_SIG->procTime);

    PROC->next_state = STATE_Handle_Recv_Msg;

    /* Add session to pendent request*/
    if( addToPendingResponse(PDATA) == 0){
        log_msg(LOG_DEBUG, 0, "Stored session %x", PDATA);
        return 1;
    }
    else{
        log_msg(LOG_WARNING, 0, "Is this a retransmission? Not implemented");
    }
    return 0;
}

static int STATE_attach(S11_user_t u, S11_Signal sig){
    S11_Signal *output, *contCreateUserctx;
    struct t_process *old;
    struct user_ctx_t *user = NULL;

    INIT_TIME_MEASUREMENT_ENVIRONMENT

    log_msg(LOG_DEBUG, 0, "Enter");

    switch (sig)
    {
    case S11_attach:
        /*  Look for active bearer contexts */
        user = get_user_ctx_imsi(PDATA->user_ctx->imsi);
        if(!user){
            /*log_msg(LOG_DEBUG, 0, "Test1 teid = %d, count = %d", ((struct SessionStruct_t*)signal->processTo->data)->user_ctx->S11MMETeid, count_users());*/
            user = get_user_ctx_teid(PDATA->user_ctx->S11MMETeid);
            if(user){
                log_msg(LOG_DEBUG, 0, "User Context (imsi %llu, MMETEID = %d) exists, removing before the new attach, count %d", user->imsi, user->S11MMETeid, count_users());
            }
        }
        if(user){
            if(user->ebearer[0].id >5){
                log_msg(LOG_DEBUG, 0, "User Context (imsi %llu, MMETEID = %d) exists, removing before the new attach, count %d", user->imsi, user->S11MMETeid, count_users());
                PROC->next_state = STATE_Delete_User_Session;
                signal_send(signal);
            }else{
                log_msg(LOG_DEBUG, 0, "Bearer Context doesn't exist, creating a new one.");

                TASK_MME_S11___newCtx(signal);

                MME_MEASURE_PROC_TIME
                log_msg(LOG_DEBUG, 0, "S11: Sent Create Session Request - time = %u us", SELF_ON_SIG->procTime);

                PROC->next_state = STATE_Handle_Recv_Msg;
                /* Add session to pendent request*/
                if( addToPendingResponse(PDATA) == 0){
                    return 1; /*Save signal on process*/
                }
                else{
                    log_msg(LOG_WARNING, 0, "Couldn't store session %x, Is this a retransmission? Not implemented", PDATA);
                }
            }

        }else{
            log_msg(LOG_DEBUG, 0, "User Context doesn't exist, creating a new one.");
            /*  Add User context to storage*/
            store_user_ctx(PDATA->user_ctx);
            s6a_UpdateLocation(SELF_ON_SIG->s6a, user,
                              (void(*)(gpointer)) sendFirstStoredS11_Signal,
                              (gpointer)PDATA->sessionHandler);
            return 1;      /* S11_Signal to continue with the context recreate after S6*/
        }
        break;
    case S11_ModifyBearer:
        TASK_MME_S11___Forge_ModifyBearerReqAttach(signal);

        MME_MEASURE_PROC_TIME
        log_msg(LOG_DEBUG, 0, "S11: Sent Modify Bearer Req - time = %u us", SELF_ON_SIG->procTime);

        PROC->next_state = STATE_Handle_Recv_Msg;
        addToPendingResponse(PDATA);
        return 1;
    default:
        return(0);
    }

    return 0;   /*free signal & send stored signals on process*/
    /*return 1;   Save signal on process*/
}

static int STATE_CreateIndirectDataForwardingTunnel(S11_user_t u, S11_Signal sig)
{
    log_msg(LOG_DEBUG, 0, "Enter");

    TASK_MME_S11___Forge_CreateIndirectDataForwardingTunnel(signal);
    PROC->next_state = STATE_Handle_Recv_Msg;

    /* Add session to pendent request*/
    if( addToPendingResponse(PDATA) == 0){
        log_msg(LOG_DEBUG, 0, "Stored session %x", PDATA);
        return 1;
    }
    else{
        log_msg(LOG_WARNING, 0, "Is this a retransmission? Not implemented");
    }
    return 0;
}

static int STATE_ReleaseAccessBearers(S11_user_t u, S11_Signal sig)
{
    log_msg(LOG_DEBUG, 0, "Enter");

    /*TASK_MME_S11___Forge_CreateIndirectDataForwardingTunnel(signal);
    PROC->next_state = STATE_Handle_Recv_Msg;
    */
    /* Add session to pendent request*//*
    if( addToPendingResponse(PDATA) == 0){
        log_msg(LOG_DEBUG, 0, "Stored session %x", PDATA);
        return 1;
    }
    else{
        log_msg(LOG_WARNING, 0, "Is this a retransmission? Not implemented");
    }*/
    /*TODO Not implemented*/
    run_parent(signal);
    return 0;
}


/* ======================================================================
 * TASK Prototypes Implementations
 * ====================================================================== */


void TASK_MME_S11___handler_error(S11_user_t u, S11_Signal sig){

}

/* void TASK_MME_S11___CreateContextResp(S11_user_t u, S11_Signal sig){ */

/*     struct t_message *msg; */
/*     union gtpie_member *ie[GTPIE_SIZE], *bearerCtxGroupIE[GTPIE_SIZE]; */
/*     uint8_t value[40]; */
/*     uint16_t vsize; */
/*     uint32_t numIE; */
/*     struct fteid_t fteid; */
/*     struct in_addr s1uaddr; */

/*     msg = (struct t_message *)signal->data; */

/*     /\*  TODO @Vicent Check message mandatory IE*\/ */
/*     log_msg(LOG_DEBUG, 0, "Parsing Create Session Resp."); */
/*     gtp2ie_decap(ie, &(msg->packet), msg->length); */
/*     if(ntoh32(msg->packet.gtp.gtp2l.h.tei) != PDATA->user_ctx->S11MMETeid){ */
/*         log_msg(LOG_WARNING, 0, "TEID incorrect 0x%x != 0x%x", ntoh32(msg->packet.gtp.gtp2l.h.tei), PDATA->user_ctx->S11MMETeid); */
/*         return; */
/*     } */

/*     /\* Cause*\/ */
/*     gtp2ie_gettliv(ie, GTPV2C_IE_CAUSE, 0, value, &vsize); */
/*     if(value!= NULL && vsize>0){ */
/*         if(value[0]!=GTPV2C_CAUSE_REQUEST_ACCEPTED){ */
/*             log_msg(LOG_WARNING, 0, "Create Session request rejected Cause %d", value[0]); */
/*             return; */
/*         } */
/*     } */

/*     /\* F-TEID S11 (SGW)*\/ */
/*     gtp2ie_gettliv(ie, GTPV2C_IE_FTEID, 0, value, &vsize); */
/*     if(value!= NULL && vsize>0){ */
/*         memcpy(&(PDATA->user_ctx->s11), value, vsize); */
/*         log_msg(LOG_DEBUG, 0, "S11 Sgw teid = %x into", hton32(PDATA->user_ctx->s11.teid)); */
/*     } */


/*     /\* F-TEID S5 /S8 (PGW)*\/ */
/*     gtp2ie_gettliv(ie, GTPV2C_IE_FTEID, 1, value, &vsize); */
/*     if(value!= NULL && vsize>0){ */
/*         memcpy(&(PDATA->user_ctx->s5s8), value, vsize); */
/*         log_msg(LOG_DEBUG, 0, "S5/S8 Pgw teid = %x into", hton32(PDATA->user_ctx->s5s8.teid)); */
/*     } */

/*     /\* PDN Address Allocation - PAA*\/ */
/*     gtp2ie_gettliv(ie, GTPV2C_IE_PAA, 0, value, &vsize); */
/*     if(value!= NULL && vsize>0){ */
/*         memcpy(&(PDATA->user_ctx->pAA), value, vsize); */
/*         log_msg(LOG_DEBUG, 0, "PDN Allocated Addr type %u", PDATA->user_ctx->pAA.type); */
/*     } */
/*     /\* APN Restriction*\/ */

/*     vsize=0; */
/*     /\* Protocol Configuration Options PCO*\/ */
/*     gtp2ie_gettliv(ie, GTPV2C_IE_PCO, 0, value, &vsize); */
/*     if(value!= NULL && vsize>0){ */
/*         memcpy(&(PDATA->user_ctx->pco[2]), value, vsize); */
/*         PDATA->user_ctx->pco[1]= (uint8_t) ntoh16(vsize); */
/*         log_msg(LOG_DEBUG, 0, "PDN Allocated Addr type %u", PDATA->user_ctx->pAA.type); */
/*     } */

/*     /\* Bearer Context*\/ */
/*     gtp2ie_gettliv(ie, GTPV2C_IE_BEARER_CONTEXT, 0, value, &vsize); */
/*     if(value!= NULL && vsize>0){ */
/*         gtp2ie_decaps_group(bearerCtxGroupIE, &numIE, value, vsize); */

/*         /\* EPS Bearer ID*\/ */
/*         gtp2ie_gettliv(bearerCtxGroupIE, GTPV2C_IE_EBI, 0, value, &vsize); */
/*         memcpy(&(PDATA->user_ctx->ebearer[0].id), value, vsize); */
/*         log_msg(LOG_DEBUG, 0, "EPC Bearer ID = %u", PDATA->user_ctx->ebearer[0].id); */

/*         /\* F-TEID S1-U (SGW)*\/ */
/*         gtp2ie_gettliv(bearerCtxGroupIE, GTPV2C_IE_FTEID, 0, value, &vsize); */
/*         if(value!= NULL && vsize>0){ */
/*             memcpy(&(PDATA->user_ctx->ebearer[0].s1u_sgw), value, vsize); */
/*             s1uaddr.s_addr = PDATA->user_ctx->ebearer[0].s1u_sgw.addr.addrv4; */
/*             log_msg(LOG_DEBUG, 0, "S1-u Sgw teid = %x, ip = %s", hton32(PDATA->user_ctx->ebearer[0].s1u_sgw.teid), inet_ntoa(s1uaddr)); */
/*         } */

/*         /\* F-TEID S5/S8-U(PGW)*\/ */
/*         gtp2ie_gettliv(bearerCtxGroupIE, GTPV2C_IE_FTEID, 1, value, &vsize); */
/*         if(value!= NULL && vsize>0){ */
/*             memcpy(&(PDATA->user_ctx->ebearer[0].s5s8u), value, vsize); */
/*             log_msg(LOG_DEBUG, 0, "S5/S8 Pgw teid = %x into", hton32(PDATA->user_ctx->ebearer[0].s5s8u.teid)); */
/*         }else{ */

/*         } */
/*     } */

/*     /\* Recovery *\/ */
/*     /\*gtp2ie_gettliv(ie, GTPV2C_IE_RECOVERY, 0, value, &vsize); */
/*     if(value!= NULL && vsize>0){ */
/*         log_msg(LOG_DEBUG, 0, "Recovery %d", value[0]); */
/*         return; */
/*     }*\/ */


/* } */

void TASK_MME_S11___removeCtx(S11_user_t u, S11_Signal sig){
    /*  Send  Delete Session Request to SGW */
    struct user_ctx_t *user;
    union gtp_packet packet;
    struct nodeinfo_t sgw;
    struct fteid_t  fteid;
    struct qos_t    qos;
    size_t peerlen;
    struct sockaddr_in  *peer;
    struct in_addr sgwAddr;
    union gtpie_member ie[13], ie_bearer_ctx[3];
    int hlen, sock, a;
    uint32_t length, ielen;
    uint8_t bytefield[30];

    log_msg(LOG_DEBUG, 0, "Enter TASK_MME_S11___removeCtx , sending Delete Session Request");

    /*Get User context*/
    user = get_user_ctx_imsi(((struct SessionStruct_t*)signal->processTo->data)->user_ctx->imsi);

    /*  Send Delete Session Request to SGW*/
    /******************************************************************************/

    length = get_default_gtp(2, GTP2_DELETE_SESSION_REQ, &packet);

    /*  EPS Bearer ID (EBI) to be removed*/
    ie[0].tliv.i=0;
    ie[0].tliv.l=hton16(1);
    ie[0].tliv.t=GTPV2C_IE_EBI;
    /*ie[0].tliv.v[0]=user.ebi; *//*Future*/
    ie[0].tliv.v[0]=0x05; /*EBI = 5,  EBI > 4, see 3GPP TS 24.007 11.2.3.1.5  EPS bearer identity */

    gtp2ie_encaps(ie, 1, &packet, &length);

    /*Packet header modifications*/
    packet.gtp2l.h.seq = hton24(getNextSeq(SELF_ON_SIG));
    packet.gtp2l.h.tei = user->s11.teid;

    /*Debug information*/
    /*printf("packet length %d\n",length);
    print_packet(&packet, length);*/

    /*******************************************************************/
    /*Get SGW addr*/
    /*sgwAddr.s_addr = user->s11.addr.addrv4;*/
    /*getNodeByAddr4(&sgwAddr, SGW, &sgw);*/
    sock = PDATA->s11->fd;
    peer = (struct sockaddr_in *)&(PDATA->s11->peerAddr);
    peer->sin_family = AF_INET;
    peer->sin_port = htons(GTP2C_PORT);
    /*inet_pton(AF_INET, "10.11.0.142", &(sgw.addrv4));*/
    /*user->s11.addr.addrv4;*/
    peer->sin_addr.s_addr = user->s11.addr.addrv4;
    PDATA->s11->socklen = sizeof(struct sockaddr_in);

    peerlen = PDATA->s11->socklen;

    if (sendto(sock, &packet, length, 0, (struct sockaddr *)peer, peerlen) < 0) {
        log_errpack(LOG_ERR, errno, (struct sockaddr_in *)peer, &packet, length,
                "Sendto(fd=%d, msg=%lx, len=%d) failed", sock, (unsigned long) &packet, length);
        return;
    }
    /*******************************************************************/

};

/* void TASK_MME_S11___newCtx(S11_user_t u, S11_Signal sig){ */
/*     /\* TODO @Vicent */
/*      * The new MME selects a Serving GW as described in clause 4.3.8.2 3GPP 23.401 on Serving GW selection function and */
/*      * allocates an EPS Bearer Identity for the Default Bearer associated with the UE. */
/*      * Then it sends a Create Session Request (IMSI, MSISDN, MME TEID for control plane, PDN GW address, */
/*      * PDN Address, APN, RAT type, Default EPS Bearer QoS, PDN Type, APN-AMBR, EPS Bearer Identity, */
/*      * Protocol Configuration Options, Handover Indication, ME Identity (IMEISV), User Location Information (ECGI), */
/*      * UE Time Zone, User CSG Information, MS Info Change Reporting support indication, Selection Mode, */
/*      * Charging Characteristics, Trace Reference, Trace Type, Trigger Id, OMC Identity, Maximum APN Restriction, */
/*      * Dual Address Bearer Flag, the Protocol Type over S5/S8, Serving Network) message to the selected Serving GW. */
/*      * User CSG Information includes CSG ID, access mode and CSG membership indication. *\/ */

/*     struct user_ctx_t *user; */
/*     union gtp_packet packet; */
/*     struct fteid_t  fteid; */
/*     struct qos_t    *qos; */
/*     size_t peerlen; */
/*     struct sockaddr_in  *peer; */
/*     union gtpie_member ie[13], ie_bearer_ctx[3]; */
/*     int hlen, sock, a; */
/*     uint32_t length, ielen, ienum=0; */
/*     uint8_t bytefield[30], *tmp; */
/*     struct nodeinfo_t pgwInfo; */

/*     log_msg(LOG_DEBUG, 0, "Enter"); */
/*     /\*  Send Create Context Request to SGW*\/ */
/*     /\******************************************************************************\/ */
/*     user = PDATA->user_ctx; */
/*     qos = &(user->ebearer->qos); */

/*     length = get_default_gtp(2, GTP2_CREATE_SESSION_REQ, &packet); */

/*     /\*IMSI*\/ */
/*     ie[ienum].tliv.i=0; */
/*     ie[ienum].tliv.t=GTPV2C_IE_IMSI; */
/*     dec2tbcd(ie[ienum].tliv.v, &ielen, PDATA->user_ctx->imsi); */
/*     ie[ienum].tliv.l=hton16(ielen); */
/*     ienum++; */
/*     /\*MSISDN*\/ */
/*     ie[ienum].tliv.i=0; */
/*     ie[ienum].tliv.t=GTPV2C_IE_MSISDN; */
/*     dec2tbcd(ie[ienum].tliv.v, &ielen, PDATA->user_ctx->msisdn); */
/*     ie[ienum].tliv.l=hton16(ielen); */
/*     ienum++; */
/*     /\*MEI*\/ */
/*     ie[ienum].tliv.i=0; */
/*     ie[ienum].tliv.t=GTPV2C_IE_MEI; */
/*     dec2tbcd(ie[ienum].tliv.v, &ielen, PDATA->user_ctx->imsi); */
/*     ie[ienum].tliv.l=hton16(ielen); */
/*     ienum++; */
/*     /\*RAT type*\/ */
/*     ie[ienum].tliv.i=0; */
/*     ie[ienum].tliv.l=hton16(1); */
/*     ie[ienum].tliv.t=GTPV2C_IE_RAT_TYPE; */
/*     ie[ienum].tliv.v[0]=6;                  /\*Type 6= EUTRAN*\/ */
/*     ienum++; */
/*     /\*F-TEID*\/ */
/*     ie[ienum].tliv.i=0; */
/*     ie[ienum].tliv.l=hton16(9); */
/*     ie[ienum].tliv.t=GTPV2C_IE_FTEID; */
/*     fteid.ipv4=1; */
/*     fteid.ipv6=0; */
/*     fteid.iface= hton8(S11_MME); */
/*     fteid.teid = hton32(PDATA->user_ctx->S11MMETeid); */
/*     fteid.addr.addrv4 = SELF_ON_SIG->ipv4; */
/*     ie[ienum].tliv.l=hton16(FTEID_IP4_SIZE); */
/*     memcpy(ie[ienum].tliv.v, &fteid, FTEID_IP4_SIZE); */
/*     ienum++; */
/*     /\*F-TEID PGW S5/S8 Address for Control Plane or PMIP *\/ */
/*     ie[ienum].tliv.i=1; */
/*     ie[ienum].tliv.l=hton16(FTEID_IP4_SIZE); */
/*     ie[ienum].tliv.t=GTPV2C_IE_FTEID; */
/*     fteid.ipv4=1; */
/*     fteid.ipv6=0; */
/*     fteid.iface= hton8(S5S8C_PGW); */
/*     fteid.teid = hton32(0); */
/*     getNode(&pgwInfo, PGW, PDATA->user_ctx); */
/*     fteid.addr.addrv4 = pgwInfo.addrv4.s_addr; */
/*     memcpy(ie[ienum].tliv.v, &fteid, FTEID_IP4_SIZE); */
/*     ienum++; */
/*     /\*APN*\/ */
/*     ie[ienum].tliv.i=0; */
/*     ie[ienum].tliv.l=hton16(strlen(user->aPname)); */
/*     ie[ienum].tliv.t=GTPV2C_IE_APN; */
/*     sprintf(ie[ienum].tliv.v, user->aPname, strlen(user->aPname)); */
/*     ienum++; */

/*     /\*PAA*\/ */
/*     ie[ienum].tliv.i=0; */
/*     ie[ienum].tliv.l=hton16(5); */
/*     ie[ienum].tliv.t=GTPV2C_IE_PAA; */
/*     bytefield[0]=0x01;  /\*PDN Type  IPv4 *\/ */
/*     memset(bytefield+1, 0, 4);   /\*IP = 0.0.0.0*\/ */
/*     memcpy(ie[ienum].tliv.v, bytefield, 5); */
/*     ienum++; */
/*     /\*Serving Network*\/ */
/*     ie[ienum].tliv.i=0; */
/*     ie[ienum].tliv.l=hton16(3); */
/*     ie[ienum].tliv.t=GTPV2C_IE_SERVING_NETWORK; */
/*     memcpy(ie[ienum].tliv.v, PDATA->user_ctx->tAI.sn, 3); */
/*     ienum++; */
/*     /\*PDN type*\/ */
/*     ie[ienum].tliv.i=0; */
/*     ie[ienum].tliv.l=hton16(1); */
/*     ie[ienum].tliv.t=GTPV2C_IE_PDN_TYPE; */
/*     bytefield[0]=user->pdn_type; /\* PDN type IPv4*\/ */
/*     memcpy(ie[ienum].tliv.v, bytefield, 1); */
/*     ienum++; */
/*     /\*APN restriction*\/ */
/*     ie[ienum].tliv.i=0; */
/*     ie[ienum].tliv.l=hton16(1); */
/*     ie[ienum].tliv.t=GTPV2C_IE_APN_RESTRICTION; */
/*     bytefield[0]=0x00; /\* APN restriction*\/ */
/*     memcpy(ie[ienum].tliv.v, bytefield, 1); */
/*     ienum++; */
/*     /\*Selection Mode*\/ */
/*     ie[ienum].tliv.i=0; */
/*     ie[ienum].tliv.l=hton16(1); */
/*     ie[ienum].tliv.t=GTPV2C_IE_SELECTION_MODE; */
/*     bytefield[0]=0x01; /\* Selection Mode*\/ */
/*     memcpy(ie[ienum].tliv.v, bytefield, 1); */
/*     ienum++; */

/*     /\*Protocol Configuration Options*\/ */
/*     if(user->pco[0]==0x27){ */
/*         ie[ienum].tliv.i=0; */
/*         ie[ienum].tliv.l=hton16(user->pco[1]); */
/*         ie[ienum].tliv.t=GTPV2C_IE_PCO; */
/*         tmp = user->pco+2; */
/*         memcpy(ie[ienum].tliv.v, tmp, user->pco[1]); */
/*         ienum++; */
/*     } */
/*     /\*Bearer contex*\/ */
/*         /\*EPS Bearer ID *\/ */
/*         ie_bearer_ctx[0].tliv.i=0; */
/*         ie_bearer_ctx[0].tliv.l=hton16(1); */
/*         ie_bearer_ctx[0].tliv.t=GTPV2C_IE_EBI; */
/*         ie_bearer_ctx[0].tliv.v[0]=0x05; /\*EBI = 5,  EBI > 4, see 3GPP TS 24.007 11.2.3.1.5  EPS bearer identity *\/ */
/*         /\* Bearer QoS *\/ */
/*         ie_bearer_ctx[1].tliv.i=0; */
/*         ie_bearer_ctx[1].tliv.l=hton16(sizeof(struct qos_t)); */
/*         ie_bearer_ctx[1].tliv.t=GTPV2C_IE_BEARER_LEVEL_QOS; */
/*         memcpy(ie_bearer_ctx[1].tliv.v, qos, sizeof(struct qos_t)); */
/*         /\*EPS Bearer TFT *\/ */
/*         /\*ie_bearer_ctx[2].tliv.i=0; */
/*         ie_bearer_ctx[2].tliv.l=hton16(3); */
/*         ie_bearer_ctx[2].tliv.t=GTPV2C_IE_BEARER_TFT; */
/*         bytefield[0]=0x01; */
/*         bytefield[1]=0x01; */
/*         bytefield[2]=0x01; */
/*         memcpy(ie_bearer_ctx[2].tliv.v, bytefield, 3); */
/*     gtp2ie_encaps_group(GTPV2C_IE_BEARER_CONTEXT, 0, &ie[12], ie_bearer_ctx, 3);*\/ */
/*     gtp2ie_encaps_group(GTPV2C_IE_BEARER_CONTEXT, 0, &ie[ienum], ie_bearer_ctx, 2); */
/*     ienum++; */
/*     gtp2ie_encaps(ie, ienum, &packet, &length); */

/*     /\*Packet header modifications*\/ */
/*     packet.gtp2l.h.seq = hton24(getNextSeq(SELF_ON_SIG)); */
/*     packet.gtp2l.h.tei = user->s11.teid; */

/*     /\*Debug information*\/ */
/*     /\*printf("packet length %d\n",length); */
/*     print_packet(&packet, length);*\/ */

/*     /\*******************************************************************\/ */
/*     /\*Get SGW addr*\/ */
/*     struct nodeinfo_t sgw; */
/*     getNode(&sgw, SGW, user); */
/*     sock = PDATA->s11->fd; */
/*     peer = (struct sockaddr_in *)&(PDATA->s11->peerAddr); */
/*     peer->sin_family = AF_INET; */
/*     peer->sin_port = htons(GTP2C_PORT); */
/*     peer->sin_addr = sgw.addrv4; */
/*     PDATA->s11->socklen = sizeof(struct sockaddr_in); */

/*     peerlen = PDATA->s11->socklen; */

/*     if (sendto(sock, &packet, length, 0, (struct sockaddr *)peer, peerlen) < 0) { */
/*         log_errpack(LOG_ERR, errno, (struct sockaddr_in *)peer, &packet, length, */
/*                 "Sendto(fd=%d, msg=%lx, len=%d) failed", sock, (unsigned long) &packet, length); */
/*         return; */
/*     } */
/*     /\*******************************************************************\/ */

/*     /\*  Add User context to storage*\/ */
/*     /\*store_user_ctx(user);*\/ */

/* }; */

void TASK_MME_S11___processHandler(S11_user_t u, S11_Signal sig){

    struct t_message *msg;
    union gtpie_member *ie[GTPIE_SIZE];
    uint8_t value[20];
    uint16_t vsize;
    struct fteid_t fteid;

    msg = (struct t_message *)signal->data;

    if(msg == NULL)
        log_msg(LOG_ERR, 0, "Message not stored correctly");
    log_msg(LOG_DEBUG, 0, "TASK_MME_S11___processHandler Received Response message type: %d", msg->packet.gtp.gtp2l.h.type);

    /*removePendentResponse(PDATA);     *//*  Remove response from hash table*/

    switch(msg->packet.gtp.gtp2l.h.type){
    case GTP2_CREATE_SESSION_RSP:
        TASK_MME_S11___CreateContextResp(signal);

        break;
    case GTP2_DELETE_SESSION_RSP:
        /*  TODO @Vicent Check message fields*/
        log_msg(LOG_DEBUG, 0, "Parsing Delete Session RSP.");
        gtp2ie_decap(ie, &(msg->packet), msg->length);
        if(ntoh32(msg->packet.gtp.gtp2l.h.tei) != PDATA->user_ctx->S11MMETeid){
            log_msg(LOG_WARNING, 0, "TEID incorrect 0x%x != 0x%x", ntoh32(msg->packet.gtp.gtp2l.h.tei), PDATA->user_ctx->S11MMETeid);
            return;
        }

        gtp2ie_gettliv(ie, GTPV2C_IE_CAUSE, 0, value, &vsize);
        if(vsize>0){
            if(value[0]!=GTPV2C_CAUSE_REQUEST_ACCEPTED){
                log_msg(LOG_WARNING, 0, "Create Session request rejected Cause %d", value[0]);
                return;
            }
        }

        /*  Delete node info*/
        delete_user_ctx_teid(ntoh32(msg->packet.gtp.gtp2l.h.tei));
        PDATA->user_ctx->s11.teid = hton32(0);


        break;
    default:
        log_msg(LOG_DEBUG, 0, "TASK_MME_S11___handler_ready Message processing not implemented for this message type");
        break;
    }
};

/* void TASK_MME_S11___Forge_ModifyBearerReqAttach(S11_user_t u, S11_Signal sig){ */
/*     struct user_ctx_t *user; */
/*     union gtp_packet packet; */
/*     struct fteid_t  fteid; */
/*     size_t peerlen; */
/*     struct sockaddr_in  *peer; */
/*     union gtpie_member ie[13], ie_bearer_ctx[3]; */
/*     int hlen, sock, a; */
/*     uint32_t length, fteid_size; */

/*     log_msg(LOG_DEBUG, 0, "Enter"); */
/*     /\*  Send Create Context Request to SGW*\/ */
/*   /\******************************************************************************\/ */
/*     user = PDATA->user_ctx; */

/*     length = get_default_gtp(2, GTP2_MODIFY_BEARER_REQ, &packet); */

/*     /\*F-TEID*\/ */
/*     ie[0].tliv.i=0; */
/*     ie[0].tliv.t=GTPV2C_IE_FTEID; */
/*     ie[0].tliv.l=hton16(FTEID_IP4_SIZE); */
/*     fteid.ipv4=1; */
/*     fteid.ipv6=0; */
/*     fteid.iface= hton8(S11_MME); */
/*     fteid.teid = hton32(PDATA->user_ctx->S11MMETeid); */
/*     fteid.addr.addrv4 = SELF_ON_SIG->ipv4; */
/*     memcpy(ie[0].tliv.v, &fteid, FTEID_IP4_SIZE); */

/*     /\*Bearer contex*\/ */
/*         /\*EPS Bearer ID *\/ */
/*         ie_bearer_ctx[0].tliv.i=0; */
/*         ie_bearer_ctx[0].tliv.l=hton16(1); */
/*         ie_bearer_ctx[0].tliv.t=GTPV2C_IE_EBI; */
/*         ie_bearer_ctx[0].tliv.v[0]=user->ebearer[0].id; */
/*         /\* fteid S1-U eNB*\/ */
/*         memcpy(&fteid, &(user->ebearer[0].s1u_eNB), sizeof(struct fteid_t)); */
/*         ie_bearer_ctx[1].tliv.i=0; */
/*         ie_bearer_ctx[1].tliv.t=GTPV2C_IE_FTEID; */
/*         if(fteid.ipv4 == 1 && fteid.ipv6 == 0){ */
/*             fteid_size = FTEID_IP4_SIZE; */
/*         }else if (fteid.ipv4 == 0 && fteid.ipv6 == 1){ */
/*             fteid_size = FTEID_IP6_SIZE; */
/*         }else{ */
/*             fteid_size = FTEID_IP46_SIZE; */
/*         } */
/*         ie_bearer_ctx[1].tliv.l=hton16(fteid_size); */
/*         memcpy(ie_bearer_ctx[1].tliv.v, &fteid, fteid_size); */
/*     gtp2ie_encaps_group(GTPV2C_IE_BEARER_CONTEXT, 0, &ie[1], ie_bearer_ctx, 2); */
/*     gtp2ie_encaps(ie, 2, &packet, &length); */

/*     /\*Packet header modifications*\/ */
/*     packet.gtp2l.h.seq = hton24(getNextSeq(SELF_ON_SIG)); */
/*     packet.gtp2l.h.tei = user->s11.teid; */

/*     /\*******************************************************************\/ */
/*     sock = PDATA->s11->fd; */
/*     peer = (struct sockaddr_in *)&(PDATA->s11->peerAddr); */
/*     peer->sin_family = AF_INET; */
/*     peer->sin_port = htons(GTP2C_PORT); */
/*     peer->sin_addr.s_addr = user->s11.addr.addrv4; */
/*     PDATA->s11->socklen = sizeof(struct sockaddr_in); */

/*     peerlen = PDATA->s11->socklen; */

/*     if (sendto(sock, &packet, length, 0, (struct sockaddr *)peer, peerlen) < 0) { */
/*         log_errpack(LOG_ERR, errno, (struct sockaddr_in *)peer, &packet, length, */
/*                 "Sendto(fd=%d, msg=%lx, len=%d) failed", sock, (unsigned long) &packet, length); */
/*         return; */
/*     } */
/*     /\*******************************************************************\/ */

/* } */

/* uint8_t TASK_MME_S11___validate_ModifyBearerReq(S11_user_t u, S11_Signal sig){ */
/*     struct t_message *msg; */
/*     union gtpie_member *ie[GTPIE_SIZE], *bearerCtxGroupIE[GTPIE_SIZE]; */
/*     uint8_t value[40]; */
/*     uint16_t vsize; */
/*     uint32_t numIE; */
/*     struct fteid_t fteid; */
/*     struct in_addr s1uaddr; */

/*     msg = (struct t_message *)signal->data; */

/*     /\*  TODO @Vicent Check message mandatory IE*\/ */
/*     log_msg(LOG_DEBUG, 0, "Parsing Modify Bearer Req"); */

/*     gtp2ie_decap(ie, &(msg->packet), msg->length); */
/*     if(ntoh32(msg->packet.gtp.gtp2l.h.tei) != PDATA->user_ctx->S11MMETeid){ */
/*         log_msg(LOG_WARNING, 0, "TEID incorrect 0x%x != 0x%x", ntoh32(msg->packet.gtp.gtp2l.h.tei), PDATA->user_ctx->S11MMETeid); */
/*         return 1; */
/*     } */

/*     /\* Cause*\/ */
/*     gtp2ie_gettliv(ie, GTPV2C_IE_CAUSE, 0, value, &vsize); */
/*     if(value!= NULL && vsize>0){ */
/*         if(value[0]!=GTPV2C_CAUSE_REQUEST_ACCEPTED){ */
/*             log_msg(LOG_WARNING, 0, "Create Session request rejected Cause %d", value[0]); */
/*             return 1; */
/*         } */
/*     } */


/*     /\* Bearer Context*\/ */
/*     gtp2ie_gettliv(ie, GTPV2C_IE_BEARER_CONTEXT, 0, value, &vsize); */
/*     if(value!= NULL && vsize>0){ */
/*         gtp2ie_decaps_group(bearerCtxGroupIE, &numIE, value, vsize); */

/*         /\* EPS Bearer ID*\/ */
/*         gtp2ie_gettliv(bearerCtxGroupIE, GTPV2C_IE_EBI, 0, value, &vsize); */
/*         if(vsize != 1 && PDATA->user_ctx->ebearer[0].id != *value){ */
/*             log_msg(LOG_ERR, 0, "EPC Bearer ID %u != %u received", PDATA->user_ctx->ebearer[0].id, value); */
/*             return 1; */
/*         } */


/*         /\* F-TEID S1-U (SGW)*\/ */
/*         gtp2ie_gettliv(bearerCtxGroupIE, GTPV2C_IE_FTEID, 0, value, &vsize); */
/*         if(value!= NULL && vsize>0){ */
/*             memcpy(&fteid, value, vsize); */
/*             if(memcmp(&(PDATA->user_ctx->ebearer[0].s1u_sgw), value, vsize) != 0){ */
/*                 log_msg(LOG_ERR, 0, "S1-U SGW FEID not corresponds to the stored one"); */
/*                 return 1; */
/*             } */
/*         } */
/*     } */
/*     return 0; */
/* } */

void TASK_MME_S11___Forge_CreateIndirectDataForwardingTunnel(S11_user_t u, S11_Signal sig){
    union gtp_packet packet;
    struct fteid_t  dl_fteid, ul_fteid;
    size_t peerlen;
    struct sockaddr_in  *peer;
    union gtpie_member ie[2], ie_bearer_ctx[3];
    int hlen, sock, a;
    uint32_t length, fteid_size, teid;
    struct user_ctx_t *user = PDATA->user_ctx;


    log_msg(LOG_DEBUG, 0, "Enter");

    /******************************************************************************/
    user = PDATA->user_ctx;

    length = get_default_gtp(2, GTP2_CREATE_INDIRECT_DATA_FORWARDING_TUNNEL_REQ, &packet);

    print_packet(&packet, length);
    /*Bearer contex*/
        /*EPS Bearer ID */
        ie_bearer_ctx[0].tliv.i=0;
        ie_bearer_ctx[0].tliv.l=hton16(1);
        ie_bearer_ctx[0].tliv.t=GTPV2C_IE_EBI;
        ie_bearer_ctx[0].tliv.v[0]=user->ebearer[0].id;

        /* fteid S1-U eNB dl*/
        if(user->hoCtx.dL_Forward_transportLayerAddress.len==32){
            dl_fteid.ipv4=1;
            dl_fteid.ipv6=0;
            fteid_size = FTEID_IP4_SIZE;
        }else if(user->hoCtx.dL_Forward_transportLayerAddress.len==128){
            dl_fteid.ipv4=0;
            dl_fteid.ipv6=1;
            fteid_size = FTEID_IP6_SIZE;
        }else{
            dl_fteid.ipv4=1;
            dl_fteid.ipv6=1;
            fteid_size = FTEID_IP46_SIZE;
        }
        dl_fteid.iface= hton8(eNB_U_DL);
        memcpy(&teid ,user->hoCtx.dL_Forward_GTP_TEID.teid, sizeof(uint32_t));
        dl_fteid.teid = hton32(teid);
        memcpy(&(dl_fteid.addr.addrv4), user->hoCtx.dL_Forward_transportLayerAddress.addr, user->hoCtx.dL_Forward_transportLayerAddress.len/8);
        ie_bearer_ctx[1].tliv.i=0;
        ie_bearer_ctx[1].tliv.t=GTPV2C_IE_FTEID;
        ie_bearer_ctx[1].tliv.l=hton16(fteid_size);
        memcpy(ie_bearer_ctx[1].tliv.v, &dl_fteid, fteid_size);

        /* fteid S1-U eNB ul*/
        if(user->hoCtx.uL_Forward_transportLayerAddress.len==32){
            ul_fteid.ipv4=1;
            ul_fteid.ipv6=0;
            fteid_size = FTEID_IP4_SIZE;
        }else if(user->hoCtx.uL_Forward_transportLayerAddress.len==128){
            ul_fteid.ipv4=0;
            ul_fteid.ipv6=1;
            fteid_size = FTEID_IP6_SIZE;
        }else{
            ul_fteid.ipv4=1;
            ul_fteid.ipv6=1;
            fteid_size = FTEID_IP46_SIZE;
        }
        ul_fteid.iface= hton8(eNB_U_UL);
        memcpy(&teid ,user->hoCtx.uL_Forward_GTP_TEID.teid, sizeof(uint32_t));
        ul_fteid.teid = hton32(teid);
        memcpy(&(ul_fteid.addr.addrv4), user->hoCtx.uL_Forward_transportLayerAddress.addr, user->hoCtx.uL_Forward_transportLayerAddress.len/8);
        ie_bearer_ctx[2].tliv.i=4;
        ie_bearer_ctx[2].tliv.t=GTPV2C_IE_FTEID;
        ie_bearer_ctx[2].tliv.l=hton16(fteid_size);
        memcpy(ie_bearer_ctx[2].tliv.v, &ul_fteid, fteid_size);

    gtp2ie_encaps_group(GTPV2C_IE_BEARER_CONTEXT, 0, &ie[0], ie_bearer_ctx, 3);
    gtp2ie_encaps(ie, 1, (void*)&packet, &length);

    /*Packet header modifications*/
    packet.gtp2l.h.seq = hton24(getNextSeq(SELF_ON_SIG));
    packet.gtp2l.h.tei = user->s11.teid;

    print_packet(&packet, length);
    /*******************************************************************/
    sock = PDATA->s11->fd;
    peer = (struct sockaddr_in *)&(PDATA->s11->peerAddr);
    peer->sin_family = AF_INET;
    peer->sin_port = htons(GTP2C_PORT);
    peer->sin_addr.s_addr = user->s11.addr.addrv4;
    PDATA->s11->socklen = sizeof(struct sockaddr_in);

    peerlen = PDATA->s11->socklen;

    if (sendto(sock, &packet, length, 0, (struct sockaddr *)peer, peerlen) < 0) {
        log_errpack(LOG_ERR, errno, (struct sockaddr_in *)peer, &packet, length,
                "Sendto(fd=%d, msg=%lx, len=%d) failed", sock, (unsigned long) &packet, length);
        return;
    }
    /*******************************************************************/

}

uint8_t TASK_MME_S11___Validate_createIndirectDataForwardingTunnelRsp(S11_user_t u, S11_Signal sig){
    struct t_message *msg;
    union gtpie_member *ie[GTPIE_SIZE], *bearerCtxGroupIE[GTPIE_SIZE];
    uint8_t value[40];
    uint16_t vsize;
    uint32_t numIE;
    struct fteid_t fteid;
    struct in_addr s1uaddr;

    msg = (struct t_message *)signal->data;

    /*  TODO @Vicent Check message mandatory IE*/
    log_msg(LOG_DEBUG, 0, "Enter");

    gtp2ie_decap(ie, &(msg->packet), msg->length);
    if(ntoh32(msg->packet.gtp.gtp2l.h.tei) != PDATA->user_ctx->S11MMETeid){
        log_msg(LOG_WARNING, 0, "TEID incorrect 0x%x != 0x%x", ntoh32(msg->packet.gtp.gtp2l.h.tei), PDATA->user_ctx->S11MMETeid);
        return 1;
    }

    /* Cause*/
    gtp2ie_gettliv(ie, GTPV2C_IE_CAUSE, 0, value, &vsize);
    if(value!= NULL && vsize>0){
        if(value[0]!=GTPV2C_CAUSE_REQUEST_ACCEPTED){
            log_msg(LOG_WARNING, 0, "Create Session request rejected Cause %d", value[0]);
            return 1;
        }
    }


    /* Bearer Context*/
    gtp2ie_gettliv(ie, GTPV2C_IE_BEARER_CONTEXT, 0, value, &vsize);
    if(value!= NULL && vsize>0){
        gtp2ie_decaps_group(bearerCtxGroupIE, &numIE, value, vsize);

        /* EPS Bearer ID*/
        gtp2ie_gettliv(bearerCtxGroupIE, GTPV2C_IE_EBI, 0, value, &vsize);
        if(vsize != 1 && PDATA->user_ctx->ebearer[0].id != *value){
            log_msg(LOG_ERR, 0, "EPC Bearer ID %u != %u received", PDATA->user_ctx->ebearer[0].id, value);
            return 1;
        }


        /* F-TEID S1-U (SGW)*/
        gtp2ie_gettliv(bearerCtxGroupIE, GTPV2C_IE_FTEID, 0, value, &vsize);
        if(value!= NULL && vsize>0){
            memcpy(&fteid, value, vsize);
            if(memcmp(&(PDATA->user_ctx->ebearer[0].s1u_sgw), value, vsize) != 0){
                log_msg(LOG_ERR, 0, "S1-U SGW FEID not corresponds to the stored one");
                return 1;
            }
        }

        /* F-TEID GTP-U Data Forwarding(SGW)*/
        gtp2ie_gettliv(bearerCtxGroupIE, GTPV2C_IE_FTEID, 3, value, &vsize);
        if(value!= NULL && vsize>0){
            memcpy(&(PDATA->user_ctx->hoCtx.dataforwarding_sgw), value, vsize);
        }
    }
    return 0;
}


/* ======================================================================
 * S11 API Implementation
 * ====================================================================== */
/**@brief Create S11 process structure
 * @param [in]  engine Engine reference
 * @param [in]  owner Parent process.
 *
 */
struct t_process *S11_handler_create(struct t_engine_data *engine, struct t_process *owner){
    struct t_process    *pSelf = (struct t_process *)NULL;  /* agent process */

    pSelf = process_create(engine, STATE_Handle_Recv_Msg, NULL, owner);
    return pSelf;
}

gpointer S11_newUserAttach(gpointer s11_h, struct user_ctx_t *user,
                        void(*cb)(gpointer), gpointer args){
    log_msg(LOG_DEBUG, 0, "enter S11_newUserAttach()");

    S11_t *self = (S11_t *) s11_h;

    /* New MME TEID for the new user*/
    gpointer u = s11_newSession(self, user);

    /* u->state = STATE_attach; */
    /* u->state(u, S11_attach); */
    attach(u, cb, args);

    //cb(args);
    return u;
}


void S11_Attach_ModifyBearerReq(gpointer s11_user, void(*cb)(gpointer), gpointer args){
    log_msg(LOG_DEBUG, 0, "enter");

    /* u->state = STATE_attach; */
    /* u->state(u, S11_ModifyBearer); */
    modBearer(u, cb, args);

    //cb(args);
}

void S11_dettach(gpointer s11_user, void(*cb)(gpointer), gpointer args){
    log_msg(LOG_DEBUG, 0, "enter");

    u->state = STATE_Delete_User_Session;
    u->state(u, S11_detach);

    //cb(args);
}

void S11_ReleaseAccessBearers(gpointer s11_user, void(*cb)(gpointer), gpointer args){
    log_msg(LOG_DEBUG, 0, "enter");

    u->state = STATE_ReleaseAccessBearers;
    u->state(u, S11_releaseAccessBearers);

    //cb(args);
}

/* void S11_CreateIndirectDataForwardingTunnel(gpointer s11_user, void(*cb)(gpointer), gpointer args){ */
/*     log_msg(LOG_DEBUG, 0, "enter"); */

/*     u->state = STATE_CreateIndirectDataForwardingTunnel; */
/*      u->state(u, S11_createIndirectDataForwardingTunnel); */

/*      //cb(args); */
/* } */
