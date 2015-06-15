/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S11_NoCtx.c
 * @Author Vicent Ferrer
 * @date   June, 2015
 * @brief  S11 State
 *
 */

#include "S11_NoCtx.h"

static void processMsg(gpointer){
	log_msg(LOG_ERROR, 0, "Not Implemented");
}

static void attach(gpointer self){
	log_msg(LOG_DEBUG, 0, "Bearer Context doesn't exist, creating a new one.");
	sendCreateSessionReq(self);
	changeState(self, wCtxRsp);
}

static void detach(gpointer){
	log_msg(LOG_ERROR, 0, "Not Implemented");
}

static void modBearer(gpointer){
	log_msg(LOG_ERROR, 0, "Not Implemented");
}


void linkNoCtx(S11_State* s){
	s->processMsg = processMsg;
	s->attach = attach;
	s->detach =  detach;
	s->modBearer = modBearer;
}

