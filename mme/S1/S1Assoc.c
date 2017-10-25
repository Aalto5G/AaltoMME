/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S1Assoc.c
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief
 *
 *
 */

#include <glib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>
#include <unistd.h>
/* #include <netinet/sctp.h> */

#include "EMMCtx_iface.h" /* Used for paging*/
#include "S1Assoc.h"
#include "S1Assoc_priv.h"
#include "S1Assoc_FSMConfig.h"
#include "S1AP.h"
#include "logmgr.h"
#include "MMEutils.h"
#include "ECMSession.h"
#include "MME.h"
#include "MME_S1_priv.h"
#include "MMEutils.h"

void s1Assoc_log_(S1Assoc assoc, int pri, char *fn, const char *func, int ln,
              int en, char *fmt, ...){
    S1Assoc_t *self = (S1Assoc_t *)assoc;
    va_list args;
    char buf[SYSERR_MSGSIZE];
    size_t len;
    bzero(buf, SYSERR_MSGSIZE);

    if(self){
        snprintf(buf, SYSERR_MSGSIZE, "%s %s: ",
                 S1AssocStateName[self->stateName],
                 s1Assoc_getName(self));
    }

    len = strlen(buf);
    va_start(args, fmt);
    vsnprintf(buf+len, SYSERR_MSGSIZE, fmt, args);
    buf[SYSERR_MSGSIZE-1] = 0; /* Make sure it is null terminated */
    log_msg_s(pri, fn, func, ln, en, buf);
    va_end(args);
}

/* API to MME_S1 */
S1Assoc s1Assoc_init(S1 s1){
    S1Assoc_t *self = g_new0(S1Assoc_t, 1);
    self->s1 = s1;
    self->eNBname = g_string_new("");
    self->ecm_sessions = g_hash_table_new_full(g_int_hash,
                                               g_int_equal,
                                               NULL,
                                               NULL);
    s1ChangeState(self, S1_NotConfigured);
    return self;
}

static gboolean s1Assoc_free_deleteECM(gpointer key,
                                       gpointer value,
                                       gpointer user_data){
    S1Assoc_t *self = (S1Assoc_t *)user_data;
    struct mme_t * mme = s1_getMME(self->s1);

    mme_deregisterECM(mme, value);
    /*The ecmSession_free(value); is executed in the previous function*/
    return TRUE;
}

void s1Assoc_free(gpointer h){
    S1Assoc_t *self = (S1Assoc_t *)h;
    s1Assoc_log(self, LOG_DEBUG, 0, "Enter");
    g_hash_table_foreach_remove(self->ecm_sessions, s1Assoc_free_deleteECM, self);

    if(self->ecm_sessions)
        g_hash_table_destroy(self->ecm_sessions);

    if(self->fd>0)
        close(self->fd);

    if(self->eNBname)
        g_string_free(self->eNBname, TRUE);

    if(self->supportedTAs){
        if(self->supportedTAs->freeIE){
            self->supportedTAs->freeIE(self->supportedTAs);
        }
    }

    if(self->cSG_IdList){
        if(self->cSG_IdList->freeIE){
            self->cSG_IdList->freeIE(self->supportedTAs);
        }
    }

    g_free(self);
}

