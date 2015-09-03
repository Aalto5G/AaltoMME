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

#include "S1Assoc_priv.h"
#include "ECMSession.h"
#include "ECMSession_priv.h"
#include "ECMSession_FSMConfig.h"
#include "S1AP.h"
#include "logmgr.h"
#include "NAS_EMM.h"
#include "MME_S1_priv.h"

#include "hmac_sha2.h"


/* API to S1AP */
ECMSession ecmSession_init(S1Assoc s1, guint32 l_id){
    ECMSession_t *self = g_new0(ECMSession_t, 1);
    self->assoc = s1;
    self->mmeUEId = l_id;
    self->r_sid_valid = FALSE;

    /* Initial state for ECM FSM*/
    ecm_ChangeState(self, ECM_Idle);

    /* Configure high layer*/
    self->emm = emm_init(self);

    s1Assoc_registerECMSession(s1, self);

    return self;
}

void ecmSession_free(ECMSession h){
    ECMSession_t *self = (ECMSession_t *)h;
    s1Assoc_deregisterECMSession(self->assoc, self);
    emm_free(self->emm);
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
    S1AP_Message_t *s1out;

    MME_UE_S1AP_ID_t *mmeUEId;
    ENB_UE_S1AP_ID_t *eNBUEId;
    Unconstrained_Octed_String_t *nAS_PDU;

    ECMSession_t *self = (ECMSession_t *)h;

    s1out = S1AP_newMsg();
    s1out->choice = initiating_message;
    s1out->pdu->procedureCode = id_downlinkNASTransport;
    s1out->pdu->criticality = ignore;

    /* MME-UE-S1AP-ID*/
    mmeUEId = s1ap_newIE(s1out, id_MME_UE_S1AP_ID, mandatory, reject);
    mmeUEId->mme_id = self->mmeUEId;

    /* eNB-UE-S1AP-ID*/
    eNBUEId = s1ap_newIE(s1out, id_eNB_UE_S1AP_ID, mandatory, reject);
    eNBUEId->eNB_id = self->eNBUEId;

    /* NAS-PDU*/
    nAS_PDU = s1ap_newIE(s1out, id_NAS_PDU, mandatory, reject);
    nAS_PDU->len = len;
    nAS_PDU->str = msg;

    /*s1out->showmsg(s1out);*/
    s1Assoc_send(self->assoc, self->l_sid, s1out);
    s1out->freemsg(s1out);
}


const guint8 *ecmSession_getServingNetwork_TBCD(const ECMSession h){
    ECMSession_t *self = (ECMSession_t *)h;
    return self->tAI.sn;
}

/**
 * @brief generate_KeNB - KDF function to derive the K_eNB
 * @param [in]  kasme       derived key - 256 bits
 * @param [in]  ulNASCount  Uplink NAS COUNT
 * @param [out] keNB        eNB result key - 256 bits
 */
void generate_KeNB(const uint8_t *kasme, const uint32_t ulNASCount, uint8_t *keNB){

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

gpointer ecmSession_getS6a(const ECMSession h){
    ECMSession_t *self = (ECMSession_t *)h;
    struct mme_t *mme;
    S1 s1 = s1Assoc_getS1(self->assoc);
    mme = s1_getMME(s1);
    return  mme_getS6a(mme);
}

gpointer ecmSession_getS11(const ECMSession h){
    ECMSession_t *self = (ECMSession_t *)h;
    struct mme_t *mme;
    S1 s1 = s1Assoc_getS1(self->assoc);
    mme = s1_getMME(s1);
    return  mme_getS11(mme);
}
