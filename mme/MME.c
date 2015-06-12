/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   MME.h
 * @Author Vicent Ferrer
 * @date   March, 2013
 * @brief  MME type definition and functions.
 *
 * The goal of this file is to define the generic functions of the MME and its interfaces.
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/sctp.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <glib.h>

#include "logmgr.h"
#include "gtp.h"
#include "commands.h"
#include "MME.h"
#include "MME_engine.h"
#include "MME_S1.h"
#include "MME_S11.h"
#include "MME_S6a.h"
#include "MME_Controller.h"
#include "HSS.h"

int mme_run_flag=0;


void free_ep(gpointer data){
    struct EndpointStruct_t* ep = (struct EndpointStruct_t*)data;
    if (ep->info)
        free_S1_EndPoint_Info(ep->info);

    if(ep->handler)
        engine_process_stop(ep->handler);

    if (ep->ev)
        event_free(ep->ev);

    if (ep->portState != closed){
        close(ep->fd);
    }
    free(ep);
}


/**@brief Simple UDP creation
 * @param [in] port server UDP port
 * @returns file descriptor*/
int init_udp_srv(int port){
    int fd;
    struct sockaddr_in addr;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        log_msg(LOG_ERR, errno, "socket(domain=%d, type=%d, protocol=%d) failed", AF_INET, SOCK_DGRAM, 0);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    #if defined(__FreeBSD__) || defined(__APPLE__)
    addr.sin_len = sizeof(addr);
    #endif

    if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        log_msg(LOG_ERR, errno, "bind(fd=%d, addr=%lx, len=%d) failed", fd, (unsigned long) &addr, sizeof(addr));
        return -1;
    }
    return fd;
}

/**@brief Simple SCTP creation
 * @param [in] port server SCTP port
 * @returns file descriptor*/
int init_sctp_srv(int port, int addr){
    int listenSock, on=0, status, optval;
    struct sockaddr_in servaddr;

    struct sctp_initmsg initmsg;


    if ((listenSock =  socket( AF_INET, SOCK_STREAM, IPPROTO_SCTP ) )< 0 ) {
        log_msg(LOG_ERR, errno, "socket(domain=%d, type=%d, protocol=%d) failed", AF_INET, SOCK_STREAM, IPPROTO_SCTP);
        return -1;
    }

    /* Accept connections from any interface */
    bzero( (void *)&servaddr, sizeof(servaddr) );
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = addr;
    servaddr.sin_port = htons(port);

    /* Turn off bind address checking and allow port numbers to be reused*/
    /* on = 1; */
    /* if( setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int))==-1){ */
    /*     log_msg(LOG_ERR, errno, "setsockopt(fd=%d) to turn off bind address checking failed. ", listenSock); */
    /*     return -1; */
    /* } */


    if (bind( listenSock, (struct sockaddr *)&servaddr, sizeof(servaddr) ) < 0) {
        log_msg(LOG_ERR, errno, "bind(fd=%d, addr=%lx, len=%d) failed", listenSock, (unsigned long) &servaddr, sizeof(servaddr));
        return -1;
    }

    /* Specify that a maximum of 5 streams will be available per socket */
    memset( &initmsg, 0, sizeof(initmsg) );
    initmsg.sinit_num_ostreams = 2;
    initmsg.sinit_max_instreams = 2;
    initmsg.sinit_max_attempts = 4;
    if ( setsockopt( listenSock, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg) )< 0) {
        log_msg(LOG_ERR, errno, "setsockopt error");
        return -1;
    }

    /* Disable Nagle algorithm */
    optval=1;
    if (setsockopt(listenSock, IPPROTO_SCTP, SCTP_NODELAY, (void*)&optval, sizeof(optval)) ==-1){
        log_msg(LOG_WARNING, errno, "Couldn't set SCTP_NODELAY socket option.");
        /* No critical*/
    }

    /* Place the server socket into the listening state */
    if (listen( listenSock, 5 )< 0) {
        log_msg(LOG_ERR, errno, "listen(fd=%d, 5) failed", listenSock);
        return -1;
    }

    return listenSock;
}

