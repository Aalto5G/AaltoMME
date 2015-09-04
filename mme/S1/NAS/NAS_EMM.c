/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   NAS_EMM.c
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  NAS EMM implementation
 *
 * This module implements the NAS EMM interface state machine on the MME EndPoint.
 */

#include <glib.h>
#include "NAS_EMM_priv.h"
#include "NAS.h"
#include "logmgr.h"
#include "EMMCtx.h"
#include "ECMSession_priv.h"
#include "NAS_ESM.h"
#include "EMM_State.h"

gpointer emm_init(gpointer ecm){
    EMMCtx_t *self = emmCtx_init();
    emmConfigureFSM();
    emmChangeState(self, EMM_Deregistered);
    self->ecm = ecm;
    self->s6a = ecmSession_getS6a(ecm);
    self->esm = esm_init(self);
    return self;
}

void emm_free(gpointer emm_h){
    EMMCtx_t *self = (EMMCtx_t*)emm_h;
    esm_free(self->esm);
    emmDestroyFSM();
    emmCtx_free(self);
}

gpointer emm_getS11(gpointer emm_h){
	EMMCtx_t *self = (EMMCtx_t*)emm_h;
	return ecmSession_getS11(self->ecm);
}


void emm_processMsg(gpointer emm_h, gpointer buffer, gsize len){
    EMMCtx_t *self = (EMMCtx_t*)emm_h;
    GenericNASMsg_t msg;
    dec_NAS(&msg, buffer, len);

    self->state->processMsg(self, &msg);
}

void emm_sendAuthRequest(EMMCtx emm_h){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    guint8 *pointer;
    guint8 buffer[150];
    AuthQuadruplet *sec;

    log_msg(LOG_DEBUG, 0, "Initiating UE authentication");

    /* Build Auth Request*/
    sec = (AuthQuadruplet *)g_ptr_array_index(emm->authQuadrs,0);

    pointer = buffer;
    newNASMsg_EMM(&pointer, EPSMobilityManagementMessages, PlainNAS);

    encaps_EMM(&pointer, AuthenticationRequest);

    /* NAS Key Set ID */
    nasIe_v_t1_l(&pointer, emm->ksi&0x0F);
    pointer++; /*Spare half octet*/

    /* RAND */
    nasIe_v_t3(&pointer, sec->rAND, 16); /* 256 bits */

    /* AUTN */
    nasIe_lv_t4(&pointer, sec->aUTN, 16); /* 256 bits */

    ecm_send(emm->ecm, buffer, pointer-buffer);
    emmChangeState(emm, EMM_CommonProcedureInitiated);
}

void emm_setSecurityQuadruplet(EMMCtx emm_h){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;

    AuthQuadruplet *sec;
    sec = (AuthQuadruplet *)g_ptr_array_index(emm->authQuadrs,0);

    if(emm->ksi < 6){
        emm->old_ksi = emm->ksi;
        emm->old_ncc = emm->ncc;
        memcpy(emm->old_kasme, emm->kasme, 32);
        memcpy(emm->old_nh, emm->nh, 32);
        emm->ksi++;
    }else{
        emm->ksi = 1;
    }

    memcpy(emm->kasme, sec->kASME, 32);

    g_ptr_array_remove_index(emm->authQuadrs, 0);
}

void emm_sendSecurityModeCommand(EMMCtx emm_h){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    guint8 *pointer, algorithms;
    guint16 capabilities;
    guint32 mac;
    guint8 count, buffer[150], req;
    memset(buffer, 0, 150);

     /* Build Security Mode Command*/
    pointer = buffer;
    newNASMsg_EMM(&pointer, EPSMobilityManagementMessages,
                  IntegrityProtectedWithNewEPSSecurityContext);

    mac = 0;
    nasIe_v_t3(&pointer, (uint8_t *)&mac, 4);

    count = emm->nasDlCount % 0xff;
    nasIe_v_t3(&pointer, (uint8_t*)&count, 1);


    newNASMsg_EMM(&pointer, EPSMobilityManagementMessages, PlainNAS);

    encaps_EMM(&pointer, SecurityModeCommand);

    /* Selected NAS security algorithms */
    algorithms=0;
    nasIe_v_t3(&pointer, &algorithms, 1);

    /*NAS key set identifier*/
    nasIe_v_t1_l(&pointer, (emm->ksi)&0x0F);
    pointer++; /*Spare half octet*/

    /* Replayed UE security capabilities */
    capabilities = hton16(emm->ueCapabilities);
    nasIe_lv_t4(&pointer, (uint8_t*)&capabilities, 2); /* 256 bits */

    /* IMEISV request */
    req = 0xc0&0xf0  /* Type*/
        | 0          /* Spare */
        | 0x01&0x07; /* Request*/
    nasIe_v_t3(&pointer, &req, 1);

    ecm_send(emm->ecm, buffer, pointer-buffer);
    emmChangeState(emm, EMM_CommonProcedureInitiated);

    /* Set timer T3460*/
}

void emm_processFirstESMmsg(EMMCtx emm_h){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    GByteArray *esmRaw;
    gsize len;
    esmRaw = g_ptr_array_index(emm->pendingESMmsg, 0);

    esm_processMsg(emm->esm, esmRaw->data, esmRaw->len);

    g_ptr_array_remove_index(emm->pendingESMmsg, 0);
}

void emm_attachAccept(EMMCtx emm_h, gpointer esm_msg, gsize len){
	EMMCtx_t *emm = (EMMCtx_t*)emm_h;

	emm->state->attachAccept(emm, esm_msg, len);
}
