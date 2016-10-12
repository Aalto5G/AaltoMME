/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */
/**
 * @file   S11_Peer.c
 * @Author Vicent Ferrer
 * @date   October, 2016
 * @brief  S11 peer mgmt.
 *
 */
#include <stdlib.h>
#include <string.h>

#include "S11_Peer.h"
#include "MME_S11.h"
#include "logmgr.h"

static guint s11peer_hash(gconstpointer key){
    Peer_t* k = (Peer_t*)key;
    guint res = 0;
    GBytes * a = g_bytes_new(&k->addr, k->len);
    res = g_bytes_hash(a);
    g_bytes_unref(a);
    return res;
}

static gboolean s11peer_equal(gconstpointer a, gconstpointer b){
    Peer_t* l = (Peer_t*)a;
    Peer_t* r = (Peer_t*)b;
    return l->len == r->len && memcmp(&l->addr, &r->addr, l->len)==0;
}

GHashTable *s11peer_buildTable(){
    return g_hash_table_new_full( s11peer_hash,
                                  s11peer_equal,
                                  NULL,
                                  (GDestroyNotify)free);
}

void s11peer_destroyTable(GHashTable *peers){
    g_hash_table_destroy(peers);
}

gboolean s11peer_isFirstSession(GHashTable *peers,
                                const struct sockaddr *rAddr,
                                const socklen_t rAddrLen,
                                Peer_t **p){
    Peer_t key = {.len = rAddrLen}, *_p;

    memcpy(&key.addr, rAddr, rAddrLen);
    *p = g_hash_table_lookup(peers, &key);
    if(*p!= NULL){
        (*p)->num_sessions++;
        return FALSE;
    }
    _p = malloc(sizeof(Peer_t));

    _p->len = rAddrLen;
    memcpy(&_p->addr, rAddr, rAddrLen);
    _p->num_sessions=1;
    _p->restartCounter = 0;
    _p->restartValid = FALSE;
    g_hash_table_insert(peers, _p, _p);
    *p = _p;
    return TRUE;
}

gboolean s11peer_hasRestarted(GHashTable *peers,
                              const struct sockaddr *rAddr,
                              const socklen_t rAddrLen,
                              const guint8 counter){
    Peer_t *p;
    Peer_t key = {.len = rAddrLen};
    gboolean ret;
    memcpy(&key.addr, rAddr, rAddrLen);
    p = g_hash_table_lookup(peers, &key);
    if(!p) return FALSE; /* The node is not being tracked*/

    if(counter > p->restartCounter ||
       (counter<2 && p->restartCounter>254)/*OverFlow*/){
        p->restartCounter=counter;
        ret = (p->restartValid)? TRUE:FALSE;
        p->restartValid = TRUE;
        return ret;
    }else if(counter < p->restartCounter){
        log_msg(LOG_WARNING, 0,
                "Received restart counter is lower that the stored counter. Ignoring");
    }
    p->restartValid = TRUE; /* Case counter = 0 and p->restartCounter = 0*/
    return FALSE;
}

Peer_t *s11peer_get(GHashTable *peers,
                    const struct sockaddr *rAddr,
                    const socklen_t rAddrLen){
    Peer_t key = {.len = rAddrLen};
    memcpy(&key.addr, rAddr, rAddrLen);
    return g_hash_table_lookup(peers, &key);
}

static void _s11peer_processEchoRsp(Peer_t *p, union gtp_packet *msg, size_t msg_len){
    union gtpie_member *echo_ie[GTPIE_SIZE];
    guint8             value[GTP2IE_MAX] = {0};
    guint16            vsize = 0;
    char               addrStr[INET6_ADDRSTRLEN];

    /*Restart timer*/
    s11peer_untrack(p);
    s11peer_track(p);

    gtp2ie_decap(echo_ie, msg, msg_len);
    gtp2ie_gettliv(echo_ie,  GTPV2C_IE_RECOVERY, 0, value, &vsize);
    log_msg(LOG_INFO, 0, "Received ECHO RSP from %s, recovery %u",
            inet_ntop(p->addr.sa_family,
                      &((struct sockaddr_in*)&p->addr)->sin_addr,
                      addrStr,
                      p->len),
            value[0]);

    S11_checkPeerRestart(p->s11, &p->addr, p->len, value[0], NULL);
}

void s11peer_processEchoRsp(GHashTable *peers,
                            const struct sockaddr *rAddr,
                            const socklen_t rAddrLen,
                            union gtp_packet *msg,
                            size_t msg_len){
    Peer_t *p = s11peer_get(peers, rAddr, rAddrLen);
    _s11peer_processEchoRsp(p, msg, msg_len);
}


void s11peer_sendEcho(Timer t, void* arg){
    Peer_t *p = (Peer_t *)arg;
    int fd;
    const struct timeval tv = {.tv_sec = 20, .tv_usec = 0};

    log_msg(LOG_INFO, 0, "Sending S11 ECHO REQ (test), sessions %u", p->num_sessions);

    /* Open UDP socket*/
    /* if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { */
    /*     log_msg(LOG_ERR, errno, */
    /*             "socket(domain=%d, type=%d, protocol=%d) failed", AF_INET, SOCK_DGRAM, 0); */
    /*     return; */
    /* } */
    /* Send Echo Req*/

    /* Next iteration*/
    p->t = tm_add_timer(p->tm, &tv, 1, s11peer_sendEcho, NULL, NULL, p);
}


void s11peer_track(Peer_t *p){
    const struct timeval tv = {.tv_sec = 20, .tv_usec = 0};
    p->t = tm_add_timer(p->tm, &tv, 1, s11peer_sendEcho, NULL, NULL, p);
}

void s11peer_untrack(Peer_t *p){
    tm_stop_timer(p->t);
}
