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
#include "MME_S6a.h"
#include "S1Assoc_priv.h"
#include "MME_S1_priv.h"


void processAttach(gpointer emm_h,  GenericNASMsg_t* msg, guint8 *ksi_msg);
void attachContinuationSwitch(gpointer emm_h, guint8 ksi_msg);
void sendIdentityReq(gpointer emm_h);
void emm_AuthInfoAvailable(gpointer emm_h);

static void emmProcessMsg(gpointer emm_h,  GenericNASMsg_t* msg){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    guint8 ksi_msg;

    if(msg->header.securityHeaderType.v != PlainNAS){
        log_msg(LOG_ERR, 0, "NAS Integrity or security not implemented");
        return;
    }

    switch(msg->plain.eMM.messageType){

    case AttachRequest:
        processAttach(emm, msg, &ksi_msg);
        attachContinuationSwitch(emm, ksi_msg);
        break;
    default:
        log_msg(LOG_WARNING, 0,
                "NAS Message type (%u) not recognized in this context",
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
        processAttach(emm, &msg, &msg_ksi);

        if(!isAuth){
            emm_triggerAKAprocedure(emm);
            return;
        }

        emm->nasUlCountForSC = nas_getLastCount(emm->parser, NAS_UpLink);
        emmChangeState(emm, EMM_SpecificProcedureInitiated);
        emm_processFirstESMmsg(emm);
        break;
    case IdentityResponse:
    case AuthenticationResponse:
    case AuthenticationFailure:
    case SecurityModeReject:
    case DetachAccept:
        log_msg(LOG_INFO, 0,
                "NAS Message type (%u) not valid in this state",
                msg.plain.eMM.messageType);
        break;
    /* case TrackingAreaUpdateRequest: */
    /*     emm_processTAUReq(emm, &msg, &msg_ksi, &msg_guti); */
    /*     if(!isAuth){ */
    /*         emm_triggerAKAprocedure(emm); */
    /*         return; */
    /*     } */
    /*     emm->nasUlCountForSC = nas_getLastCount(emm->parser, NAS_UpLink); */
    /*     emmChangeState(emm, EMM_SpecificProcedureInitiated); */
    /*     emm_sendTAUAccept(emm); */
    /*     break; */
    case TrackingAreaUpdateRequest:
    case DetachRequest:
        log_msg(LOG_WARNING, 0,
                "NAS Message type (%u) not supported in this context",
                msg.plain.eMM.messageType);
        break;
    default:
        log_msg(LOG_WARNING, 0,
                "NAS Message type (%u) not recognized in this context",
                msg.plain.eMM.messageType);
    }
}


void linkEMMDeregistered(EMM_State* s){
    s->processMsg = emmProcessMsg;
    s->attachAccept = NULL;
    s->processSecMsg = emm_processSecMsg;
    s->sendESM = NULL;
}



void processAttach(gpointer emm_h,  GenericNASMsg_t* msg, guint8 *ksi_msg){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    AttachRequest_t *attachMsg;
    guint i;
    uint64_t mobid=0ULL;
    GByteArray *esmRaw;
    guint16 cap;

    attachMsg = (AttachRequest_t*)&(msg->plain.eMM);
    emm->attachStarted = TRUE;
    emm->attachType = attachMsg->ePSAttachType.v;
    *ksi_msg = attachMsg->nASKeySetId.v & 0x07;

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
        log_msg(LOG_DEBUG, 0,"Attach Received from imsi : %llu", mobid);
        emm->imsi = mobid;
    }else if(((ePSMobileId_header_t*)attachMsg->ePSMobileId.v)->type == 6 ){    /*GUTI*/
        memcpy(&(emm->msg_guti), (guti_t *)(attachMsg->ePSMobileId.v+1), 10);
    }

    memcpy(emm->ueCapabilities,
           attachMsg->uENetworkCapability.v,
           attachMsg->uENetworkCapability.l);
    emm->ueCapabilitiesLen = attachMsg->uENetworkCapability.l;
}

void attachContinuationSwitch(gpointer emm_h, guint8 ksi_msg){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;

    if(emm->imsi == 0ULL){ /* !isIMSIavailable(emm) */
        sendIdentityReq(emm);
        emmChangeState(emm, EMM_CommonProcedureInitiated);
        return;
    }

    /* Check Auth, Proof down*/
    if( emm->ksi == 7 && !emm->authQuadrsLen>0){
        log_msg(LOG_DEBUG, 0,"Getting info from HSS");
        s6a_GetAuthInformation(emm->s6a, emm, emm_AuthInfoAvailable, emm);
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

void sendIdentityReq(gpointer emm_h){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    guint8 *pointer;
    guint8 buffer[150];

    log_msg(LOG_DEBUG, 0, "Building Identity Request");

    pointer = buffer;
    newNASMsg_EMM(&pointer, EPSMobilityManagementMessages, PlainNAS);

    encaps_EMM(&pointer, IdentityRequest);

    /* Selected NAS security algorithms */
    nasIe_v_t1_l(&pointer, 1); /*Get Imsi*/
    pointer++; /*Spare half octet*/

    ecm_send(emm->ecm, buffer, pointer-buffer);
}
