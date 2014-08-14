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

#include "logmgr.h"
#include "gtp.h"
#include "commands.h"
#include "MME.h"
#include "MME_engine.h"
#include "MME_S1.h"
#include "MME_S11.h"
#include "MME_Controller.h"
#include "HSS.h"

int mme_run_flag=0;

/***********************************************
SIGINT handler
***********************************************/
void termination_handler (int signum)
{
    if(mme_run!=NULL){
        mme_run_flag=0;
        log_msg(LOG_INFO, 0, "SIGINT detected. Closing MME");
    }else{
        log_msg(LOG_INFO, 0, "SIGINT detected.");
        exit(0);
    }
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
int init_sctp_srv(int port){
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
    servaddr.sin_addr.s_addr = htonl( INADDR_ANY );
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
    initmsg.sinit_num_ostreams = 32;
    initmsg.sinit_max_instreams = 32;
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

    if (init_hss() != 0){
        return 1;
    }

    /*Init command server, returns server file descriptor*/
    self->command.fd = servcommand_init(COMMAND_PORT);
    self->command.portState=opened;
    log_msg(LOG_INFO, 0, "Open command server on file descriptor %d, port %d", self->command.fd, COMMAND_PORT);

    /*Init S11 server*/
    self->s11.fd =init_udp_srv(GTP2C_PORT);
    self->s11.portState=opened;
    log_msg(LOG_INFO, 0, "Open S11 server on file descriptor %d, port %d", self->s11.fd, GTP2C_PORT);

    /*Init S1 server*/
    self->s1.fd =init_sctp_srv(S1AP_PORT);
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

    disconnect_hss();

    if( self->command.portState != closed){
        close(self->command.fd);
        self->command.portState = closed;
    }
    if( self->s11.portState != closed){
        close(self->s11.fd);
        self->s11.portState = closed;
    }

    if( self->s1.portState != closed){
    	close(self->s1.fd);
        self->s1.portState = closed;
    }
    for(i=0; i<self->nums1conn;i++){
		ep = self->s1ap[i];
		free_S1_EndPoint_Info(ep->info);
		close(ep->fd);
		ep->portState = closed;
		free(ep);
	}
	free(self->s1ap);


    if( self->ctrl.portState != closed){
        close(self->ctrl.fd);
        self->ctrl.portState = closed;
    }

    return 0;
}

/**@brief Initialize mme structure
 */
int mme_run(struct mme_t *self){

    struct event_base       *base;                          /*< libevent base loop*/
    struct event            *listener_S11;                  /*< S11 Interface event*/
    struct event            *listener_S1;                   /*< S1 Interface event*/
    struct event            *listener_Ctrl;                 /*< listener_Ctrl Interface event*/
    struct event            *listener_command;              /*< Command Interface event*/

    int i = 0, allocated = 0;

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

    mme_init_ifaces(self);

    /* LibEvent Configuration*/
    /*Create event base structure*/
    self->evbase = event_base_new();
    if (!self->evbase){
        log_msg(LOG_ERR, 0, "Failed to create libevent event-base");
        return 1;
    }

    /*Create event for accepting connections*/
    listener_command = event_new(self->evbase, self->command.fd, EV_READ|EV_PERSIST, cmd_accept, self);
    listener_S11 = event_new(self->evbase, self->s11.fd, EV_READ|EV_PERSIST, s11_accept, self); /*mme possible arg in the future*/
    listener_S1 = event_new(self->evbase, self->s1.fd, EV_READ|EV_PERSIST, s1_accept_new_eNB, self); /*mme possible arg in the future*/
    listener_Ctrl = event_new(self->evbase, self->ctrl.fd, EV_READ|EV_PERSIST, ctrl_accept, self); /*mme possible arg in the future*/



    /*Make socket non blocking*/
    evutil_make_socket_nonblocking(self->command.fd);
    evutil_make_socket_nonblocking(self->s11.fd);
    evutil_make_socket_nonblocking(self->s1.fd);
    evutil_make_socket_nonblocking(self->ctrl.fd);


    /*Add command listener event to the event base*/
    event_add(listener_command, NULL);
    event_add(listener_S11, NULL);
    event_add(listener_S1, NULL);
    event_add(listener_Ctrl, NULL);


    engine_main(self);

    engine_process_stop(self->command.handler);
    engine_process_stop(self->s11.handler);
    engine_process_stop(self->ctrl.handler);

    if( self->s1.portState != closed){
        for(i=0; i<self->nums1conn;i++){
            engine_process_stop(self->s1ap[i]->handler);
            event_free(self->s1ap[i]->ev);
        }
    }

    event_free(listener_command);
    event_free(listener_S11);
    event_free(listener_S1);
    event_free(listener_Ctrl);
    event_base_free(self->evbase);

    mme_close_ifaces(self);

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
    new_action.sa_handler = termination_handler;
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = 0;

    sigaction (SIGINT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN){
        sigaction (SIGINT, &new_action, NULL);
    }

    /* run MME*/
    mme_run(mme);

    /*Close syslog entity*/
    close_logger();

    free(mme);

    mme_run_flag=0;

    return 0;
}

void free_mme(struct mme_t *self){
    int i=0;
    struct EndpointStruct_t *ep;

    if( self->command.portState != closed){
        close(self->command.fd);
        self->command.portState = closed;
    }
    if( self->s11.portState != closed){
        close(self->s11.fd);
        self->s11.portState = closed;
    }
    if( self->ctrl.portState != closed){
    	close(self->ctrl.fd);
    	self->ctrl.portState = closed;
    }

    if( self->s1.portState != closed){
        for(i=0; i<self->nums1conn;i++){
            ep = self->s1ap[i];
            free_S1_EndPoint_Info(ep->info);
            close(ep->fd);
            ep->portState = closed;

            free(ep);
        }
        free(self->s1ap);
    }

    if(self->servedGUMMEIs!=NULL)
        self->servedGUMMEIs->freeIE(self->servedGUMMEIs);
    if(self->relativeCapacity!=NULL)
        self->relativeCapacity->freeIE(self->relativeCapacity);
    if(self->name!=NULL)
        self->name->freeIE(self->name);
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
void s11_accept(evutil_socket_t listener, short event, void *arg){

    uint32_t teid;
    struct SessionStruct_t *session;
    Signal *output;
    struct t_message *msg;
    struct t_message msg1;

    struct mme_t *mme = (struct mme_t *)arg;

    msg = newMsg();
    msg->packet.gtp.flags=0x0;
    gtp2_recv(listener, &(msg->packet.gtp), &(msg->length), (struct sockaddr_in*)&(msg->peer.peerAddr), &(msg->peer.socklen));
    msg->peer.fd = listener;

    /*Debugging*/
    /*printf("S11_accept(): Packet received: \n");
    print_packet(&(msg->packet), msg->length);*/

    /*Is this necessary?*/
    msg->peer.portState = opened;
    /********************/

    if(msg->packet.gtp.gtp2s.h.type<4){
        /* TODO @Vicent :Manage echo request, echo response or version not suported*/
        log_msg(LOG_INFO, 0, "s11_accept() recv echo request, echo response or version not suported msg");
        print_packet(&(msg->packet), msg->length);
        return;
    }

    teid = ntoh32(msg->packet.gtp.gtp2l.h.tei);

    /*Look if there is any process waiting a response*/
    session = getPendingResponseByTEID(mme, teid);

    if (session){
        log_msg(LOG_INFO, 0, "s11_accept() Session found");
        if(session->sessionHandler->firstSignal!=NULL){
            /*removePendentResponse(session);*/
            log_msg(LOG_DEBUG, 0, "Found an associated signal.");
            output = session->sessionHandler->firstSignal->signal;
            session->sessionHandler->firstSignal = session->sessionHandler->firstSignal->next;
        }else{
            output = new_signal(session->sessionHandler);
            output->name=S11_handler_ready;
            output->priority = MAXIMUM_PRIORITY;
        }
    }
    else{
        /* Manage new request*/
        log_msg(LOG_INFO, 0, "s11_accept() Session not found");
        output = new_signal(S11_handler_create(mme->engine, NULL));
        output->name=S11_handler_ready;
        output->priority = MAXIMUM_PRIORITY;
    }
    output->data = (void *)msg;
    output->freedataFunc = freeMsg;
    signal_send(output);
}

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
    for(i=0; i<mme->nums1conn ; i++){
    	if(listener == mme->s1ap[i]->fd){
    	    ep_S1 = mme->s1ap[i];
    	    break;
    	}
    }

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

    /*Check errors*/
    if (msg->length <= 0) {
        /*Close socket*/
        close(listener);
        /*Delete event*/
        event_free(ep_S1->ev);
        log_msg(LOG_INFO, 0, "Connection closed");
        freeMsg(msg);
        return;
    }

    /* If the packet is not a s1ap packet, it is silently rejected*/
    if(sndrcvinfo.sinfo_ppid != SCTP_S1AP_PPID){
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

    tmp = (struct EndpointStruct_t**)realloc(mme->s1ap, (mme->nums1conn+1)*sizeof(struct EndpointStruct_t*) );
    if (tmp!=NULL) {
        mme->s1ap=tmp;
        mme->s1ap[mme->nums1conn] = news1ep;
        mme->nums1conn++;
    }
    else {
        free (mme->s1ap);
        log_msg(LOG_ERR, 0, "Error (re)allocating memory");
        exit (1);
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
