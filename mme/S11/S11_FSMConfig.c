/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S11_FSMConfig.c
 * @Author Vicent Ferrer
 * @date   June, 2015
 * @brief  Functions to modify S11 FSM
 *
 */


//#include "MME_S11.c"
#include "S11_FSMConfig.h"

#include "S11_NoCtx.h"
#include "S11_WCtxRsp.h"
#include "S11_UlCtx.h"
#include "S11_WModBearerRsp.h"
#include "S11_Ctx.h"
#include "S11_WDel.h"

S11_State *states;

void s11ConfigureFSM(){
	states = g_new(S11_State, 6);
	
	linkNoCtx(&states[0]);
	linkWCtxRsp(&states[1]);
	linkUlCtx(&states[2]);
	linkWModBearerRsp(&states[3]);
	linkCtx(&states[4]);
	linkWDel(&states[5]);
}

void s11DestroyFSM(){
	g_free(states);
}


void changeState(gpointer session, S11State s){
	s11u_setState(session, &(states[s]));
}
