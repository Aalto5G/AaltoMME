/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   EMMCtx.c
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  EMMCtx Information
 *
 * This module implements the EMMCtx Information
 */

#include <string.h>
#include "EMMCtx.h"
#include "logmgr.h"
#include "ECMSession_priv.h"

#include <time.h>
#include <stdlib.h>

void freeESMmsg(gpointer msg){
    GByteArray *array = (GByteArray *)msg;
    g_byte_array_free(array, TRUE);
}

EMMCtx emmCtx_init(){
    EMMCtx_t *self = g_new0(EMMCtx_t, 1);

    self->subs = subs_init();

    self->authQuadrs = g_ptr_array_new_full (5, g_free);
    self->authQuints = g_ptr_array_new_full (5, g_free);

    self->pendingESMmsg = g_ptr_array_new_full (5, freeESMmsg);
    self->attachStarted = FALSE;

    self->ksi = 7;
    self->msg_ksi = 1;

    self->t3412 = 0x0A; /* 20 seg*/
    /* t3412 = 0x21; /\* 1 min*\/ */
    /* t3412 = 0x23; /\* 3 min*\/ */
    /* t3412 = 0x49; /\* 54 min, default *\/ */

    return self;
}

void emmCtx_free(EMMCtx s){
    EMMCtx_t *self = (EMMCtx_t*)s;
    g_ptr_array_free (self->authQuadrs, TRUE);
    g_ptr_array_free (self->authQuints, TRUE);
    g_ptr_array_free (self->pendingESMmsg, TRUE);

    subs_free(self->subs);
    g_free(self);
}

void emm_setState(EMMCtx emm_h, EMM_State *s){
    EMMCtx_t *self = (EMMCtx_t*)emm_h;
    self->state = s;
}

const guint64 emmCtx_getIMSI(const EMMCtx emm){
    EMMCtx_t *self = (EMMCtx_t*)emm;
    return self->imsi;
}

const guint64 emmCtx_getMSISDN(const EMMCtx emm){
    EMMCtx_t *self = (EMMCtx_t*)emm;
    return self->msisdn;
}

void emmCtx_setNewAuthQuadruplet(EMMCtx emm, AuthQuadruplet *a){
    EMMCtx_t *self = (EMMCtx_t*)emm;
    g_ptr_array_add(self->authQuadrs, a);
    self->authQuadrsLen++;
}

void emmCtx_freeAuthQuadruplet(EMMCtx emm){
    EMMCtx_t *self = (EMMCtx_t*)emm;
    g_ptr_array_remove_range (self->authQuadrs,
                          0,
                          self->authQuadrsLen);
    self->authQuadrsLen=0;
}

const AuthQuadruplet *emmCtx_getFirstAuthQuadruplet(EMMCtx emm){
    EMMCtx_t *self = (EMMCtx_t*)emm;
    return g_ptr_array_index(self->authQuadrs, 0);
}

const guint8 *emmCtx_getServingNetwork_TBCD(const EMMCtx emm){
    EMMCtx_t *self = (EMMCtx_t*)emm;
    return ecmSession_getServingNetwork_TBCD(self->ecm);
}

Subscription emmCtx_getSubscription(const EMMCtx emm){
    EMMCtx_t *self = (EMMCtx_t*)emm;
    return self->subs;
}

void emmCtx_setMSISDN(EMMCtx emm, guint64 msisdn){
    EMMCtx_t *self = (EMMCtx_t*)emm;
    self->msisdn = msisdn;
}

void emmCtx_newGUTI(EMMCtx emm, guti_t *guti){
    EMMCtx_t *self = (EMMCtx_t*)emm;

    if(self->guti.mtmsi==0){
        ecmSession_newGUTI(self->ecm, &(self->guti));
    }

    if(guti!=NULL)
        memcpy(guti, &(self->guti), sizeof(guti_t));
}

const guti_t * emmCtx_getGUTI(const EMMCtx emm){
    EMMCtx_t *self = (EMMCtx_t*)emm;
    return  &(self->guti);
}

guint32 *emmCtx_getM_TMSI_p(const EMMCtx emm){
    EMMCtx_t *self = (EMMCtx_t*)emm;
    return &(self->guti.mtmsi);
}

void emmCtx_replaceEMM(EMMCtx_t **emm, EMMCtx_t *old_emm){
    EMMCtx_t *temp = *emm;
    GPtrArray *tmp;
    *emm = old_emm;

    old_emm->attachStarted = temp->attachStarted;
    old_emm->attachResult =  temp->attachResult;

    tmp = old_emm->pendingESMmsg;
    old_emm->pendingESMmsg = temp->pendingESMmsg;
    temp->pendingESMmsg = tmp;

    old_emm->ueCapabilitiesLen = temp->ueCapabilitiesLen;
    memcpy(old_emm->ueCapabilities,
           temp->ueCapabilities,
           temp->ueCapabilitiesLen);

    old_emm->state = temp->state;
    old_emm->ecm = temp->ecm;
    ecmSession_setEMM(temp->ecm, old_emm);

    old_emm->msg_ksi = temp->msg_ksi;

    emm_free(temp);
}
