/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   EMM_Registered.c
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  EMM State
 *
 */

#include "EMM_Registered.h"
#include "logmgr.h"
#include "EMM_FSMConfig.h"
#include "NAS.h"

static void emmProcessMsg(gpointer emm_h, GenericNASMsg_t* msg){
    log_msg(LOG_ERR, 0, "Not Implemented");
}


static void emm_processSecMsg(gpointer emm_h, gpointer buff, gsize len){
	EMMCtx_t *emm = (EMMCtx_t*)emm_h;

	log_msg(LOG_ERR, 0, "Received unexpected NAS message with security header");

}


void linkEMMRegistered(EMM_State* s){
    s->processMsg = emmProcessMsg;
    /* s->authInfoAvailable = emmAuthInfoAvailable; */
    s->attachAccept = NULL;
    s->processSecMsg = emm_processSecMsg;
}
