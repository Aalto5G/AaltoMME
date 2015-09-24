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

static void processIdentityRsp(EMMCtx_t *emm, GenericNASMsg_t *msg);
static void processAuthResp(EMMCtx_t * emm,  GenericNASMsg_t* msg, guint8 *isAuth);
static void processAuthFailure(EMMCtx_t *emm, GenericNASMsg_t *msg);

static void emmProcessMsg(gpointer emm_h, GenericNASMsg_t* msg){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    GenericNASMsg_t *ciph;
    NASPlainMsg_t *plain;
    GenericNASMsg_t msg2;
    guint8 isAuth;

    if(msg->header.securityHeaderType.v != PlainNAS){
        g_error("NAS message with security is not processed here");
    }

    switch(msg->plain.eMM.messageType){

    case IdentityResponse:
        processIdentityRsp(emm, msg);
        log_msg(LOG_DEBUG, 0, "Received IdentityResponse from: %llu", emm->imsi);
        s6a_GetAuthInformation(emm->s6a, emm, emm_sendAuthRequest, emm);
        break;
    case AuthenticationResponse:
        processAuthResp(emm, msg, &isAuth);
        break;
    case AuthenticationFailure:
        processAuthFailure(emm, msg);
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
    guint8 isAuth = 0, res = 0;

    nas_getHeader(buf, len, &s, &p);

    /* The UE may have a Security Context but not the MME */
    if(emm->sci || s == IntegrityProtectedAndCipheredWithNewEPSSecurityContext){
        res = nas_authenticateMsg(emm->parser, buf, len, NAS_UpLink, (uint8_t*)&isAuth);
        if(res==0){
            /* EH Send Indication Error*/
            g_error("Received malformed NAS packet");
        }else if(res==2){
            /* EH trigger AKA procedure */
            log_msg(LOG_WARNING, 0, "Wrong SQN Count");
            return;
        }

        if(!dec_secNAS(emm->parser, &msg, NAS_UpLink, buf, len)){
            g_error("NAS Decyphering Error");
        }
    }else{
        dec_NAS(&msg, buf+6, len-6);
    }

    if(!isAuth && nas_isAuthRequired(msg.plain.eMM.messageType)){
        log_msg(LOG_INFO, 0, "Received Message with wrong MAC");
        return;
    }

    switch(msg.plain.eMM.messageType){
    case IdentityResponse:
        processIdentityRsp(emm, &msg);
        log_msg(LOG_DEBUG, 0, "Received IdentityResponse from: %llu", emm->imsi);
        s6a_GetAuthInformation(emm->s6a, emm, emm_sendAuthRequest, emm);
        break;
    case AuthenticationResponse:
        processAuthResp(emm, &msg, &isAuth);
        break;
    case AuthenticationFailure:
        processAuthFailure(emm, &msg);
        break;
    case SecurityModeReject:
        log_msg(LOG_ERR, 0, "Received SecurityModeReject, not implemented EMM CPI");
        break;
    case SecurityModeComplete:
        if(! s == IntegrityProtectedAndCipheredWithNewEPSSecurityContext){
            return;
        }
        emm->sci = TRUE;
        emm->nasUlCountForSC = nas_getLastCount(emm->parser, NAS_UpLink);
        emmChangeState(emm, EMM_SpecificProcedureInitiated);
        s6a_UpdateLocation(emm->s6a, emm,
                           (void(*)(gpointer)) emm_processFirstESMmsg,
                           (gpointer)emm);
        break;
    default:
        log_msg(LOG_WARNING, 0,
                "NAS Message type (%u) not recognized in EMM CPI",
                msg.plain.eMM.messageType);
    }
}

static void emm_processSrvReq(gpointer emm_h, gpointer buf, gsize len){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    log_msg(LOG_WARNING, 0, "Received Service request, not supported in EMM CPI");
}

void linkEMMCommonProcedureInitiated(EMM_State* s){
    s->processMsg = emmProcessMsg;
    /* s->authInfoAvailable = emmAuthInfoAvailable; */
    s->attachAccept = NULL;
    s->processSecMsg = emm_processSecMsg;
    s->processSrvReq = emm_processSrvReq;
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

void test(EMMCtx_t *emm){
    log_msg(LOG_ERR, 0, "Upsated NAS SQN");
}

static void processAuthResp(EMMCtx_t * emm,  GenericNASMsg_t* msg, guint8 *isAuth){
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

static void processAuthFailure(EMMCtx_t *emm, GenericNASMsg_t *msg){
    AuthenticationFailure_t *authFail;
    authFail = (AuthenticationFailure_t*)&(msg->plain.eMM);

    if(authFail->eMMCause == EMM_SynchFailure){
        log_msg(LOG_ERR, 0, "Received AuthenticationFailure, Syncing NAS SQN");

        s6a_SynchAuthVector(emm->s6a, emm, authFail->optionals[0].tlv_t4.v,
                            emm_sendAuthRequest,
                            emm);
    }else{
        log_msg(LOG_ERR, 0, "Received AuthenticationFailure,"
                " Cause not Recognized");
    }
}

static void processIdentityRsp(EMMCtx_t *emm, GenericNASMsg_t *msg){
    uint64_t mobid=0ULL;
    uint32_t i;

    IdentityResponse_t *idRsp = (IdentityResponse_t*)&(msg->plain.eMM);

    if(((ePSMobileId_header_t*)idRsp->mobileId.v)->type == 1 ){  /* IMSI*/
        for(i=0; i<idRsp->mobileId.l-1; i++){
            mobid = mobid*10 + ((idRsp->mobileId.v[i])>>4);
            mobid = mobid*10 + ((idRsp->mobileId.v[i+1])&0x0F);
        }
        if(((ePSMobileId_header_t*)idRsp->mobileId.v)->parity == 1){
            mobid = mobid*10 + ((idRsp->mobileId.v[i])>>4);
        }
        emm->imsi = mobid;
    }
}
