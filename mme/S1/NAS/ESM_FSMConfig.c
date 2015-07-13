/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   ESM_FSMConfig.c
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  Functions to modify ESM FSM
 *
 */


#include "ESM_FSMConfig.h"

#include "ESM_Inactive.h"
#include "ESM_ActivePending.h"
#include "ESM_Active.h"
#include "ESM_ModifyPending.h"
#include "ESM_InactivePending.h"

#include "logmgr.h"

ESM_State *states;

void esmConfigureFSM(){
	states = g_new(ESM_State, 5);
	
	linkESMInactive(&states[0]);
	linkESMActivePending(&states[1]);
	linkESMActive(&states[2]);
	linkESMModifyPending(&states[3]);
	linkESMInactivePending(&states[4]);
}

void esmDestroyFSM(){
	g_free(states);
}


void esmChangeState(gpointer session, ESMState s){
	esm_bc_setState(session, &(states[s]));
}

void notImplemented(gpointer self){
	log_msg(LOG_ERR, 0, "Not Implemented");
}
