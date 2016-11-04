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
#include "MME_S6a.h"
#include "EMM_Timers.h"

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
        emm_stopTimer(emm, T3470);
        processIdentityRsp(emm, msg);
        emm_log(emm, LOG_DEBUG, 0, "Received IdentityResponse");
        s6a_GetAuthInformation(emm->s6a, emm, emm_sendAuthRequest, emm_processS6aError, emm);
        break;
    case AuthenticationResponse:
        emm_stopTimer(emm, T3460);
        processAuthResp(emm, msg, &isAuth);
        break;
    case AuthenticationFailure:
        emm_stopTimer(emm, T3460);
        processAuthFailure(emm, msg);
        break;
    case SecurityModeReject:
        emm_stopTimer(emm, T3460);
        emm_log(emm, LOG_ERR, 0, "Received SecurityModeReject, not implemented");
        break;
    default:
        emm_log(emm, LOG_WARNING, 0,
                "NAS Message type (%x) not recognized in this context",
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
            emm_log(emm, LOG_WARNING, 0, "Wrong SQN Count. Local sqn: %#x, packet sqn: %#x",
                    nas_getLastCount(emm->parser, NAS_UpLink),
                    ((guint8*)buf)[5]);
            return;
        }

        if(!dec_secNAS(emm->parser, &msg, NAS_UpLink, buf, len)){
            g_error("NAS Decyphering Error");
        }
    }else{
        dec_NAS(&msg, buf+6, len-6);
    }

    if(!isAuth && nas_isAuthRequired(msg.plain.eMM.messageType)){
        emm_log(emm, LOG_INFO, 0, "Received Message with wrong MAC");
        return;
    }

    switch(msg.plain.eMM.messageType){
    case IdentityResponse:
        emm_stopTimer(emm, T3470);
        processIdentityRsp(emm, &msg);
        emm_log(emm, LOG_DEBUG, 0, "Received IdentityResponse");
        s6a_GetAuthInformation(emm->s6a, emm, emm_sendAuthRequest, emm_processS6aError, emm);
        break;
    case AuthenticationResponse:
        emm_stopTimer(emm, T3460);
        processAuthResp(emm, &msg, &isAuth);
        break;
    case AuthenticationFailure:
        emm_stopTimer(emm, T3460);
        processAuthFailure(emm, &msg);
        break;
    case SecurityModeReject:
        emm_stopTimer(emm, T3460);
        emm_log(emm, LOG_ERR, 0, "Received SecurityModeReject");
        break;
    case SecurityModeComplete:
        emm_stopTimer(emm, T3460);
        if(! s == IntegrityProtectedAndCipheredWithNewEPSSecurityContext){
            return;
        }
        emm->sci = TRUE;
        emm->nasUlCountForSC = nas_getLastCount(emm->parser, NAS_UpLink);
        emmChangeState(emm, EMM_SpecificProcedureInitiated);
        s6a_UpdateLocation(emm->s6a, emm,
                           (void(*)(gpointer)) emm_selectGateways,
                           (gpointer)emm);
        break;
    default:
        emm_log(emm, LOG_WARNING, 0,
                "NAS Message type (%x) not recognized",
                msg.plain.eMM.messageType);
        if(!emm->s1BearersActive || !emm->attachStarted){
            /* Disconnect ECM */
            ecm_sendUEContextReleaseCommand(emm->ecm, CauseNas, CauseNas_normal_release);
        }
    }
}

static void emm_processSrvReq(gpointer emm_h, gpointer buf, gsize len){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    emm_log(emm, LOG_WARNING, 0, "Received Service request, not supported");
}

