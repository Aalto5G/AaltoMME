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
/* #include <netinet/sctp.h> */

#include "S1Assoc.h"
#include "S1Assoc_priv.h"
#include "S1Assoc_FSMConfig.h"
#include "S1AP.h"
#include "logmgr.h"
#include "MMEutils.h"

/* API to MME_S1 */
S1Assoc s1Assoc_init(S1 s1){
    S1Assoc_t *self = g_new0(S1Assoc_t, 1);
    self->s1 = s1;
    self->eNBname = g_string_new("");
    s1ChangeState(self, NotConfigured);
    return self;
}

void s1Assoc_free(gpointer h){
    S1Assoc_t *self = (S1Assoc_t *)h;
    log_msg(LOG_DEBUG, 0, "Enter");
    self->state->disconnect(self);
    close(self->fd);

    g_string_free(self->eNBname, TRUE);

    if(self->suportedTAs){
        if(self->suportedTAs->freeIE){
            self->suportedTAs->freeIE(self->suportedTAs);
        }
    }

    if(self->cGS_IdList){
        if(self->cGS_IdList->freeIE){
            self->cGS_IdList->freeIE(self->suportedTAs);
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

    struct EndpointStruct_t* ep_S1 = NULL;

    /*SCTP variables*/
    struct sctp_sndrcvinfo sndrcvinfo;

    S1AP_Message_t *s1msg;

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
        log_msg(LOG_INFO, 0, "Received SCTP notification");
    }

    /*Check errors*/
    if (msg->length <= 0) {
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

    log_msg(LOG_DEBUG, 0, "S1AP: Received %u bytes on stream %x",
            msg->length,
            sndrcvinfo.sinfo_stream);

    s1msg = s1ap_decode((void *)msg->packet.raw, msg->length);

    /* Process message*/
    self->state->processMsg(self, s1msg, sndrcvinfo.sinfo_stream);

    s1msg->freemsg(s1msg);
    freeMsg(msg);
}

void s1Assoc_accept(S1Assoc h, int ss){
    S1Assoc_t *self = (S1Assoc_t *)h;

    struct sockaddr_in *addr_in;
    char ipStr[INET6_ADDRSTRLEN];

    /*SCTP structures*/
    struct sctp_event_subscribe events;
    int optval;

    self->socklen = sizeof(struct sockaddr);

    /* Accept new connection*/
    self->fd = accept(ss,
                      &(self->peerAddr),
                      &(self->socklen) );
    if(self->fd==-1){
        log_msg(LOG_ERR, errno,
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
    log_msg(LOG_DEBUG, 0, "Accepted SCTP association from %s:%u",
            ipStr, ntoh16(addr_in->sin_port));

    /* Enable receipt of SCTP Snd/Rcv Data via sctp_recvmsg */
    memset((void *)&events, 0, sizeof(events) );
    events.sctp_data_io_event = 1;
    setsockopt(self->fd, SOL_SCTP, SCTP_EVENTS,
               (const void *)&events, sizeof(events) );

    s1_registerAssoc(self->s1, self, self->fd, s1_accept);
}


void s1Assoc_setState(S1Assoc s1, S1Assoc_State *s){
    S1Assoc_t *self = (S1Assoc_t *)s1;
    self->state = s;
}

/**@brief S1 Send message
 * @param [in] ep_S1    Destination EndPoint information
 * @param [in] streamId Strem to send the message
 * @param [in] s1msg    Message to be sent
 *
 * This function send the S1 message using the SCTP protocol
 * */
void s1Assoc_send(gpointer s1,uint32_t streamId, S1AP_Message_t *s1msg){
    uint8_t buf[10000];
    uint32_t bsize, ret;
    S1Assoc_t *self = (S1Assoc_t *)s1;

    memset(buf, 0, 10000);
    log_msg(LOG_DEBUG, 0, "S1AP: Send %s", elementaryProcedureName[s1msg->pdu->procedureCode]);
    s1ap_encode(buf, &bsize, s1msg);

    /*printfbuffer(buf, bsize);*/

    /* sctp_sendmsg*/
    ret = sctp_sendmsg( self->fd, (void *)buf, (size_t)bsize, NULL, 0, SCTP_S1AP_PPID, 0, streamId, 0, 0 );

    if(ret==-1){
        log_msg(LOG_ERR, errno, "S1AP : Error sending SCTP message to eNB %s", self->eNBname->str);
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
