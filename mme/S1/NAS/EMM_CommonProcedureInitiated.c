/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   EMM_CommonProcedureInitiated.c
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  EMM State
 *
 */

#include "EMM_CommonProcedureInitiated.h"
#include "logmgr.h"
#include "EMM_FSMConfig.h"
#include "NAS.h"

void processAuthResp(EMMCtx_t * emm,  GenericNASMsg_t* msg);

static void emmProcessMsg(gpointer emm_h, GenericNASMsg_t* msg){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
	if(msg->header.securityHeaderType.v != PlainNAS){
        log_msg(LOG_ERR, 0, "NAS Integrity or security not implemented");
        return;
    }

	switch(msg->plain.eMM.messageType){

    case IdentityResponse:
	    log_msg(LOG_ERR, 0, "Received IdentityResponse, not implemented");
        break;
	case AuthenticationResponse:
		processAuthResp(emm, msg);
		break;
	case AuthenticationFailure:
		log_msg(LOG_ERR, 0, "Received AuthenticationFailure, not implemented");
		break;
	case SecurityModeComplete:
		log_msg(LOG_ERR, 0, "Received SecurityModeComplete, not implemented");
		break;
	case SecurityModeReject:
		log_msg(LOG_ERR, 0, "Received SecurityModeReject, not implemented");
		break;
    default:
        log_msg(LOG_WARNING, 0,
                "NAS Message type (%u) not recognized in this context",
                msg->plain.eMM.messageType);
    }
}


void linkEMMCommonProcedureInitiated(EMM_State* s){
    s->processMsg = emmProcessMsg;
    /* s->authInfoAvailable = emmAuthInfoAvailable; */
}

void sendAuthReject(EMMCtx_t * emm){
	guint8 *pointer;
    guint8 buffer[150];
    uint8_t c;

    pointer = buffer;
    newNASMsg_EMM(&pointer, EPSMobilityManagementMessages, PlainNAS);
    encaps_EMM(&pointer, AuthenticationReject);
  
	ecm_send(emm->ecm, buffer, pointer-buffer);
	emmChangeState(emm, EMM_Deregistered);
}

void test(EMMCtx_t emm){
	log_msg(LOG_ERR, 0, "Upsated S6a location");
}

void processAuthResp(EMMCtx_t * emm,  GenericNASMsg_t* msg){
	AuthenticationResponse_t * authRsp;
	authRsp = (AuthenticationResponse_t*)&(msg->plain.eMM);
	AuthQuadruplet *sec;

	sec = (AuthQuadruplet *)g_ptr_array_index(emm->authQuadrs,0);

	/* Stop T3460*/

	/* Check XRES == RES*/
	if(authRsp->authParam.l != 8){
		log_msg(LOG_ERR, 0, "NAS: Authentication Parameter has a wrong lenght");
		return;
	}

	/* Check Commented for testing*/
	/* if(memcmp(authRsp->authParam.v, sec->xRES, 8)!=0){ */
	/* 	log_msg(LOG_WARNING, 0, "NAS: Authentication Failed for user: %llu", emm->imsi); */
	/* 	sendAuthReject(emm); */
	/* 	return; */
	/* } */

	emm_setSecurityQuadruplet(emm);

	/*Temporary*/
	/* @TODO Comment the following line and uncomment the next ones
	 *  to activate the Security Mode Procedure
	 */
	s6a_UpdateLocation(emm->s6a, emm,
	                   (void(*)(gpointer)) test,
	                   (gpointer)emm);
}
