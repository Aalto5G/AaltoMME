/* AaltoMME - Mobility Management Entity for LTE networks
 * Copyright (C) 2013 Vicent Ferrer Guash & Jesus Llorente Santos
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file   EMM_Deregistered.c
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  EMM State
 *
 * The MME has no EMM context or the EMM Context is marked as detached
 */

#include "EMM_Deregistered.h"
#include "logmgr.h"
#include "EMM_FSMConfig.h"
#include "NAS.h"
#include "ECMSession_priv.h"
#include "NAS_EMM_priv.h"
#include "NAS_ESM.h"
#include "MME_S6a.h"
#include "S1Assoc_priv.h"
#include "MME_S1_priv.h"
#include "EMM_Timers.h"

void attachContinuationSwitch(gpointer emm_h, guint8 ksi_msg);
void emm_AuthInfoAvailable(gpointer emm_h);


static void emmProcessMsg(gpointer emm_h,  GenericNASMsg_t* msg){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    guint8 ksi_msg;

    if(msg->header.securityHeaderType.v != PlainNAS){
        emm_log(emm, LOG_ERR, 0, "NAS Integrity or security not implemented");
        return;
    }

    switch(msg->plain.eMM.messageType){

    case AttachRequest:
        ecmSession_getTAI(emm->ecm, emm->sn, &(emm->tac));
        processAttach(emm, msg);
        if(!emm_selectAttachType(emm)){
            return;
        }
        emm_triggerAKAprocedure(emm);
        break;
    case TrackingAreaUpdateRequest:
        emm_log(emm, LOG_INFO, 0,
                "Received TAU Req "
                "Sending TAU Reject. Implicitly Detached");
        emm_sendTAUReject(emm, EMM_ImplicitlyDetached);
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
    guint8 res, msg_ksi;
    guint8 isAuth = 0;
    ECMSession_t *ecm = (ECMSession_t*)emm->ecm;
    struct mme_t *mme = s1_getMME(s1Assoc_getS1(ecm->assoc));
    guti_t msg_guti;

    nas_getHeader(buf, len, &s, &p);

    /* The UE may have a Security Context but not the MME */
    if(emm->sci){
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
    case AttachRequest:
        ecmSession_getTAI(emm->ecm, emm->sn, &(emm->tac));
        processAttach(emm, &msg);
        /* Check last TAI and trigger S6a if different from current*/
        if(!emm_selectAttachType(emm)){
            return;
        }

        if(!isAuth){
            emm_triggerAKAprocedure(emm);
            return;
        }

        emm->nasUlCountForSC = nas_getLastCount(emm->parser, NAS_UpLink);
        emmChangeState(emm, EMM_SpecificProcedureInitiated);
        emm_processFirstESMmsg(emm);
        break;
    case TrackingAreaUpdateRequest:
        emm_log(emm, LOG_INFO, 0,
                "Received TAU Req. "
                "Sending TAU Reject. Implicitly Detached");
        emm_sendTAUReject(emm, EMM_ImplicitlyDetached);
        break;
    case DetachRequest:
        /* HACK ? */
        emm_log(emm, LOG_INFO, 0, "Received DetachRequest");
        processDetachReq(emm, &msg);
        if(emm->ksi != emm->msg_ksi){
            emm_log(emm, LOG_ALERT, 0, "DetachRequest, ksi mismatch");
            return;
        }
        esm_detach(emm->esm, emm_detachAccept, emm);
        break;
    case IdentityResponse:
    case AuthenticationResponse:
    case AuthenticationFailure:
    case SecurityModeReject:
    case DetachAccept:
        emm_log(emm, LOG_INFO, 0,
                "NAS Message type (%x) not valid",
                msg.plain.eMM.messageType);
        break;
    default:
        emm_log(emm, LOG_WARNING, 0,
                "NAS Message type (%x) not recognized",
                msg.plain.eMM.messageType);
    }
}

static void emm_processSrvReq(gpointer emm_h, gpointer buf, gsize len){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    emm_log(emm, LOG_INFO, 0, "Received Service request. "
            "Service Reject sent");
    emm_sendServiceReject(emm, EMM_ImplicitlyDetached);
}

static void emm_processError(gpointer emm_h, GError *err){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;

    emm_log(emm, LOG_INFO, 0, "Received Error");
    if(g_error_matches(err, MME_S6a, S6a_UNKNOWN_EPS_SUBSCRIPTION)){
        if(emm->attachStarted){
            emm_sendAttachReject(emm, EMM_EPSServicesAndNonEPSServicesNotAllowed,
                                 NULL, 0);
        }
    }else{
        emm_log(emm, LOG_ERR, 0, "Error not recognized");
    }
}


static void emm_processTimeout(gpointer emm_h, gpointer buf, gsize len,
                               EMM_TimerCode c){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    emm_log(emm, LOG_WARNING, 0, "Timeout %s, not supported",
            EMM_TimerStr[c]);
}


static void emm_processTimeoutMax(gpointer emm_h, gpointer buf, gsize len,
                                  EMM_TimerCode c){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    emm_log(emm, LOG_WARNING, 0, "Timeout Max %s, not supported",
            EMM_TimerStr[c]);
}


void linkEMMDeregistered(EMM_State* s){
    s->processMsg = emmProcessMsg;
    s->attachAccept = NULL;
    s->processSecMsg = emm_processSecMsg;
    s->processSrvReq = emm_processSrvReq;
    s->sendESM = NULL;
    s->processError = emm_processError;
    s->processTimeout = (EMM_eventTimeout) emm_processTimeout;
    s->processTimeoutMax = (EMM_eventTimeout) emm_processTimeoutMax;
}


void attachContinuationSwitch(gpointer emm_h, guint8 ksi_msg){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;

    if(emm->imsi == 0ULL){ /* !isIMSIavailable(emm) */
        emm_sendIdentityReq(emm);
        emmChangeState(emm, EMM_CommonProcedureInitiated);
        return;
    }

    /* Check Auth, Proof down*/
    if( emm->ksi == 7 && !emm->authQuadrsLen>0){
        emm_log(emm, LOG_DEBUG, 0,"Getting info from HSS");
        s6a_GetAuthInformation(emm->s6a, emm, emm_AuthInfoAvailable, emm_processS6aError, emm);
        return;
    }else if(emm->ksi == 7 && emm->authQuadrsLen>0
             || emm->ksi != ksi_msg){
        /* Remove Sec. Ctx*/
        /* New context available in EMM ctx*/
        /* Send Auth request */
        /* Set T3460 */
        emm_sendAuthRequest(emm);
        emmChangeState(emm, EMM_CommonProcedureInitiated);
        return;
    }
    /* User Authenticated,
     * Proof:
     *
     * A = Sec ctx available
     * B = New Sec ctx available
     * C = UE Sec Ctx available
     * D = UE Sec equal to MME Sec Ctx
     * Eqs:
     * 1) not (not A and not B) and not (not A and B or not D)
     *    = A and D
     * 2) A and D -> C
     * */

    /* Check Security */
    if(!0){

    }
}

void emm_AuthInfoAvailable(gpointer emm_h){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    emm_sendAuthRequest(emm);
}
