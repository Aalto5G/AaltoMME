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
#include "MMEutils.h"
#include "S1Assoc.h"
#include "ECMSession.h"
#include "NAS_EMM.h"

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
int init_udp_srv(const char *src, int port){
    int fd;
    struct sockaddr_in addr;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        log_msg(LOG_ERR, errno, "socket(domain=%d, type=%d, protocol=%d) failed", AF_INET, SOCK_DGRAM, 0);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET, src, &(addr.sin_addr));

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
int init_sctp_srv(const char *src, int port){
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
    inet_pton(AF_INET, src, &(servaddr.sin_addr));
    /* servaddr.sin_addr.s_addr = addr; */
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
    uint32_t addr = 0;

    self->s6a = s6a_init((gpointer)self);

    /*Init command server, returns server file descriptor*/
    self->cmd = servcommand_init(self, COMMAND_PORT);

    /*Init S11 server*/
    self->s11 = s11_init((gpointer)self);

    /*Init S1 server*/
    self->s1 = s1_init((gpointer)self);

    /*Init Controller server*/
    self->sdnCtrl = sdnCtrl_init((gpointer)self);

    return 0;
}

int mme_close_ifaces(struct mme_t *self){

    struct EndpointStruct_t *ep;
    int i=0;

    sdnCtrl_free(self->sdnCtrl);
    s6a_free(self->s6a);
    servcommand_stop(self->cmd);
    s11_free(self->s11);
    s1_free(self->s1);
    return 0;
}


void mme_registerRead(struct mme_t *self, int fd, event_callback_fn cb, void * args){
    struct event *ev;
    int *_fd = g_new(gint, 1);
    *_fd = fd;
    log_msg(LOG_DEBUG, 0, "ENTER, fd %u", fd);
    ev = event_new(self->evbase, fd, EV_READ|EV_PERSIST, (event_callback_fn)cb, args);
    evutil_make_socket_nonblocking(fd);
    event_add(ev, NULL);
    g_hash_table_insert(self->ev_readers, _fd, ev);
}

void test_lprint(gpointer data, gpointer user){
    log_msg(LOG_DEBUG, 0, "Key: fd %u", *(int*)data);
}

void mme_deregisterRead(struct mme_t *self, int fd){
    if(g_hash_table_remove(self->ev_readers, &fd) != TRUE){
        log_msg(LOG_ERR, 0, "Unable to find read event, fd %u", fd);
        GList * l = g_hash_table_get_keys(self->ev_readers);
        g_list_foreach(l, test_lprint, NULL);
        g_list_free(l);
    }
}

struct event_base *mme_getEventBase(struct mme_t *self){
    return self->evbase;
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

    /*Init user storage*/
    init_storage_system();

    /*Init node manager*/
    init_nodemgr();

    /*Load MME information from config file*/
    loadMMEinfo(self);

    self->ev_readers = g_hash_table_new_full( g_int_hash,
                                              g_int_equal,
                                              g_free,
                                              (GDestroyNotify) event_free);

    self->s1_localIDs = g_hash_table_new_full(g_int_hash,
                                              g_int_equal,
                                              g_free,
                                              NULL);

    self->ecm_sessions_by_localID = g_hash_table_new_full(g_int_hash,
                                                          g_int_equal,
                                                          NULL,
                                                          (GDestroyNotify)ecmSession_free);

    self->emm_sessions = g_hash_table_new_full(g_int_hash,
                                               g_int_equal,
                                               NULL,
                                               (GDestroyNotify) emm_free);

    self->s1_by_GeNBid = g_hash_table_new_full( (GHashFunc)  globaleNBID_Hash,
                                                (GEqualFunc) globaleNBID_Equal,
                                                NULL,
                                                NULL);

    /*Create event base structure*/
    self->evbase = event_base_new();
    if (!self->evbase){
        log_msg(LOG_ERR, 0, "Failed to create libevent event-base");
        if(allocated==1)
            free(self);
        return 1;
    }

    mme_init_ifaces(self);

    /*Create event for processing SIGINT*/
    kill_event = evsignal_new(self->evbase, SIGINT, kill_handler, self);
    event_add(kill_event, NULL);


    /* Loop blocking*/
    engine_main(self);

    mme_close_ifaces(self);

    event_free(kill_event);
    event_base_free(self->evbase);

    g_hash_table_destroy(self->s1_by_GeNBid);

    g_hash_table_destroy(self->emm_sessions);

    g_hash_table_destroy(self->ecm_sessions_by_localID);

