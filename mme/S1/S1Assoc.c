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
#include "S1AP.h"
#include "logmgr.h"

/* API to MME_S1 */
S1Assoc s1Assoc_init(S1 s1){
    S1Assoc_t *self = g_new0(S1Assoc_t, 1);
    self->s1 = s1;
    return self;
}

void s1Assoc_free(gpointer h){
    S1Assoc_t *self = (S1Assoc_t *)h;
    s1_deregisterAssoc(self->s1, h);
    close(self->fd);
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
    struct sctp_status status;

    memset(&sndrcvinfo, 0, sizeof(struct sctp_sndrcvinfo));

    msg = newMsg();

    /* Read and emit the status of the Socket (optional step) */
    /*in = sizeof(status);
    getsockopt( listener, SOL_SCTP, SCTP_STATUS, (void *)&status, (socklen_t *)&in );

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

    log_msg(LOG_DEBUG, 0, "S1AP: Received %u bytes on stream %x", msg->length, sndrcvinfo.sinfo_stream);

    /* if(ep_S1->portState!=opened){ */
    /*     if(ep_S1->portState == listening){ */
    /*         /\* Process new connection*\/ */
    /*         output = new_signal(S1_Setup(mme->engine, NULL, ep_S1)); */
    /*         output->name=S1_Setup_Endpoint; */
    /*         output->priority = MAXIMUM_PRIORITY; */
    /*     }else{ */
    /*         log_msg(LOG_ERR, 0, "S1AP: Received a message on a closed endpoint", msg->length, sndrcvinfo.sinfo_stream); */
    /*     } */
    /* }else{ */

    /*     output = new_signal(ep_S1->handler); */
    /*     output->name=S1_handler_ready; */

    /*     ep_S1->handler->data = ep_S1;   /\* The data field on the endpoint handler is a reference to the endpoint parent*\/ */
    /*     memcpy(&((S1_EndPoint_Info_t*)ep_S1->info)->sndrcvinfo, &sndrcvinfo, sizeof(struct sctp_sndrcvinfo)); */
    /* } */

    /* output->data = (void *)msg; */
    /* output->freedataFunc = freeMsg; */

    /* signal_send(output); */

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

int *s1Assoc_getfd_p(const S1Assoc h){
    S1Assoc_t *self = (S1Assoc_t *)h;
    return &(self->fd);
}

const int s1Assoc_getfd(const S1Assoc h){
    S1Assoc_t *self = (S1Assoc_t *)h;
    return self->fd;
}
