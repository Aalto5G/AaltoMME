/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   MME_S1.c
 * @Author Vicent Ferrer
 * @date   May, 2013
 * @brief  MME S1 interface protocol state machine.
 *
 * This module implements the S1 interface state machine.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "signals.h"

//#include "MME.h"
//#include "NAS_FSM.h"
#include "S1AP.h"
#include "MME_S1.h"
#include "MME_S1_priv.h"
#include "logmgr.h"
#include "hmac_sha2.h"
#include "S1Assoc.h"
#include "S1Assoc_FSMConfig.h"


/** Macro to check mandatory IE presence */
#define CHECKIEPRESENCE(p) \
    if(p==NULL){log_msg(LOG_ERR, 0, "IE not found on message"); return 1; }

/** Dummy null function callback */
typedef void (*nullparam_cb_cast)(void *);

/** S1 Accept function callback. Used to accept a new S1-MME connection
 * (from eNB)*/
void s1_accept_new_eNB(evutil_socket_t ss, short event, void *arg){

    struct S1_t *self = (struct S1_t*)arg;
    S1Assoc assoc;
    log_msg(LOG_DEBUG, 0, "enter s1_accept_new_eNB()");

    assoc = s1Assoc_init(self);
    s1Assoc_accept(assoc, ss);
}


gpointer s1_init(gpointer mme){
    S1_t *self = g_new0(S1_t, 1);

    self->mme = mme;

    s1ConfigureFSM();

    /*Init S1 server*/
    self->fd =init_sctp_srv(mme_getLocalAddress(self->mme), S1AP_PORT);
    log_msg(LOG_INFO, 0, "Open S1 server on file descriptor %d, port %d",
            self->fd, S1AP_PORT);

    mme_registerRead(self->mme, self->fd, s1_accept_new_eNB, self);

    self->assocs = g_hash_table_new_full( g_int_hash,
                                          g_int_equal,
                                          NULL,
                                          s1Assoc_free);

    return self;
}

void s1_free(S1 s1_h){
    S1_t *self = (S1_t *) s1_h;

    g_hash_table_destroy(self->assocs);
    mme_deregisterRead(self->mme, self->fd);
    close(self->fd);
    s1DestroyFSM();
    g_free(self);
}

void s1_registerAssoc(S1 s1_h, gpointer assoc){
    S1_t *self = (S1_t *) s1_h;

    /*Store the new connection*/
    g_hash_table_insert(self->assocs, s1Assoc_getfd_p(assoc), assoc);
}

void s1_deregisterAssoc(S1 s1_h, gpointer assoc){
    S1_t *self = (S1_t *) s1_h;
    g_hash_table_remove(self->assocs, s1Assoc_getfd_p(assoc));
}


struct mme_t *s1_getMME(S1_t *self){
    return self->mme;
}

/* ======================================================================
 * S1 Private Tool functions
 * ====================================================================== */

/**
 * @brief generate_NH -
 * @param [inout]  sec      security context
 *
 * fill the security context with a new derived NH (next hop) parameter with
 * the KDF function
 */
static void refresh_NH(SecurityCtx_t* sec){

    /*
    -    FC = 0x12
    -    P0 = SYNC-input
    -    L0 = length of SYNC-input (i.e. 0x00 0x20)
    The SYNC-input parameter shall be the newly derived K eNB  for the initial NH derivation, and the previous NH for all
    subsequent derivations. This results in a NH chain, where the next NH is always fresh and derived from the previous
    NH.
     */

    uint8_t s[35], zero[32];
    memset(zero, 0, 32);

    s[0]=0x12;
    if(memcmp(sec->nh, zero, 32)==0){
        /*First hop*/
        memcpy(s+1, sec->keNB, 32);
    }else{
        memcpy(s+1, sec->nh, 32);
        sec->ncc++;
    }
    s[33]=0;
    s[34]=0x20;

    hmac_sha256(sec->kASME, 32, s, 35, sec->nh, 32);
}