/** S1 Accept function callback*/
static void s1_accept(evutil_socket_t fd, short event, void *arg){

    uint32_t flags=0, i=0;

    S1Assoc_t *self = (S1Assoc_t *)arg;
    struct t_message *msg;
    struct user_ctx_t *user;

    /*SCTP variables*/
    struct sctp_sndrcvinfo sndrcvinfo;

    S1AP_Message_t *s1msg;
    GError *error = NULL;
    struct mme_t * mme = s1_getMME(self->s1);

    memset(&sndrcvinfo, 0, sizeof(struct sctp_sndrcvinfo));

    msg = newMsg();

    /* Read and emit the status of the Socket (optional step) */
    /*struct sctp_status status;
    int in;
    in = sizeof(status);
    getsockopt( fd, SOL_SCTP, SCTP_STATUS, (void *)&status, (socklen_t *)&in );

    printf("assoc id  = %d\n", status.sstat_assoc_id );
    printf("state     = %d\n", status.sstat_state );
    printf("instrms   = %d\n", status.sstat_instrms );
    printf("outstrms  = %d\n", status.sstat_outstrms );*/

    msg->length = sctp_recvmsg( fd,
                                (void *)&(msg->packet),
                                sizeof(msg->packet),
                                (struct sockaddr *)NULL,
                                0,
                                &sndrcvinfo,
                                &flags );

    if (flags & MSG_NOTIFICATION){
        s1Assoc_log(self, LOG_INFO, 0, "Received SCTP notification");
    }

    /*Check errors*/
    if (msg->length <= 0) {
        mme_deregisterRead(mme, s1Assoc_getfd(self));
        mme_deregisterS1Assoc(mme, self);
        s1_deregisterAssoc(self->s1, self);
        log_msg(LOG_DEBUG, 0, "Connection closed");
        freeMsg(msg);
        return;
    }

    /* If the packet is not a s1ap packet, it is silently rejected*/
    if(sndrcvinfo.sinfo_ppid != SCTP_S1AP_PPID){
        freeMsg(msg);
        return;
    }

    s1Assoc_log(self, LOG_DEBUG, 0, "Received %u bytes on stream %x",
            msg->length,
            sndrcvinfo.sinfo_stream);

    s1msg = s1ap_decode((void *)msg->packet.raw, msg->length);

    /* Process message*/
    self->state->processMsg(self, s1msg, sndrcvinfo.sinfo_stream, &error);
    if (error != NULL){
        mme_deregisterRead(mme, s1Assoc_getfd(self));
        mme_deregisterS1Assoc(mme, self);
        s1_deregisterAssoc(self->s1, self);
    }

    s1msg->freemsg(s1msg);
    freeMsg(msg);
}

void s1Assoc_accept(S1Assoc h, int ss){
    S1Assoc_t *self = (S1Assoc_t *)h;

    struct sockaddr_in *addr_in;
    char ipStr[INET6_ADDRSTRLEN];

    /*SCTP structures*/
    struct sctp_event_subscribe events;
    int optval, on;
    struct mme_t * mme = s1_getMME(self->s1);

    self->socklen = sizeof(struct sockaddr);

    /* Accept new connection*/
    self->fd = accept(ss,
                      &(self->peerAddr),
                      &(self->socklen) );
    if(self->fd==-1){
        s1Assoc_log(self, LOG_ERR, errno,
                "Error accepting connection, fd = %d, addr %#x, endpoint %#x",
                ss,
                &(self->peerAddr),
                self);
        return;
    }
    addr_in = (struct sockaddr_in *)&(self->peerAddr);
    inet_ntop(AF_INET,
              &(addr_in->sin_addr),
              ipStr,
              INET_ADDRSTRLEN);
    s1Assoc_log(self, LOG_DEBUG, 0, "Accepted SCTP association from %s:%u",
            ipStr, ntoh16(addr_in->sin_port));

    /* Enable reception of SCTP Snd/Rcv Data via sctp_recvmsg */
    memset((void *)&events, 0, sizeof(struct sctp_event_subscribe) );
    events.sctp_data_io_event = 1;
    setsockopt(self->fd, SOL_SCTP, SCTP_EVENTS,
               (const void *)&events, sizeof(struct sctp_event_subscribe) );
    /* on = 1; */
    /* setsockopt(self->fd, IPPROTO_SCTP, SCTP_RECVRCVINFO, */
    /*                &on, sizeof(on)); */

    mme_registerRead(mme, self->fd, s1_accept, self);
    s1_registerAssoc(self->s1, self);
}

void s1Assoc_disconnect(S1Assoc h){
    S1Assoc_t *self = (S1Assoc_t *)h;
    s1_deregisterAssoc(self->s1, self);
}


void s1Assoc_registerECMSession(S1Assoc h, gpointer  ecm){
    S1Assoc_t *self = (S1Assoc_t *)h;
    struct mme_t * mme = s1_getMME(self->s1);

    g_hash_table_insert(self->ecm_sessions, ecmSession_geteNBUEID_p(ecm), ecm);
}

void s1Assoc_deregisterECMSession(S1Assoc h, gpointer ecm){
    S1Assoc_t *self = (S1Assoc_t *)h;
    if(!g_hash_table_remove(self->ecm_sessions, ecmSession_geteNBUEID_p(ecm))){
        s1Assoc_log(self, LOG_ERR,  0, "ECM session not found in S1 Association");
    }
}

