/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S11_Ctx.c
 * @Author Vicent Ferrer
 * @date   June, 2015
 * @brief  S11 State
 *
 */

#include "S11_Ctx.h"


static void processMsg(gpointer){
	log_msg(LOG_ERROR, 0, "Not Implemented");
}

static void attach(gpointer){
	log_msg(LOG_ERROR, 0, "Not Implemented");
}

static void detach(gpointer){
	log_msg(LOG_DEBUG, 0, "Deleting Bearer Context");
	sendDeleteSessionReq(self);
	changeState(self, wDel);
}

static void modBearer(gpointer){
	log_msg(LOG_ERROR, 0, "Not Implemented");
}


void linkCtx(S11_State* s){
	s->processMsg = processMsg;
	s->attach = attach;
	s->detach =  detach;
	s->modBearer = modBearer;
}