int mme_init_ifaces(struct mme_t *self){

    /*LibEvent structures*/
    struct event_base *base;
    struct event *listener_S11, *listener_command, *listener_S1, *listener_Ctrl;
    uint32_t addr = 0;

    self->s6a = s6a_init((gpointer)self);

    /*Init command server, returns server file descriptor*/
    self->command.fd = servcommand_init(COMMAND_PORT);
    self->command.portState=opened;
    log_msg(LOG_INFO, 0, "Open command server on file descriptor %d, port %d", self->command.fd, COMMAND_PORT);

    /*Init S11 server*/
    self->s11 = s11_init((gpointer)self);

    /*Init S1 server*/
    self->s1.fd =init_sctp_srv(S1AP_PORT, self->ipv4);
    self->s1.portState=opened;
    log_msg(LOG_INFO, 0, "Open S1 server on file descriptor %d, port %d", self->s1.fd, S1AP_PORT);

    /*Init Controller server*/
    self->ctrl.fd =init_udp_srv(CONTROLLER_PORT);
    self->ctrl.portState=opened;
    log_msg(LOG_INFO, 0, "Open SDN Controller server on file descriptor %d, port %d", self->ctrl.fd, CONTROLLER_PORT);

    return 0;
}

int mme_close_ifaces(struct mme_t *self){

    struct EndpointStruct_t *ep;
    int i=0;

    s6a_free(self->s6a);

    if( self->command.portState != closed){
        close(self->command.fd);
        self->command.portState = closed;
    }

    s11_free(self->s11);

    if( self->s1.portState != closed){
        close(self->s1.fd);
        self->s1.portState = closed;
    }

    /*
      Other S1AP SCTP associations remaining are freed when the GHashTable is destroyed
     */

    if( self->ctrl.portState != closed){
        close(self->ctrl.fd);
        self->ctrl.portState = closed;
    }

    return 0;
}


void mme_registerRead(struct mme_t *self, int fd, event_callback_fn cb, void * args){
    struct event *ev;
    ev = event_new(self->evbase, fd, EV_READ|EV_PERSIST, (event_callback_fn)cb, args);
    evutil_make_socket_nonblocking(fd);
    event_add(ev, NULL);
    g_hash_table_insert(self->ev_readers, &fd, ev);
}

void mme_deregisterRead(struct mme_t *self, int fd){
    g_hash_table_remove(self->ev_readers, &fd);
}


/**@brief Initialize mme structure
 */
int mme_run(struct mme_t *self){

    struct event_base       *base;                          /*< libevent base loop*/
    struct event            *kill_event;                    /*< Kill Posix signal event*/

    int allocated = 0;

    if(self==NULL){
        self = malloc(sizeof(struct mme_t));
        if(self==NULL){
            return 1;
        }
        allocated = 1;
        memset(self, 0, sizeof(struct mme_t));
        self->run = &mme_run_flag;
    }

    /*NULL initizalization required on hash table*/
    self->sessionht_byTEID = NULL;
    self->sessionht_byS1APID = NULL;

    self->seq = 0;

    /*Init user storage*/
    init_storage_system();

    /*Init node manager*/
    init_nodemgr();

    /*Load MME information from config file*/
    loadMMEinfo(self);

    self->s1ap = g_hash_table_new_full( g_int_hash,
                                        g_int_equal,
                                        NULL,
                                        free_ep);

    self->ev_readers = g_hash_table_new_full( g_int_hash,
                                              g_int_equal,
                                              NULL,
                                              (GDestroyNotify) event_free);
    /*Create event base structure*/
    self->evbase = event_base_new();
    if (!self->evbase){
        log_msg(LOG_ERR, 0, "Failed to create libevent event-base");
        if(allocated==1)
	        free(self);
        return 1;
    }

    mme_init_ifaces(self);

    mme_registerRead(self, self->command.fd, cmd_accept, self);
    mme_registerRead(self, self->s1.fd, s1_accept_new_eNB, self);
    mme_registerRead(self, self->ctrl.fd, ctrl_accept, self);

    /*Create event for processing SIGINT*/
    kill_event = evsignal_new(self->evbase, SIGINT, kill_handler, self);
    event_add(kill_event, NULL);


    /* Loop blocking*/
    engine_main(self);


    /*Dealocation */
    engine_process_stop(self->command.handler);
    //engine_process_stop(self->s11.handler);
    engine_process_stop(self->ctrl.handler);

    mme_deregisterRead(self, self->ctrl.fd);
    mme_deregisterRead(self, self->s1.fd);
    mme_deregisterRead(self, self->command.fd);

    mme_close_ifaces(self);

    event_free(kill_event);
    event_base_free(self->evbase);

    g_hash_table_destroy(self->s1ap);
    g_hash_table_destroy(self->ev_readers);

    freeMMEinfo(self);

    free_nodemgr();

    free_storage_system();

    if(allocated==1){
        free(self);
    }

    return 0;
}

