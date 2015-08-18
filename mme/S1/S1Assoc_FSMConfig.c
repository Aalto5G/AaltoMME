/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S1Assoc_FSMConfig.c
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  Functions to modify S1Assoc FSM
 *
 */

#include "S1Assoc.h"
#include "S1Assoc_FSMConfig.h"
/* Include States*/
#include "S1Assoc_NotConfigured.h"
#include "S1Assoc_Active.h"

#include "logmgr.h"

S1Assoc_State *states;

void s1ConfigureFSM(){
	states = g_new(S1Assoc_State, 2);
	
	linkS1AssocNotConfigured(&states[0]);
	linkS1AssocActive(&states[1]);
}

void s1DestroyFSM(){
	g_free(states);
}


void s1ChangeState(gpointer s1, S1AssocState s){
	s1Assoc_setState(s1, &(states[s]));
}

void s1notImplemented(gpointer self){
	log_msg(LOG_ERR, 0, "Not Implemented");
}