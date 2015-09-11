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
#include "NAS_EMM_priv.h"

static void processAuthResp(EMMCtx_t * emm,  GenericNASMsg_t* msg, gboolean *isAuth);

static void emmProcessMsg(gpointer emm_h, GenericNASMsg_t* msg){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    GenericNASMsg_t *ciph;
    NASPlainMsg_t *plain;
    GenericNASMsg_t msg2;
    gboolean isAuth;

    if(msg->header.securityHeaderType.v != PlainNAS){
        g_error("NAS message with security is not processed here");
    }

    switch(msg->plain.eMM.messageType){

    case IdentityResponse:
        log_msg(LOG_ERR, 0, "Received IdentityResponse, not implemented");
        break;
    case AuthenticationResponse:
	    processAuthResp(emm, msg, &isAuth);
        break;
    case AuthenticationFailure:
        log_msg(LOG_ERR, 0, "Received AuthenticationFailure, not implemented");
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

static void emm_processSecMsg(gpointer emm_h, gpointer buf, gsize len){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;

    GenericNASMsg_t msg;

    SecurityHeaderType_t s;
    ProtocolDiscriminator_t p;
    gboolean isAuth, res;

    nas_getHeader(buf, len, &s, &p);

    if(emm->sci || s == IntegrityProtectedAndCipheredWithNewEPSSecurityContext){
	    res = nas_authenticateMsg(emm->parser, buf, len, NAS_UpLink, (uint8_t*)&isAuth);
	    if(res==2){
		    log_msg(LOG_WARNING, 0, "Wrong SQN Count");
		    return;
	    }else if(res==0){
		    g_error("NAS Authentication Error");
	    }
    }
    
    if(!dec_secNAS(emm->parser, &msg, NAS_UpLink, buf, len)){
	    g_error("NAS Decyphering Error");
    }

    switch(msg.plain.eMM.messageType){

    case IdentityResponse:
        log_msg(LOG_ERR, 0, "Received IdentityResponse, not implemented");
        break;
    case AuthenticationResponse:
	    processAuthResp(emm, &msg, &isAuth);
        break;
    case AuthenticationFailure:
        log_msg(LOG_ERR, 0, "Received AuthenticationFailure, not implemented");
        break;
    case SecurityModeComplete:
	    if(!isAuth){
		    log_msg(LOG_ERR, 0, "Authentication Error on Security Mode Complete");
		    return;
	    }
        nas_incrementNASCount(emm->parser, NAS_UpLink);
        emm->sci = TRUE;
        s6a_UpdateLocation(emm->s6a, emm,
                           (void(*)(gpointer)) emm_processFirstESMmsg,
                           (gpointer)emm);
        emmChangeState(emm, EMM_SpecificProcedureInitiated);
        break;
    case SecurityModeReject:
        log_msg(LOG_ERR, 0, "Received SecurityModeReject, not implemented");
        break;
    default:
        log_msg(LOG_WARNING, 0,
                "NAS Message type (%u) not recognized in this context",
                msg.plain.eMM.messageType);
    }

}


void linkEMMCommonProcedureInitiated(EMM_State* s){
    s->processMsg = emmProcessMsg;
    /* s->authInfoAvailable = emmAuthInfoAvailable; */
    s->attachAccept = NULL;
    s->processSecMsg = emm_processSecMsg;
    s->sendESM = NULL;
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

static void processAuthResp(EMMCtx_t * emm,  GenericNASMsg_t* msg, gboolean *isAuth){
    AuthenticationResponse_t * authRsp;
    authRsp = (AuthenticationResponse_t*)&(msg->plain.eMM);
    AuthQuadruplet *sec;
    guint8 ekey[16] = {0};
    guint8 ikey[16] = {0};

    *isAuth = FALSE;
    sec = (AuthQuadruplet *)g_ptr_array_index(emm->authQuadrs,0);

    /* Stop T3460*/

    /* Check XRES == RES*/
    if(authRsp->authParam.l != 8){
        log_msg(LOG_ERR, 0, "NAS: Authentication Parameter has a wrong lenght");
        return;
    }

    /* Check Commented for testing*/
    if(memcmp(authRsp->authParam.v, sec->xRES, 8)!=0){
	    log_msg(LOG_WARNING, 0, "NAS: Authentication Failed for user: %llu", emm->imsi);
	    sendAuthReject(emm);
	    return;
    }
    *isAuth = TRUE;

    emm_setSecurityQuadruplet(emm);

    nas_setSecurity(emm->parser, NAS_EIA2, NAS_EEA0, emm->kasme);
    emm_sendSecurityModeCommand(emm);
}