int mme_main(){

    struct mme_t *mme;
    struct sigaction new_action, old_action;

    /*Init syslog entity*/
    init_logger("MME", LOG_INFO);

    mme = malloc(sizeof(struct mme_t));
    memset(mme, 0, sizeof(struct mme_t));

    mme->run = &mme_run_flag;

    mme_run_flag = 1;

    /* Configure SIGINT */
    /*new_action.sa_handler = termination_handler;
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = 0;

    sigaction (SIGINT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN){
        sigaction (SIGINT, &new_action, NULL);
    }*/

    /* run MME*/
    mme_run(mme);

    /*Close syslog entity*/
    close_logger();

    free(mme);

    mme_run_flag=0;

    return 0;
}

struct t_message *newMsg(){
    struct t_message *msg;
    msg = malloc(sizeof(struct t_message));
    memset(msg, 0, sizeof(struct t_message));
    return msg;
}

void freeMsg(void *msg){
    free((struct t_message *)msg);
}

/** S11 Accept function callback*/


/** S1 Accept function callback*/
void s1_accept(evutil_socket_t listener, short event, void *arg){

    uint32_t flags=0, i=0;

    Signal *output;

    struct mme_t *mme = (struct mme_t *)arg;
    struct t_message *msg;
    struct SessionStruct_t *usersession = NULL;
    struct user_ctx_t *user;

    struct EndpointStruct_t* ep_S1 = NULL;

    /*SCTP variables*/
    struct sctp_sndrcvinfo sndrcvinfo;
    struct sctp_status status;

    log_msg(LOG_DEBUG, 0, "Received listener=%u as arg", listener);

    memset(&sndrcvinfo, 0, sizeof(struct sctp_sndrcvinfo));

    /*Identify the Endpoint*/
    ep_S1 = g_hash_table_lookup(mme->s1ap, &listener);

    if(ep_S1 == NULL){
        log_msg(LOG_DEBUG, 0, "Invalid Endpoint");
        return;
    }

    msg = newMsg();

    /* Read and emit the status of the Socket (optional step) */
    /*in = sizeof(status);
    getsockopt( listener, SOL_SCTP, SCTP_STATUS, (void *)&status, (socklen_t *)&in );

    printf("assoc id  = %d\n", status.sstat_assoc_id );
    printf("state     = %d\n", status.sstat_state );
    printf("instrms   = %d\n", status.sstat_instrms );
    printf("outstrms  = %d\n", status.sstat_outstrms );*/

    msg->length = sctp_recvmsg( listener, (void *)&(msg->packet), sizeof(msg->packet), (struct sockaddr *)NULL, 0, &sndrcvinfo, &flags );

    if (flags & MSG_NOTIFICATION){
        log_msg(LOG_INFO, 0, "Received SCTP notification");
    }

    /*Check errors*/
    if (msg->length <= 0) {
        g_hash_table_remove(mme->s1ap, &listener);
        log_msg(LOG_INFO, 0, "Connection closed");
        freeMsg(msg);
        return;
    }

    /* If the packet is not a s1ap packet, it is silently rejected*/
    if(sndrcvinfo.sinfo_ppid != SCTP_S1AP_PPID){
        freeMsg(msg);
        return;
    }

    log_msg(LOG_DEBUG, 0, "S1AP: Received %u bytes on stream %x", msg->length, sndrcvinfo.sinfo_stream);

    if(ep_S1->portState!=opened){
        if(ep_S1->portState == listening){
            /* Process new connection*/
            output = new_signal(S1_Setup(mme->engine, NULL, ep_S1));
            output->name=S1_Setup_Endpoint;
            output->priority = MAXIMUM_PRIORITY;
        }else{
            log_msg(LOG_ERR, 0, "S1AP: Received a message on a closed endpoint", msg->length, sndrcvinfo.sinfo_stream);
        }
    }else{

        output = new_signal(ep_S1->handler);
        output->name=S1_handler_ready;

        ep_S1->handler->data = ep_S1;   /* The data field on the endpoint handler is a reference to the endpoint parent*/
        memcpy(&((S1_EndPoint_Info_t*)ep_S1->info)->sndrcvinfo, &sndrcvinfo, sizeof(struct sctp_sndrcvinfo));
    }

    output->data = (void *)msg;
    output->freedataFunc = freeMsg;

    signal_send(output);

}