static void emm_processError(gpointer emm_h, GError *err){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;

    emm_log(emm, LOG_INFO, 0, "Received Error");
    if(g_error_matches(err, MME_S6a, S6a_UNKNOWN_EPS_SUBSCRIPTION)){
        if(emm->attachStarted){
            emm_sendAttachReject(emm, EMM_EPSServicesAndNonEPSServicesNotAllowed,
                                 NULL, 0);
        }
        emm_stop(emm);
    }else{
        emm_log(emm, LOG_ERR, 0, "Error not recognized, transition to EMM Deregisted");
        emm_stop(emm);
    }
}


static void emm_processTimeout(gpointer emm_h, gpointer buf, gsize len,
                               EMM_TimerCode c){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    guint8 out[1500];
    gsize olen=0;
    SecurityHeaderType_t s;
    NASMessageType_t type = (NASMessageType_t)((guint8*)buf)[1];

    switch (c) {
    case T3460: /* Auth Req or Security Mode Command*/
        emm_log(emm, LOG_INFO, 0, "%s expiration", EMM_TimerStr[c]);
        if(emm->sci || type == SecurityModeCommand){
            s = IntegrityProtectedAndCiphered;
            if(type==SecurityModeCommand){
                s = IntegrityProtectedWithNewEPSSecurityContext;
            }
            newNASMsg_sec(emm->parser, out, &olen,
                          EPSMobilityManagementMessages,
                          s,
                          NAS_DownLink,
                          buf, len);
            ecm_send(emm->ecm, out, olen);
        }else{
            ecm_send(emm->ecm, buf, len);
        }
        break;
    case T3470: /* Identity Req*/
        if(emm->imsi){
            emm_log(emm, LOG_INFO, 0, "%s expiration.",
                    EMM_TimerStr[c]);
        }else if(emm->guti.mtmsi){
            emm_log(emm, LOG_INFO, 0, "%s expiration. UE TMSI %x",
                    EMM_TimerStr[c], emm->guti.mtmsi);
        }else{
            emm_log(emm, LOG_INFO, 0, "%s expiration. unidentified UE",
                    EMM_TimerStr[c]);
        }
        /* Retransmission */
        ecm_send(emm->ecm, buf, len);
        break;
    case TMOBILE_REACHABLE:
        emm_log(emm, LOG_ERR, 0, "%s expiration. Setting Implicit detach timer",
                EMM_TimerStr[c]);
        emm_stopTimer(emm, TMOBILE_REACHABLE);
        emm_setTimer(emm, TIMPLICIT_DETACH, NULL, 0);
        break;
    case TIMPLICIT_DETACH:
        emm_stopTimer(emm, TIMPLICIT_DETACH);
        emm_log(emm, LOG_ERR, 0, "%s expiration. Implicit detach",
                EMM_TimerStr[c]);
        emm_stop(emm);
        /* @TODO: Delete EMM if IMSI == 0*/
        break;
    default:
        emm_log(emm, LOG_ERR, 0, "Timer (%s) not recognized", EMM_TimerStr[c]);
        break;
    }
}


static void emm_processTimeoutMax(gpointer emm_h, gpointer buf, gsize len,
                                  EMM_TimerCode c){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    NASMessageType_t type = (NASMessageType_t)((guint8*)buf)[1];
    switch (c) {
    case T3460: /* Auth Req or Security Mode Command*/
        emm_log(emm, LOG_NOTICE, 0, "%s Max expirations reached"
                "Deregistering.", EMM_TimerStr[c]);
        if(type == AuthenticationRequest){
            emm_stop(emm);
        }else{
            /* Security Mode Command shall be aborted.
             * No other indications in the standard */
            emm_stop(emm);
        }
        break;
    case T3470: /* Identity Req*/
        emm_log(emm, LOG_NOTICE, 0, "%s Max expirations reached. "
                "Deregistering.", EMM_TimerStr[c]);
        emm_stop(emm);
        /*The EMM ctx is registered after the Identity Reply, if the Reply is not Received
         * the context is freed*/
        emm_free(emm);

        ecm_sendUEContextReleaseCommand(emm->ecm, CauseNas, CauseNas_normal_release);
        break;
    case TMOBILE_REACHABLE:
    case TIMPLICIT_DETACH:
        emm_log(emm, LOG_ERR, 0, "Timer (%s) not implemented", EMM_TimerStr[c]);
        break;
    default:
        emm_log(emm, LOG_ERR, 0, "Timer (%s) not recognized", EMM_TimerStr[c]);
        break;
    }
}


