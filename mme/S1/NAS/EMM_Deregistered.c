/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
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


void processAttach(gpointer emm_h,  GenericNASMsg_t* msg);
void attachContinuationSwitch(gpointer emm_h, guint8 ksi_msg);
void emm_AuthInfoAvailable(gpointer emm_h);
static int emm_selectAttachType(EMMCtx_t *emm);


static void emmProcessMsg(gpointer emm_h,  GenericNASMsg_t* msg){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    guint8 ksi_msg;

    if(msg->header.securityHeaderType.v != PlainNAS){
        log_msg(LOG_ERR, 0, "NAS Integrity or security not implemented");
        return;
    }

    switch(msg->plain.eMM.messageType){

    case AttachRequest:
        processAttach(emm, msg);
        if(!emm_selectAttachType(emm)){
            return;
        }
        emm_triggerAKAprocedure(emm);
        break;
    default:
        log_msg(LOG_WARNING, 0,
                "NAS Message type (%x) not recognized in this context",
                msg->plain.eMM.messageType);
    }
}


static void emm_processSecMsg(gpointer emm_h, gpointer buf, gsize len){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    EMMCtx_t *old_emm = NULL;

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
    case AttachRequest:
        processAttach(emm, &msg);
        /* Check last TAI and trigger S6a if different from current*/
        if(!emm_selectAttachType(emm)){
            return;
        }

        if(!isAuth || TRUE){
            emm_triggerAKAprocedure(emm);
            return;
        }

        emm->nasUlCountForSC = nas_getLastCount(emm->parser, NAS_UpLink);
        emmChangeState(emm, EMM_SpecificProcedureInitiated);
        emm_processFirstESMmsg(emm);
        break;
    case TrackingAreaUpdateRequest:
        log_msg(LOG_INFO, 0,
                "Received TAU Req on EMM Deregistered. "
                "Sending TAU Reject. Implicitly Detached");
        emm_sendTAUReject(emm, EMM_ImplicitlyDetached);
        break;
    case DetachRequest:
        /* HACK ? */
        log_msg(LOG_INFO, 0, "Received DetachRequest");
        processDetachReq(emm, &msg);
        if(emm->ksi != emm->msg_ksi){
            log_msg(LOG_ALERT, 0, "DetachRequest, ksi mismatch");
            return;
        }
        esm_detach(emm->esm, emm_detachAccept, emm);
        break;
    case IdentityResponse:
    case AuthenticationResponse:
    case AuthenticationFailure:
    case SecurityModeReject:
    case DetachAccept:
        log_msg(LOG_INFO, 0,
                "NAS Message type (%u) not valid in EMM Deregistered",
                msg.plain.eMM.messageType);
        break;
    default:
        log_msg(LOG_WARNING, 0,
                "NAS Message type (%u) not recognized in EMM Deregistered",
                msg.plain.eMM.messageType);
    }
}

static void emm_processSrvReq(gpointer emm_h, gpointer buf, gsize len){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    log_msg(LOG_INFO, 0, "Received Service request while in EMM Deregistered, "
            "Service Reject sent");
    emm_sendServiceReject(emm, EMM_ImplicitlyDetached);
}

static void emm_processError(gpointer emm_h, GError *err){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;

    log_msg(LOG_INFO, 0, "Received Error");
    if(g_error_matches(err, MME_S6a, S6a_UNKNOWN_EPS_SUBSCRIPTION)){
        if(emm->attachStarted){
            emm_sendAttachReject(emm, EMM_EPSServicesAndNonEPSServicesNotAllowed,
                                 NULL, 0);
        }
    }else{
        log_msg(LOG_ERR, 0, "Error not recognized, transition to EMM Deregisted");
    }
}

void linkEMMDeregistered(EMM_State* s){
    s->processMsg = emmProcessMsg;
    s->attachAccept = NULL;
    s->processSecMsg = emm_processSecMsg;
    s->processSrvReq = emm_processSrvReq;
    s->sendESM = NULL;
    s->processError = emm_processError;
}