/** S1 Accept function callback. Used to accept a new S1-MME connection (from eNB)*/
void s1_accept_new_eNB(evutil_socket_t listener, short event, void *arg){

    struct EndpointStruct_t *news1ep;
    struct EndpointStruct_t **tmp;
    Signal *output;
    int optval;

    struct mme_t *mme = (struct mme_t*)arg;

    /*Libevent structures*/
    struct event *listener_S1_Conn;

    /*SCTP structures*/
    struct sctp_event_subscribe events;

    log_msg(LOG_DEBUG, 0, "enter s1_accept_new_eNB()");

    /* Create new endpoint and add it to the mme structure*/
    news1ep =  (struct EndpointStruct_t*)malloc(sizeof(struct EndpointStruct_t));

    if(news1ep==NULL){
        log_msg(LOG_ERR, errno, "Error allocating Endpoint");
        return;
    }

    news1ep->socklen = sizeof(struct sockaddr);

    /* Accept new connection*/
    news1ep->fd = accept( listener, (struct sockaddr *)&(news1ep->peerAddr), (socklen_t *)&(news1ep->socklen) );
    if(news1ep->fd==-1){
        log_msg(LOG_ERR, errno, "Error accepting connection, fd = %d, addr %#x, endpoint %#x", listener, &(news1ep->peerAddr), news1ep);
        return;
    }
    log_msg(LOG_DEBUG, 0, "accept %d, server listener %u", news1ep->fd, listener);

    /* Store End point information*/
    news1ep->portState = listening;

    /* Enable receipt of SCTP Snd/Rcv Data via sctp_recvmsg */
    memset( (void *)&events, 0, sizeof(events) );
    events.sctp_data_io_event = 1;
    setsockopt( news1ep->fd, SOL_SCTP, SCTP_EVENTS, (const void *)&events, sizeof(events) );

    /* Create event for accepting messages from this new eNB*/
    listener_S1_Conn = event_new(mme->evbase, news1ep->fd, EV_READ|EV_PERSIST, s1_accept, mme);
    news1ep->ev = listener_S1_Conn;

    /* Make socket non blocking*/
    /*log_msg(LOG_DEBUG, 0, "news1mme->fd %u", news1ep->fd);*/
    evutil_make_socket_nonblocking(news1ep->fd);

    /*Store the new connection*/
    g_hash_table_insert(mme->s1ap, &(news1ep->fd), news1ep);

    /*Add event to the event base (libevent)*/
    event_add(listener_S1_Conn, NULL);

    log_msg(LOG_INFO, 0, "new eNB added");
}

