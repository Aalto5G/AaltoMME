/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S11_WDel.c
 * @Author Vicent Ferrer
 * @date   June, 2015
 * @brief  S11 State
 *
 */

#include "S11_WDel.h"


static void processMsg(gpointer){
	GError *err = NULL;
	parseIEs(self);
	switch(msg->packet.gtp.gtp2l.h.type){
	case GTP2_DELETE_SESSION_RSP:
		if(!accepted(self)){
			log_msg(LOG_WARNING, 0, "Create Session request rejected "
			        "Cause %d", cause(self));
		}
		parseDelCtxRsp(self, &err);
		if(err!=NULL){
			log_msg(LOG_ERROR, 0, err->message);
			g_error_free (err);
			return;
		}
		returnControlAndRemoveSession(self);	
        break;
	default:
		log_msg(LOG_DEBUG, 0, "Msg for this state not Implemented");
		break;
	}
}

static void attach(gpointer){
	
}

static void detach(gpointer){
	
}

static void modBearer(gpointer){
	
}


void linkWDel(S11_State* s){
	s->processMsg = processMsg;
	s->attach = attach;
	s->detach =  detach;
	s->modBearer = modBearer;
}
