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
#include "NAS_EMM.h"
#include "NAS.h"
#include "logmgr.h"
#include "EMMCtx.h"
#include "ECMSession_priv.h"

gpointer emm_init(gpointer ecm){
    EMMCtx_t *self = emmCtx_init();
    emmConfigureFSM();
    emmChangeState(self, EMM_Deregistered);
    self->ecm = ecm;
    self->s6a = ecmSession_getS6a(ecm);
    return self;
}

void emm_free(gpointer emm_h){
    EMMCtx_t *self = (EMMCtx_t*)emm_h;
    emmDestroyFSM();
    emmCtx_free(self);
}

void emm_processMsg(gpointer emm_h, gpointer buffer, size_t len){
    EMMCtx_t *self = (EMMCtx_t*)emm_h;
    GenericNASMsg_t msg;
    dec_NAS(&msg, buffer, len);

    log_msg(LOG_WARNING, 0, "Enter");

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
		emm->ksi = 0;
	}

	memcpy(emm->kasme, sec->kASME, 32);

	g_ptr_array_remove_index(emm->authQuadrs, 0);
}