void ctrl_accept(evutil_socket_t listener, short event, void *arg){

    uint32_t teid;
    struct SessionStruct_t *session;
    Signal *output;
    struct t_message *msg;
    struct t_message msg1;

    struct mme_t *mme = (struct mme_t *)arg;

    msg = newMsg();
    /* TODO @Robin Recv message from UDP socket*/
    ctrlp_recv(listener, &(msg->packet.sdnp), &(msg->length), (struct sockaddr_in*)&(msg->peer.peerAddr), &(msg->peer.socklen));
    msg->peer.fd = listener;

    /*Debugging*/
    /*printf("ctrl_accept(): Packet received: \n");
    print_packet(&(msg->packet), msg->length);*/

    /*Is this necessary?*/
    msg->peer.portState = opened;
    /********************/


    /* Manage new request*/
    log_msg(LOG_DEBUG, 0, "New SDN Controller message recv");
    output = new_signal(Controller_handler_create(mme->engine, NULL));
    output->name=Controller_handler_ready;
    output->priority = MAXIMUM_PRIORITY;

    output->data = (void *)msg;
    output->freedataFunc = freeMsg;
    signal_send(output);
}

void kill_handler(evutil_socket_t listener, short event, void *arg){
  struct mme_t *mme = (struct mme_t *)arg;

  if(mme_run!=NULL){
    mme_run_flag=0;
    log_msg(LOG_INFO, 0, "SIGINT detected. Closing MME");
  }else{
    log_msg(LOG_INFO, 0, "SIGINT detected.");
    exit(0);
  }
}

int addToPendingResponse(struct SessionStruct_t *session){

    struct SessionStruct_t *old = NULL;
    struct mme_t *mme = session->sessionHandler->engine->mme;
    /*Checking that the session is not on the hash table already, the value 0 as a key is not accepted*/
    if(session->user_ctx->S11MMETeid != 0){
        HASH_FIND(hh1, mme->sessionht_byTEID, &(session->user_ctx->S11MMETeid), sizeof(uint32_t), old);
    }else{
        HASH_FIND(hh2, mme->sessionht_byS1APID, &(session->user_ctx->mME_UE_S1AP_ID), sizeof(uint32_t), old);
    }

    if(old == NULL){
        if(session->user_ctx->S11MMETeid != 0){
            HASH_ADD(hh1, mme->sessionht_byTEID, user_ctx->S11MMETeid, sizeof(uint32_t), session);
        }
        if(session->user_ctx->mME_UE_S1AP_ID != 0){
            HASH_ADD(hh2, mme->sessionht_byS1APID, user_ctx->mME_UE_S1AP_ID, sizeof(uint32_t), session);
        }
        log_msg(LOG_DEBUG, 0, "Session with Teid 0x%x, MME UE S1AP ID %u stored", session->user_ctx->S11MMETeid, session->user_ctx->mME_UE_S1AP_ID);

    }else{
        log_msg(LOG_WARNING, 0, "The session is already on the hash table. Something is wrong. (TEID 0x%x, MME UE S1AP ID %u)", session->user_ctx->S11MMETeid, session->user_ctx->mME_UE_S1AP_ID);
        return 1;
    }

    session->pendingRsp=1;
    return 0;
}

int removePendentResponse(struct SessionStruct_t *session){

    struct mme_t *mme = session->sessionHandler->engine->mme;
    struct SessionStruct_t *old = NULL;
    /*Checking that the session is on the hash table already,*/
    if(session->user_ctx->S11MMETeid != 0){
        HASH_FIND(hh1, mme->sessionht_byTEID, &(session->user_ctx->S11MMETeid), sizeof(uint32_t), old);
    }else{
        HASH_FIND(hh2, mme->sessionht_byS1APID, &(session->user_ctx->mME_UE_S1AP_ID), sizeof(uint32_t), old);
    }

    if(old != NULL){
        if(session->user_ctx->S11MMETeid != 0){
            HASH_DELETE(hh1, mme->sessionht_byTEID, old);
        }
        if(session->user_ctx->mME_UE_S1AP_ID != 0){
            HASH_DELETE(hh2, mme->sessionht_byS1APID, old);
        }
    }else{
        log_msg(LOG_WARNING, 0, "removeToPendentResponse() Couldn't find the session of teid : %d, MME UE S1AP ID %u on the has table.", session->user_ctx->S11MMETeid, session->user_ctx->mME_UE_S1AP_ID);
        return 1;
    }
    session->pendingRsp=0;
    return 0;
}

