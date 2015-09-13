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
#include "NAS_EMM_priv.h"

static void emmProcessMsg(gpointer emm_h, GenericNASMsg_t* msg){
    log_msg(LOG_ERR, 0, "Not Implemented");
}


static void emm_processSecMsg(gpointer emm_h, gpointer buf, gsize len){
	EMMCtx_t *emm = (EMMCtx_t*)emm_h;

	GenericNASMsg_t msg;
	SecurityHeaderType_t s;
    ProtocolDiscriminator_t p;
    gboolean isAuth = FALSE, res;
	nas_getHeader(buf, len, &s, &p);
	if(emm->sci){
		res = nas_authenticateMsg(emm->parser, buf, len, NAS_UpLink, (uint8_t*)&isAuth);
		if(res==2){
			log_msg(LOG_WARNING, 0, "Wrong SQN Count");
			return;
		}else if(res==0){
			g_error("NAS Authentication Error");
		}
	}
	if(!isAuth){
		log_msg(LOG_INFO, 0, "Received Message with wrong MAC");
		return;
	}
	nas_incrementNASCount(emm->parser, NAS_UpLink);

    switch(msg.plain.eMM.messageType){
    /* case AttachComplete: */
	/*     log_msg(LOG_DEBUG, 0, "Received AttachComplete"); */
	/*     processAttachComplete(emm, &msg); */
	/*     break; */
    default:
        log_msg(LOG_WARNING, 0,
                "NAS Message type (%u) not recognized in this context",
                msg.plain.eMM.messageType);
    }

}


void linkEMMRegistered(EMM_State* s){
    s->processMsg = emmProcessMsg;
    /* s->authInfoAvailable = emmAuthInfoAvailable; */
    s->attachAccept = NULL;
    s->processSecMsg = emm_processSecMsg;
    s->sendESM = emm_internalSendESM;
}