static int emm_selectAttachType(EMMCtx_t * emm){
    switch(emm->msg_attachType){
    case 2:
        /*Combined EPS/IMSI attach*/
        if(emm->msg_additionalUpdateType && emm->msg_smsOnly){
            emm->attachResult = 2;
            return 1;
        }else{
            /* /\*Reject*\/ */
            /* log_msg(LOG_ERR, 0, */
            /*         "Combined EPS/IMSI attach not supported"); */
            /* return 1; */
            /* HACK */
            emm->attachResult = 2;
            return 1;
        }
    case 4:
        /*EPS emergency attach */
        /*Reject*/
        log_msg(LOG_ERR, 0,
                "Emergency attach not supported");
        return 0;
    case 7:
        /*Reserved*/
        /*Reject*/
        log_msg(LOG_ERR, 0,
                "Reserved attach type received");
        return 0;
    case 1:
    default:
        /*EPS attach*/
        emm->attachResult = 1;
        return 1;
    }
}

void processAttach(gpointer emm_h,  GenericNASMsg_t* msg){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    AttachRequest_t *attachMsg;
    guint i;
    uint64_t mobid=0ULL;
    GByteArray *esmRaw;
    guint16 cap;
    union nAS_ie_member const *optIE=NULL;

    attachMsg = (AttachRequest_t*)&(msg->plain.eMM);
    emm->attachStarted = TRUE;
    emm->msg_attachType = attachMsg->ePSAttachType.v;
    emm->msg_ksi = attachMsg->nASKeySetId.v & 0x07;

    esmRaw = g_byte_array_new();
    g_byte_array_append(esmRaw,
                        (guint8*)attachMsg->eSM_MessageContainer.v,
                        attachMsg->eSM_MessageContainer.l);

    g_ptr_array_add(emm->pendingESMmsg, esmRaw);

    if(((ePSMobileId_header_t*)attachMsg->ePSMobileId.v)->type == 1 ){  /* IMSI*/
        for(i=0; i<attachMsg->ePSMobileId.l-1; i++){
            mobid = mobid*10 + ((attachMsg->ePSMobileId.v[i])>>4);
            mobid = mobid*10 + ((attachMsg->ePSMobileId.v[i+1])&0x0F);
        }
        if(((ePSMobileId_header_t*)attachMsg->ePSMobileId.v)->parity == 1){
            mobid = mobid*10 + ((attachMsg->ePSMobileId.v[i])>>4);
        }
        log_msg(LOG_DEBUG, 0,"Attach Received from IMSI : %llu", mobid);
        emm->imsi = mobid;
    }else if(((ePSMobileId_header_t*)attachMsg->ePSMobileId.v)->type == 6 ){    /*GUTI*/
        memcpy(&(emm->msg_guti), (guti_t *)(attachMsg->ePSMobileId.v+1), 10);
    }

    memcpy(emm->ueCapabilities,
           attachMsg->uENetworkCapability.v,
           attachMsg->uENetworkCapability.l);
    emm->ueCapabilitiesLen = attachMsg->uENetworkCapability.l;

    /*Optionals*/
    /* Last visited registered TAI: 0x52*/
    nas_NASOpt_lookup(attachMsg->optionals, 17, 0x52, &optIE);
    if(optIE){
        /* optIE->tv_t3_l.v; */
    }
    /*MS network capability: 0x58*/
    nas_NASOpt_lookup(attachMsg->optionals, 17, 0x31, &optIE);
    if(optIE){
        memcpy(emm->msNetCap, optIE->tlv_t4.v, optIE->tlv_t4.l);
        emm->msNetCapLen = optIE->tlv_t4.l;
    }
    /* Additional Update type: 0xF*/
    nas_NASOpt_lookup(attachMsg->optionals, 17, 0xF, &optIE);
    if(optIE){
        emm->msg_additionalUpdateType = TRUE;
        emm->msg_smsOnly = (gboolean)optIE->v_t1_l.v;
    }
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
        log_msg(LOG_DEBUG, 0,"Getting info from HSS");
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
