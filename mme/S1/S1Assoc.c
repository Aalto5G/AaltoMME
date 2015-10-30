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

#include "S1Assoc.h"
#include "S1Assoc_priv.h"
#include "S1Assoc_FSMConfig.h"
#include "S1AP.h"
#include "logmgr.h"
#include "MMEutils.h"
#include "ECMSession.h"
#include "MME.h"
#include "MME_S1_priv.h"

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
