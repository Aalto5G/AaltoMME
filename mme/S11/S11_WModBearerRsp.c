/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S11_WModBearerRsp.c
 * @Author Vicent Ferrer
 * @date   June, 2015
 * @brief  S11 State
 *
 */

#include "S11_WModBearerRsp.h"
#include "logmgr.h"
#include "S11_FSMConfig.h"
#include "gtp.h"


static void processMsg(gpointer self){
	GError *err = NULL;
	parseIEs(self);
	switch(getMsgType(self)){
	case GTP2_MODIFY_BEARER_RSP:
		if(!accepted(self)){
			log_msg(LOG_WARNING, 0, "Create Session request rejected "
			        "Cause %d", cause(self));
		}
		parseModBearerRsp(self, &err);
		if(err!=NULL){
			log_msg(LOG_ERR, 0, err->message);
			g_error_free (err);
			return;
		}
		s11changeState(self, ctx);
		returnControl(self);
        break;
	case GTP2_RELEASE_ACCESS_BEARERS_RSP:
		if(!accepted(self)){
			log_msg(LOG_WARNING, 0, "Releas Access Bearer rejected "
			        "Cause %d", cause(self));
		}
		s11changeState(self, ulCtx);
		returnControl(self);
		break;
	default:
		log_msg(LOG_DEBUG, 0, "Msg for this state not Implemented");
		break;
	}
}

static void attach(gpointer self){
	log_msg(LOG_ERR, 0, "Not Implemented");
}

static void detach(gpointer self){
	log_msg(LOG_ERR, 0, "Not Implemented");
}

static void modBearer(gpointer self){
	log_msg(LOG_ERR, 0, "Not Implemented");
}

static void releaseAccess(gpointer self){
	log_msg(LOG_ERR, 0, "Not Implemented");
}

void linkWModBearerRsp(S11_State* s){
	s->processMsg = processMsg;
	s->attach = attach;
	s->detach =  detach;
	s->modBearer = modBearer;
	s->releaseAccess = releaseAccess;
}