    g_hash_table_destroy(self->s1_localIDs);

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

unsigned int newTeid(){
    static uint32_t i = 1;
    return i++;
}

uint32_t mme_newLocalUEid(struct mme_t *self){
    guint32 *i = g_new0(guint32, 1);
    for (*i=1; *i<=MAX_UE ;(*i)++){
        if(!g_hash_table_contains(self->s1_localIDs, i)){
            g_hash_table_add(self->s1_localIDs, i);
            log_msg(LOG_DEBUG, 0, "MME S1AP UE ID %u Chosen", *i);
            return *i;
        }
        /* log_msg(LOG_DEBUG, 0, "MME S1AP UE ID %u exists already", *i); */
    }
    g_free(i);
    log_msg(LOG_ERR, 0, "Maximum number of UE (%u) reached", *i);
    return 0;
}

void mme_freeLocalUEid(struct mme_t *self, uint32_t id){
    if(!g_hash_table_remove(self->s1_localIDs, &id)){
        log_msg(LOG_ERR, 0, "MME UE S1AP ID (%u) to be free not found", id);
    }
}

const ServedGUMMEIs_t *mme_getServedGUMMEIs(const struct mme_t *mme){
     return mme->servedGUMMEIs;
 }

const char *mme_getLocalAddress(const struct mme_t *mme){
    return mme->ipv4;
}

void mme_registerS1Assoc(struct mme_t *self, gpointer assoc){
    g_hash_table_insert(self->s1_by_GeNBid, s1Assoc_getID_p(assoc), assoc);
}

void mme_deregisterS1Assoc(struct mme_t *self, gpointer assoc){
    if(g_hash_table_remove(self->s1_by_GeNBid, s1Assoc_getID_p(assoc)) != TRUE){
        log_msg(LOG_ERR, 0, "Unable to find S1 Assoction");
    }
}


void mme_lookupS1Assoc(struct mme_t *self, gconstpointer geNBid, gpointer *assoc){
    *assoc = g_hash_table_lookup(self->s1_by_GeNBid, geNBid);
}


void mme_registerEMMCtxt(struct mme_t *self, gpointer emm){
    g_hash_table_insert(self->emm_sessions,
                        emm_getM_TMSI_p(emm),
                        emm);
}

void mme_deregisterEMMCtxt(struct mme_t *self, gpointer emm){
    if(g_hash_table_remove(self->emm_sessions, emm_getM_TMSI_p(emm)) != TRUE){
        log_msg(LOG_ERR, 0, "Unable to find EMM session");
    }
}

void mme_lookupEMMCtxt(struct mme_t *self, const guint32 m_tmsi, gpointer *emm){
    *emm = g_hash_table_lookup(self->emm_sessions, &m_tmsi);
}

void mme_registerECM(struct mme_t *self, gpointer ecm){
    g_hash_table_insert(self->ecm_sessions_by_localID,
                        ecmSession_getMMEUEID_p(ecm),
                        ecm);
}

void mme_deregisterECM(struct mme_t *self, gpointer ecm){
    if(g_hash_table_remove(self->ecm_sessions_by_localID,
                           ecmSession_getMMEUEID_p(ecm)) != TRUE){
        log_msg(LOG_ERR, 0, "Unable to find ECM session");
    }
}

void mme_lookupECM(struct mme_t *self, const guint32 id, gpointer *ecm){
    *ecm = g_hash_table_lookup(self->ecm_sessions_by_localID, &id);
}

GList *mme_getS1Assocs(struct mme_t *self){
    return g_hash_table_get_values(self->s1_by_GeNBid);
}

gpointer mme_getS6a(struct mme_t *self){
    return self->s6a;
}

gpointer mme_getS11(struct mme_t *self){
    return self->s11;
}

gboolean mme_GUMMEI_IsLocal(const struct mme_t *self,
                            const guint32 plmn,
                            const guint16 mmegi,
                            const guint8 mmec){
    return TRUE;
}


gboolean mme_containsSupportedTAs(const struct mme_t *self, SupportedTAs_t *tas){
    int i, j, k, l;
    BPLMNs_t *bc_l;
    PLMNidentity_t *plmn_eNB, *plmn_MME;
    ServedPLMNs_t *served;


    for(i=0; i<tas->size; i++){
        bc_l = tas->item[i]->broadcastPLMNs;
        for(j=0; j<bc_l->n ; j++){
            plmn_eNB = bc_l->pLMNidentity[j];
            for(k=0; k<self->servedGUMMEIs->size; k++){
                served = self->servedGUMMEIs->item[k]->servedPLMNs;
                for(l=0; l<served->size ; l++){
                    plmn_MME = served->item[l];
                    log_msg(LOG_DEBUG, 0, "Comparing SupportedTA with ServedGUMMEIs"
                            " PLMN %x%x%x <=> %x%x%x",
                            plmn_MME->tbc.s[0], plmn_MME->tbc.s[1], plmn_MME->tbc.s[2],
                            plmn_eNB->tbc.s[0], plmn_eNB->tbc.s[1], plmn_eNB->tbc.s[2]);
                    if(memcmp(plmn_MME->tbc.s, plmn_eNB->tbc.s, 3)==0){
                        return TRUE;
                    }
                }
            }
        }
    }
    return FALSE;
}
