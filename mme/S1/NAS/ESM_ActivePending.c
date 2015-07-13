/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   ESM_ActivePending.c
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  ESM State
 *
 */

#include "ESM_ActivePending.h"
#include "logmgr.h"
#include "ESM_FSMConfig.h"

static void processMsg(gpointer self){
	log_msg(LOG_ERR, 0, "Not Implemented");
}


void linkESMActivePending(ESM_State* s){
	s->processMsg = processMsg;
	s->activateDefault = notImplemented;
}
