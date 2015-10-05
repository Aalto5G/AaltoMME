/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   MME_S6a.c
 * @Author Vicent Ferrer
 * @date   June, 2013
 * @brief  MME S6a interface protocol state machine.
 *
 * This module implements the S6a interface state machine.
 * It is currently only and emulator
 */

#include <stdio.h>
#include <stdlib.h>
#include "signals.h"

#include "MME.h"
#include "MME_S6a.h"
#include "logmgr.h"

#include "hmac_sha2.h"
#include "milenage.h"

#include "HSS.h"

G_DEFINE_QUARK(MME-S6a, mme_S6a);

typedef void (*s6a_STATE)(gpointer);

struct s6a_t{
    gpointer mme;
    s6a_STATE state;
};


/**
 * @brief initiate the S6a stack
 * @param [in]  mme   pointer to mme structure to access the API
 */
gpointer s6a_init(gpointer mme){
    struct s6a_t *s6a = g_new(struct s6a_t, 1);
    s6a->mme = mme;

    if (init_hss() != 0){
        return  NULL;
    }
    //return (gpointer) s6a;
    return s6a;
}

void s6a_free(gpointer s6a){
    disconnect_hss();
    g_free(s6a);
}

/**
 * @brief generate_KeNB - KDF function to derive the K_eNB
 * @param [in]  kasme       derived key - 256 bits
 * @param [in]  ulNASCount  Uplink NAS COUNT
 * @param [out] keNB        eNB result key - 256 bits
 */
static void generate_KeNB(const uint8_t *kasme, const uint32_t ulNASCount, uint8_t *keNB){

    /*
    FC = 0x11,
    P0 = Uplink NAS COUNT,
    L0 = length of uplink NAS COUNT (i.e. 0x00 0x04)
     */
    uint8_t s[7];
    s[0]=0x11;
    memcpy(s+1, &ulNASCount, 4);
    s[5]=0x00;
    s[6]=0x04;

    hmac_sha256(kasme, 32, s, 7, keNB, 32);
}


/* ====================================================================== */

static void s6a_errorTranslation(GError *diameter, GError **s6a){
    if(g_error_matches(diameter, DIAMETER, DIAMETER_UNKNOWN_EPS_SUBSCRIPTION)){
        g_set_error(s6a, MME_S6a, S6a_UNKNOWN_EPS_SUBSCRIPTION,
                    "Unknown EPS Subscription in HSS");
    }else{
        g_set_error(s6a, MME_S6a, S6a_UNKNOWN_ERROR,
                    "Unknown Error:");
    }
}

void s6a_GetAuthInformation(gpointer s6a_h, EMMCtx emm,
                            void(*cb)(gpointer),
                            void(*error_cb)(gpointer, GError *),
                            gpointer args){
    GError *err = NULL, *err_cb=NULL;
    log_msg(LOG_DEBUG, 0, "Enter S6a State Machine");

    HSS_getAuthVec(emm, &err);
    /*generate_KeNB(user->sec_ctx.kASME, user->sec_ctx.ulNAScnt, user->sec_ctx.keNB);*/
    if(err){
        log_msg(LOG_ERR, 0, err->message);
        if(error_cb){
            s6a_errorTranslation(err, &err_cb);
            error_cb(args, err_cb);
            g_error_free(err_cb);
        }
        g_error_free(err);
    }else{
        if(cb){
            cb(args);
        }
    }
}

void s6a_SynchAuthVector(gpointer s6a_h,  EMMCtx emm, uint8_t *auts,
                         void(*cb)(gpointer), gpointer args){

    struct s6a_t *s6a = (struct s6a_t*) s6a_h;

    HSS_syncAuthVec(emm, auts);
    //generate_KeNB(user->sec_ctx.kASME, user->sec_ctx.ulNAScnt, user->sec_ctx.keNB);
    cb(args);
}


void s6a_UpdateLocation(gpointer s6a_h, EMMCtx emm,
                        void(*cb)(gpointer), gpointer args){

    struct s6a_t *s6a = (struct s6a_t*) s6a_h;
    HSS_UpdateLocation(emm, mme_getServedGUMMEIs(s6a->mme));
    cb(args);
}

/* ====================================================================== */
