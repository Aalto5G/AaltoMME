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
#include "logmgr.h"
#include "S11_FSMConfig.h"

static void processMsg(gpointer self){
	log_msg(LOG_ERR, 0, "Not Implemented");
}

static void attach(gpointer self){
	log_msg(LOG_ERR, 0, "Not Implemented");
}

static void detach(gpointer self){
	log_msg(LOG_DEBUG, 0, "Deleting Bearer Context");
	sendDeleteSessionReq(self);
	s11changeState(self, wDel);
}

static void modBearer(gpointer self){
	log_msg(LOG_DEBUG, 0, "Sending Modify Bearer Request");
	sendModifyBearerReq(self);
	s11changeState(self, wModBearerRsp);
}

static void releaseAccess(gpointer self){
	log_msg(LOG_ERR, 0, "Not Implemented");
	sendReleaseAccessBearersReq(self);
	s11changeState(self, wModBearerRsp);
}


void linkCtx(S11_State* s){
	s->processMsg = processMsg;
	s->attach = attach;
	s->detach =  detach;
	s->modBearer = modBearer;
	s->releaseAccess = releaseAccess;
}