gpointer *s1Assoc_getECMSession(const S1Assoc h, guint32 id){
    S1Assoc_t *self = (S1Assoc_t *)h;
    return g_hash_table_lookup(self->ecm_sessions, &id);
}

void s1Assoc_setState(S1Assoc s1, S1Assoc_State *s, S1AssocState name){
    S1Assoc_t *self = (S1Assoc_t *)s1;
    self->state = s;
    self->stateName = name;
}

void s1Assoc_resetECM(S1Assoc s1, gpointer ecm){
    S1Assoc_t *self = (S1Assoc_t *)s1;
    struct mme_t * mme = s1_getMME(self->s1);

    ecmSession_reset(ecm);
    mme_deregisterECM(mme, ecm);
    /*The ecmSession_free(value); is executed in the previous function*/
}

/**@brief S1 Send message
 * @param [in] ep_S1    Destination EndPoint information
 * @param [in] streamId Strem to send the message
 * @param [in] s1msg    Message to be sent
 *
 * This function send the S1 message using the SCTP protocol
 * */
void s1Assoc_send(gpointer s1, uint32_t streamId, S1AP_Message_t *s1msg){
    uint8_t buf[10000];
    uint32_t bsize, ret;
    S1Assoc_t *self = (S1Assoc_t *)s1;

    memset(buf, 0, 10000);
    s1Assoc_log(self, LOG_DEBUG, 0, "Send %s", elementaryProcedureName[s1msg->pdu->procedureCode]);
    s1ap_encode(buf, &bsize, s1msg);

    /*printfbuffer(buf, bsize);*/

    /* sctp_sendmsg*/
    ret = sctp_sendmsg( self->fd, (void *)buf, (size_t)bsize, NULL, 0, SCTP_S1AP_PPID, 0, streamId, 0, 0 );

    if(ret==-1){
        s1Assoc_log(self, LOG_ERR, errno, "Error sending SCTP message to eNB %s", self->eNBname->str);
    }
}

/**@brief S1 Send message to non UE signaling
 * @param [in] ep_S1    Destination EndPoint information
 * @param [in] s1msg    Message to be sent
 *
 * This function send the S1 message to non UE associated signaling.
 * It uses the stream id used during the S1 Setup procedure
 * */
void s1Assoc_sendNonUE(gpointer s1, S1AP_Message_t *s1msg){
    S1Assoc_t *self = (S1Assoc_t *)s1;
    s1Assoc_send(s1, self->nonue_rsid, s1msg);
}


int *s1Assoc_getfd_p(const S1Assoc h){
    S1Assoc_t *self = (S1Assoc_t *)h;
    return &(self->fd);
}

const int s1Assoc_getfd(const S1Assoc h){
    S1Assoc_t *self = (S1Assoc_t *)h;
    return self->fd;
}

void s1Assoc_setGlobalID(gpointer h, const Global_ENB_ID_t *gid){
    S1Assoc_t *self = (S1Assoc_t *)h;
    globaleNBID_Fill(&(self->global_eNB_ID), gid);
}

mme_GlobaleNBid *s1Assoc_getID_p(const S1Assoc h){
    S1Assoc_t *self = (S1Assoc_t *)h;
    return &(self->global_eNB_ID);
}


mme_GlobaleNBid *s1Assoc_getID(const S1Assoc h, mme_GlobaleNBid *out){
    S1Assoc_t *self = (S1Assoc_t *)h;
    return globaleNBID_copy(&(self->global_eNB_ID), out);
}

const char *s1Assoc_getName(const S1Assoc h){
    S1Assoc_t *self = (S1Assoc_t *)h;
    return self->eNBname->str;
}

S1 s1Assoc_getS1(gpointer h){
    S1Assoc_t *self = (S1Assoc_t *)h;
    return self->s1;
}