struct SessionStruct_t *getPendingResponseByTEID(struct mme_t *mme, uint32_t teid){
    struct SessionStruct_t *s = NULL;

    if(teid!=0){
        HASH_FIND(hh1, mme->sessionht_byTEID, &teid, sizeof(uint32_t), s);
    }
    if(s != NULL){
        if(s->user_ctx->S11MMETeid != 0){
            HASH_DELETE(hh1, mme->sessionht_byTEID, s);
        }
        if(s->user_ctx->mME_UE_S1AP_ID != 0){
            HASH_DELETE(hh2, mme->sessionht_byS1APID, s);
        }
    }else{
        log_msg(LOG_WARNING, 0, "Couldn't find the session of teid : %d, on the has table.", teid);
        return NULL;
    }
    s->pendingRsp=0;
    return s;
}

struct SessionStruct_t *getPendingResponseByUES1APID(struct mme_t *mme, uint32_t mME_UE_S1AP_ID){
    struct SessionStruct_t *s = NULL;
    if(mME_UE_S1AP_ID!=0){
        HASH_FIND(hh2, mme->sessionht_byS1APID, &mME_UE_S1AP_ID, sizeof(uint32_t), s);
    }
    if(s != NULL){
        if(s->user_ctx->S11MMETeid != 0){
            HASH_DELETE(hh1, mme->sessionht_byTEID, s);
        }
        if(s->user_ctx->mME_UE_S1AP_ID != 0){
            HASH_DELETE(hh2, mme->sessionht_byS1APID, s);
        }
    }else{
        log_msg(LOG_WARNING, 0, "Couldn't find the session of MME UE S1AP ID %u on the has table.", mME_UE_S1AP_ID);
        return NULL;
    }
    s->pendingRsp=0;
    return s;
}

unsigned int getNextSeq(struct mme_t *mme){
    return mme->seq++;
}

unsigned int newTeid(){
    static uint32_t i = 1;
    return i++;
}

uint32_t getNewLocalUEid(struct  SessionStruct_t  * s){
    struct mme_t *mme = s->sessionHandler->engine->mme;

    uint32_t i;
    for (i=FIRST_UE_SCTP_STREAM; i<MAX_UE;i++){
        if(mme->s1apUsersbyLocalID[i]==NULL){
            mme->s1apUsersbyLocalID[i]= s;
            return i;
        }
    }
    log_msg(LOG_WARNING, 0, "Maximum number of UE (%u) reached, using stream 0", i);
    return 0;
}

struct EndpointStruct_t *get_ep_with_GlobalID(struct mme_t *mme, TargeteNB_ID_t *t){
    GHashTableIter iter;
    gpointer v;
    struct EndpointStruct_t* ep;
    gboolean found = FALSE;
    Global_ENB_ID_t *id1, *id2;


    id1 = t->global_ENB_ID;

    g_hash_table_iter_init (&iter, mme->s1ap);

    while (g_hash_table_iter_next (&iter, NULL, &v)){
        ep = (struct EndpointStruct_t*)v;
        id2 = ((S1_EndPoint_Info_t*)ep->info)->global_eNB_ID;

        if( memcmp(id1->pLMNidentity->tbc.s, id2->pLMNidentity->tbc.s, 3) ==0 &&
            memcmp(id1->eNBid, id2->eNBid, sizeof(ENB_ID_t)) == 0 ){
            found = TRUE;
            break;
        }
    }

    if (found)
        return ep;
    else
        return NULL;
}

const ServedGUMMEIs_t *mme_getServedGUMMEIs(const struct mme_t *mme){
     return mme->servedGUMMEIs;
 }
