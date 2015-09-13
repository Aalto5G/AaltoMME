/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   ESM_Inactive.c
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  ESM State
 *
 */

#include "ESM_Inactive.h"
#include "logmgr.h"
#include "ESM_FSMConfig.h"

static void processMsg(gpointer self){
	log_msg(LOG_ERR, 0, "Not Implemented");
}

static void activateDefault(gpointer self){
	/* ESM_BearerContext_t bearer; */
	/* self->s11 = S11_newUserAttach(esm_getS11iface(self->esm), bearer->esm->emm, self, */
	/*                               esm_sendActivateDefaultEPSBearerCtxtReq, self); */
	//esm_DefaultEPSBearerContextActivation(self);
	esmChangeState(self, ActivePending);
	/*esm_send(self);*/
}


void linkESMInactive(ESM_State* s){
	s->processMsg = processMsg;
	s->activateDefault = activateDefault;
}