static gboolean s1Assoc_containsTA(const S1Assoc_t *self, const guint8 sn[3], const guint16 tac){
    int i, j;
    BPLMNs_t *bc_l;
    SupportedTAs_t *tas = self->supportedTAs;
    PLMNidentity_t *plmn_eNB;
    
    for(i=0; i<tas->size; i++){      
        if(memcmp(tas->item[i]->tAC->s, &tac, 2)!=0){
	    guint16 printable_tac = tac>>8 | tac<<8;
	    log_msg(LOG_WARNING, 0, "UE TAC (%#X) != Supported TAC (0x%X%X) in eNB",
	            printable_tac, tas->item[i]->tAC->s[0], tas->item[i]->tAC->s[1]);
	    continue;
        }
	
        bc_l = tas->item[i]->broadcastPLMNs;
        for(j=0; j<bc_l->n ; j++){
	    plmn_eNB = bc_l->pLMNidentity[j];
	    
            // Get PLMN from UE and eNB and convert from TBCD to human-readable output
            guint8 plmn_UE_printable [7] = {0};
            guint8 plmn_eNB_printable [7] = {0};
            plmn_FillPLMNFromTBCD (plmn_UE_printable, sn);
            plmn_FillPLMNFromTBCD (plmn_eNB_printable, plmn_eNB->tbc.s);
	    
            if(memcmp(sn, plmn_eNB->tbc.s, 3)==0){
	        log_msg(LOG_DEBUG, 0, "UE SN (%s) == Supported PLMN (%s) in eNB",
                        plmn_UE_printable, plmn_eNB_printable);
                return TRUE;
            }else{	       
                log_msg(LOG_DEBUG, 0, "UE SN (%s) != Supported PLMN (%s) in eNB",
                        plmn_UE_printable, plmn_eNB_printable);
            }
        }
    }
    return FALSE;
}

static void sendPaging(S1Assoc_t *self, EMMCtx emm){
    S1AP_Message_t *s1msg;
    S1AP_PROTOCOL_IES_t* ie;
    UEIdentityIndexValue_t *ue_id;
    UEPagingID_t *p_id;
    CNDomain_t *dom;
    TAIList_t *tais;
    TAIItem_t *tai;
    guint16   tac=0;

    /* Build paging*/
    s1msg = S1AP_newMsg();
    s1msg->choice = initiating_message;
    s1msg->pdu->procedureCode = id_Paging;
    s1msg->pdu->criticality = ignore;

    /* UEIdentityIndexValue */
    ue_id = s1ap_newIE(s1msg, id_UEIdentityIndexValue, mandatory, ignore);
    ue_id->id =  emmCtx_getIMSI(emm)%1024;

    /* UEPagingID */
    p_id = s1ap_newIE(s1msg, id_UEPagingID, mandatory, ignore);
    p_id->choice = 0;
    p_id->id.s_TMSI = new_S_TMSI();
    const guti_t *guti = emmCtx_getGUTI(emm);
    p_id->id.s_TMSI->mMEC->s[0] = guti->mmec;
    memcpy(p_id->id.s_TMSI->m_TMSI.s, &guti->mtmsi, 4);

    /* drx = s1ap_newIE(s1msg, id_pagingDRX, optional, ignore); */

    dom = s1ap_newIE(s1msg, id_CNDomain, mandatory, ignore);
    dom->domain = ps;

    tais = s1ap_newIE(s1msg, id_TAIList, mandatory, ignore);
    tai = tais->newItem(tais);
    tai->tAI->pLMNidentity = new_PLMNidentity();
    emmCtx_getTAI(emm, &tai->tAI->pLMNidentity->tbc.s, &tac);
    memcpy(tai->tAI->tAC->s, &tac, 2);

    /* csgs = s1ap_newIE(s1msg, id_CSG_IdList, optional, ignore); */

    /* p_pio = s1ap_newIE(s1msg, id_PagingPriority, optional, ignore); */

    /* rcap = s1ap_newIE(s1msg, id_UERadioCapabilityForPaging, optional, ignore); */

    /* Send Response*/
    s1Assoc_sendNonUE(self, s1msg);

    /*s1msg->showmsg(s1msg);*/

    s1msg->freemsg(s1msg);

}

void s1Assoc_paging(S1Assoc h, gpointer emm){
    S1Assoc_t *self = (S1Assoc_t *)h;
    guint64 imsi = emmCtx_getIMSI(emm);
    guint8 sn[3] = {0};
    guint16 tac = 0;

    emmCtx_getTAI(emm, &sn, &tac);
    if( s1Assoc_containsTA(self, sn, tac) ){
        sendPaging(self, emm);
    }
}
