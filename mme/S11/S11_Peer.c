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
                                Peer_t *p){

    Peer_t key = {.len = rAddrLen};
    memcpy(&key.addr, rAddr, rAddrLen);
    p = g_hash_table_lookup(peers, &key);
    if(p!= NULL){
        p->num_sessions++;
        return FALSE;
    }
    p = malloc(sizeof(Peer_t));
    p->len = rAddrLen;
    memcpy(&p->addr, rAddr, rAddrLen);
    p->num_sessions=1;
    p->restartCounter = 0;
    p->restartValid = FALSE;

    g_hash_table_insert(peers, p, p);
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