void linkEMMCommonProcedureInitiated(EMM_State* s){
    s->processMsg = emmProcessMsg;
    /* s->authInfoAvailable = emmAuthInfoAvailable; */
    s->attachAccept = NULL;
    s->processSecMsg = emm_processSecMsg;
    s->processSrvReq = emm_processSrvReq;
    s->sendESM = NULL;
    s->processError = emm_processError;
    s->processTimeout = (EMM_eventTimeout) emm_processTimeout;
    s->processTimeoutMax = (EMM_eventTimeout) emm_processTimeoutMax;
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

static void processAuthResp(EMMCtx_t * emm,  GenericNASMsg_t* msg, guint8 *isAuth){
    AuthenticationResponse_t * authRsp;
    authRsp = (AuthenticationResponse_t*)&(msg->plain.eMM);
    const AuthQuadruplet *sec;
    guint8 ekey[16] = {0};
    guint8 ikey[16] = {0};

    *isAuth = FALSE;
    sec = emmCtx_getFirstAuthQuadruplet(emm);

    /* Stop T3460*/

    /* Check XRES == RES*/
    if(authRsp->authParam.l != 8){
        emm_log(emm, LOG_ERR, 0, "NAS: Authentication Parameter has a wrong lenght");
        return;
    }

    /* Check Commented for testing*/
    if(memcmp(authRsp->authParam.v, sec->xRES, 8)!=0){
        emm_log(emm, LOG_WARNING, 0, "NAS: Authentication Failed");
        sendAuthReject(emm);
        return;
    }
    *isAuth = TRUE;

    emm_setSecurityQuadruplet(emm);

    nas_setSecurity(emm->parser, NAS_EIA2, NAS_EEA0, emm->kasme);
    emm_sendSecurityModeCommand(emm);
}

static void syncError(gpointer h, GError *err){
    EMMCtx_t *emm = (EMMCtx_t *)h;
    emm_log(emm, LOG_ERR, 0, "Couldn't synchronize NAS SQN,");

    /* @HACK - Further logic required.*/
    emm_stop(emm);
    /* TODO: Remove the hack above.
       If IMSI attach, send Auth Reject
       IF GUTI attach, start Identity Req
     */
}

static void processAuthFailure(EMMCtx_t *emm, GenericNASMsg_t *msg){
    AuthenticationFailure_t *authFail;
    authFail = (AuthenticationFailure_t*)&(msg->plain.eMM);

    if(authFail->eMMCause == EMM_SynchFailure){
        emm_log(emm, LOG_ERR, 0, "Received AuthenticationFailure, Syncing NAS SQN");
        s6a_SynchAuthVector(emm->s6a, emm, authFail->optionals[0].tlv_t4.v,
                            emm_sendAuthRequest, syncError, emm);
    }else if(authFail->eMMCause == EMM_MACFailure){
        emm_log(emm, LOG_ERR, 0, "Received AuthenticationFailure,"
                " MAC Failure");
        /* @HACK - Further logic required.*/
        emmCtx_removeFirstAuthQuadruplet(emm);
        emm_stop(emm);
        /* TODO: Remove the hack above.
         */
    }else{
        emm_log(emm, LOG_ERR, 0, "Received AuthenticationFailure,"
                " Cause not Recognized");
        /* @HACK - Further logic required.*/
        emmCtx_removeFirstAuthQuadruplet(emm);
        emm_stop(emm);
        /* TODO: Remove the hack above.
         */
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

    /* Create a new guti if empty*/
    emmCtx_newGUTI(emm, NULL);
}
