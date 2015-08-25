/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   ECMSession.c
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief
 *
 *
 */

#include <glib.h>

#include "ECMSession.h"
#include "ECMSession_priv.h"
#include "ECMSession_FSMConfig.h"
#include "S1AP.h"
#include "logmgr.h"

#include "hmac_sha2.h"


/* API to S1AP */
ECMSession ecmSession_init(S1Assoc s1, guint32 l_id){
    ECMSession_t *self = g_new0(ECMSession_t, 1);
    self->assoc = s1;
    self->mmeUEId = l_id;
    ecm_ChangeState(self, ECM_Idle);
    return self;
}

void ecmSession_free(ECMSession h){
    ECMSession_t *self = (ECMSession_t *)h;
    g_free(self);
}

void ecmSession_processMsg(ECMSession h, S1AP_Message_t *s1msg, int r_sid){
    ECMSession_t *self = (ECMSession_t *)h;

    self->state->processMsg(self, s1msg, r_sid);
}

void ecmSession_setState(ECMSession ecm, ECMSession_State *s){
    ECMSession_t *self = (ECMSession_t *)ecm;
    self->state = s;
}

const guint32 ecmSession_getMMEUEID(const ECMSession h){
    ECMSession_t *self = (ECMSession_t *)h;
    return self->mmeUEId;
}

guint32 *ecmSession_getMMEUEID_p(const ECMSession h){
    ECMSession_t *self = (ECMSession_t *)h;
    return &(self->mmeUEId);
}

/* API to NAS */
void ecm_send(ECMSession h, gpointer msg, size_t len){

}
