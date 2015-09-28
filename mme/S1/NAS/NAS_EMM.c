/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   NAS_EMM.c
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  NAS EMM implementation
 *
 * This module implements the NAS EMM interface state machine on the MME EndPoint.
 */

#include <glib.h>
#include <arpa/inet.h>
#include "NAS_EMM_priv.h"
#include "NAS.h"
#include "logmgr.h"
#include "EMMCtx.h"
#include "ECMSession_priv.h"
#include "NAS_ESM.h"
#include "EMM_State.h"

#include "hmac_sha2.h"

gpointer emm_init(gpointer ecm){
    EMMCtx_t *self = emmCtx_init();
    emmChangeState(self, EMM_Deregistered);
    self->ecm = ecm;
    self->s6a = ecmSession_getS6a(ecm);
    self->esm = esm_init(self);
    self->parser = nas_newHandler();
    return self;
}

void emm_free(gpointer emm_h){
    EMMCtx_t *self = (EMMCtx_t*)emm_h;
    nas_freeHandler(self->parser);
    esm_free(self->esm);
    emmCtx_free(self);
}

void emm_registerECM(EMMCtx emm_h, gpointer ecm){
    EMMCtx_t *self = (EMMCtx_t*)emm_h;
    self->ecm = ecm;
}

void emm_deregister(EMMCtx emm_h){
    EMMCtx_t *self = (EMMCtx_t*)emm_h;
    self->ecm = NULL;
}

gpointer emm_getS11(gpointer emm_h){
    EMMCtx_t *self = (EMMCtx_t*)emm_h;
    return ecmSession_getS11(self->ecm);
}


void emm_processMsg(gpointer emm_h, gpointer buffer, gsize len){
    EMMCtx_t *self = (EMMCtx_t*)emm_h;
    GenericNASMsg_t msg;
    SecurityHeaderType_t s;
    ProtocolDiscriminator_t p;

    if (!nas_getHeader(buffer, len, &s, &p))
        g_error("Empty NAS message buffer");

    if(s == PlainNAS){
        dec_NAS(&msg, buffer, len);
        self->state->processMsg(self, &msg);
    }else if(s > PlainNAS && s <= IntegrityProtectedAndCipheredWithNewEPSSecurityContext){
        self->state->processSecMsg(self, buffer, len);
    }else if(s == SecurityHeaderForServiceRequestMessage){
        self->state->processSrvReq(self, buffer, len);
    }else{
        log_msg(LOG_INFO, 0, "Invalid Security Header received: Ignoring");
    }
}

void emm_getGUTIfromMsg(gpointer buffer, gsize len, guti_t* guti){
    GenericNASMsg_t msg;
    SecurityHeaderType_t s;
    ProtocolDiscriminator_t p;
    AttachRequest_t *attachMsg;
    TrackingAreaUpdateRequest_t *tau_msg;

    if (!nas_getHeader(buffer, len, &s, &p))
        g_error("Empty NAS message buffer");

    if(s == PlainNAS){
        dec_NAS(&msg, buffer, len);
    }else if(s == IntegrityProtected){
        dec_NAS(&msg, buffer+6, len-6);
    }else{
        log_msg(LOG_DEBUG, 0, "Cannot get GUTI from cyphered message");
        return;
    }
    log_msg(LOG_DEBUG, 0, "Get GUTI from message type %u",
            msg.plain.eMM.messageType);

    switch(msg.plain.eMM.messageType){
    case AttachRequest:
        attachMsg = (AttachRequest_t*)&(msg.plain.eMM);
        if(((ePSMobileId_header_t*)attachMsg->ePSMobileId.v)->type == 6 ){
            memcpy(guti, (guti_t *)(attachMsg->ePSMobileId.v+1), 10);
        }
        break;
    case TrackingAreaUpdateRequest:
        tau_msg = (TrackingAreaUpdateRequest_t*)&(msg.plain.eMM);
        if(((ePSMobileId_header_t*)tau_msg->oldGUTI.v)->type == 6 ){
            memcpy(guti, (guti_t *)(tau_msg->oldGUTI.v+1), 10);
        }
        break;
    default:
        log_msg(LOG_DEBUG, 0, "Not implemented for message type %u",
                msg.plain.eMM.messageType);
        break;
    }
    log_msg(LOG_DEBUG, 0, "M-TMSI %x", guti->mtmsi);
}

static guint8 emm_nextKSI(guint8 k){
    if(k < 6){
        return k+1;
    }else{
        return 1;
    }
}

static guint8 emm_generateNewKSI(guint8 k1, guint k2){
    k1 = emm_nextKSI(k1);
    if(k1 == k2){
        k1 = emm_nextKSI(k1);
    }
    return k1;
}

void emm_sendAuthRequest(EMMCtx emm_h){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    guint8 *pointer;
    guint8 buffer[150];
    AuthQuadruplet *sec;
    guint8 old_ksi;

    memset(buffer, 0, 150);

    log_msg(LOG_DEBUG, 0, "Initiating UE authentication");

    /* Build Auth Request */
    sec = (AuthQuadruplet *)g_ptr_array_index(emm->authQuadrs,0);

    pointer = buffer;
    newNASMsg_EMM(&pointer, EPSMobilityManagementMessages, PlainNAS);

    encaps_EMM(&pointer, AuthenticationRequest);

    /* Generate new eKSI */
    emm->ksi = emm_generateNewKSI(emm->ksi, emm->msg_ksi);

    nasIe_v_t1_l(&pointer, emm->ksi&0x0F);
    pointer++; /*Spare half octet*/

    /* RAND */
    nasIe_v_t3(&pointer, sec->rAND, 16); /* 256 bits */

    /* AUTN */
    nasIe_lv_t4(&pointer, sec->aUTN, 16); /* 256 bits */

    ecm_send(emm->ecm, buffer, pointer-buffer);
    emmChangeState(emm, EMM_CommonProcedureInitiated);
}

void emm_setSecurityQuadruplet(EMMCtx emm_h){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;

    AuthQuadruplet *sec;
    sec = (AuthQuadruplet *)g_ptr_array_index(emm->authQuadrs,0);

    emm->old_ncc = emm->ncc;
    memcpy(emm->old_kasme, emm->kasme, 32);
    memcpy(emm->old_nh, emm->nh, 32);

    memcpy(emm->kasme, sec->kASME, 32);

    emm->authQuadrsLen--;
    g_ptr_array_remove_index(emm->authQuadrs, 0);
}

void emm_sendSecurityModeCommand(EMMCtx emm_h){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    guint8 *pointer, algorithms;
    guint8 capabilities[5];
    guint32 len;
    guint8 count, out[156], plain[150], req;
    memset(out, 0, 156);
    memset(plain, 0, 150);

     /* Build Security Mode Command*/
    pointer = plain;
    newNASMsg_EMM(&pointer, EPSMobilityManagementMessages, PlainNAS);

    encaps_EMM(&pointer, SecurityModeCommand);

    /* Selected NAS security algorithms */
    algorithms = 0 | NAS_EEA0<<4 | NAS_EIA2;
    nasIe_v_t3(&pointer, &algorithms, 1);

    /*NAS key set identifier*/
    nasIe_v_t1_l(&pointer, (emm->ksi)&0x0F);
    pointer++; /*Spare half octet*/

    /* Replayed UE security capabilities */
    memset(capabilities, 0, 5);
    memcpy(capabilities, emm->ueCapabilities, 4);
    capabilities[4]=0x70;
    nasIe_lv_t4(&pointer, capabilities, 5);

    /* IMEISV request */
    req = 0xc0&0xf0  /* Type*/
        | 0          /* Spare */
        | 0x01&0x07; /* Request*/
    nasIe_v_t3(&pointer, &req, 1);

    newNASMsg_sec(emm->parser, out, &len,
                  EPSMobilityManagementMessages,
                  IntegrityProtectedWithNewEPSSecurityContext,
                  NAS_DownLink,
                  plain, pointer-plain);

    ecm_send(emm->ecm, out, len);
    /* nas_incrementNASCount(emm->parser, NAS_DownLink); */
    emmChangeState(emm, EMM_CommonProcedureInitiated);

    /* Set timer T3460*/
}

void emm_processFirstESMmsg(EMMCtx emm_h){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    GByteArray *esmRaw;
    GenericNASMsg_t msg;

    esmRaw = g_ptr_array_index(emm->pendingESMmsg, 0);

    dec_NAS(&msg, esmRaw->data, esmRaw->len);
    esm_processMsg(emm->esm, &(msg.plain.eSM));

    g_ptr_array_remove_index(emm->pendingESMmsg, 0);
}

void emm_attachAccept(EMMCtx emm_h, gpointer esm_msg, gsize len, GList *bearers){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;

    emm->state->attachAccept(emm, esm_msg, len, bearers);
}


/**
 * @brief generate_KeNB - KDF function to derive the K_eNB
 * @param [in]  kasme       derived key - 256 bits
 * @param [in]  ulNASCount  Uplink NAS COUNT
 * @param [out] keNB        eNB result key - 256 bits
 */
static void generate_KeNB(const uint8_t *kasme, const uint32_t ulNASCount, uint8_t *keNB){

    /*
    FC = 0x11,
    P0 = Uplink NAS COUNT,
    L0 = length of uplink NAS COUNT (i.e. 0x00 0x04)
     */
    uint32_t count;
    uint8_t s[7];
    s[0]=0x11;
    count = htonl(ulNASCount);
    memcpy(s+1, &count, 4);
    s[5]=0x00;
    s[6]=0x04;

    hmac_sha256(kasme, 32, s, 7, keNB, 32);
}

void emm_getKeNB(const EMMCtx emm, uint8_t *keNB){
    EMMCtx_t *self = (EMMCtx_t*)emm;
    generate_KeNB(self->kasme, self->nasUlCountForSC, keNB);
}

void emm_getNH(const EMMCtx emm, guint8 *nh, guint8 *ncc){
    EMMCtx_t *self = (EMMCtx_t*)emm;
    /*
    -    FC = 0x12
    -    P0 = SYNC-input
    -    L0 = length of SYNC-input (i.e. 0x00 0x20)
    The SYNC-input parameter shall be the newly derived K eNB  for the initial
    NH derivation, and the previous NH for all subsequent derivations. This
    results in a NH chain, where the next NH is always fresh and derived from
    the previous NH.
    */

    uint8_t s[35], zero[32], keNB[32];
    memset(zero, 0, 32);

    s[0]=0x12;
    if(memcmp(self->nh, zero, 32)==0){
        /*First hop*/
        emm_getKeNB(self, keNB);
        memcpy(s+1, keNB, 32);
    }else{
        memcpy(s+1, self->nh, 32);
        self->ncc++;
    }
    s[33]=0;
    s[34]=0x20;

    hmac_sha256(self->kasme, 32, s, 35, self->nh, 32);
    *ncc = self->ncc;
}


void emm_getUESecurityCapabilities(const EMMCtx emm, UESecurityCapabilities_t *cap){
    EMMCtx_t *self = (EMMCtx_t*)emm;
    cap->encryptionAlgorithms.v = self->ueCapabilities[0] <<8 | 0x0;
    cap->integrityProtectionAlgorithms.v = self->ueCapabilities[1]<<8 | 0x0;
}

void emm_getUEAMBR(const EMMCtx emm, UEAggregateMaximumBitrate_t *ambr){
    EMMCtx_t *self = (EMMCtx_t*)emm;
    guint64 ul, dl;
    subs_getUEAMBR(self->subs, &ul, &dl);
    ambr->uEaggregateMaximumBitRateDL.rate = dl;
    ambr->uEaggregateMaximumBitRateUL.rate = ul;
}

void emm_sendESM(const EMMCtx emm, const gpointer msg, const gsize len, GError **e){
    EMMCtx_t *self = (EMMCtx_t*)emm;
    self->state->sendESM(self, msg, len, e);
}

void emm_internalSendESM(const EMMCtx emm, const gpointer msg, const gsize len, GError **e){
    EMMCtx_t *self = (EMMCtx_t*)emm;
    guint8 out[len+6];
    gsize oLen;

    if(!self->sci){
        ecm_send(self->ecm, msg, len);
        return;
    }

    newNASMsg_sec(self->parser, out, (uint32_t*)&oLen,
                  EPSMobilityManagementMessages,
                  IntegrityProtectedAndCiphered,
                  NAS_DownLink,
                  msg, len);

    ecm_send(self->ecm, out, oLen);
    /* nas_incrementNASCount(self->parser, NAS_DownLink); */
}

void emm_modifyE_RABList(EMMCtx emm,  E_RABsToBeModified_t* l,
                         void (*cb)(gpointer), gpointer args){
    EMMCtx_t *self = (EMMCtx_t*)emm;
    esm_modifyE_RABList(self->esm, l, cb, args);
}

void emm_UEContextReleaseReq(EMMCtx emm, void (*cb)(gpointer), gpointer args){
    EMMCtx_t *self = (EMMCtx_t*)emm;
    self->s1BearersActive = FALSE;
    esm_UEContextReleaseReq(self->esm, cb, args);
}


guint32 *emm_getM_TMSI_p(EMMCtx emm){
    return emmCtx_getM_TMSI_p(emm);
}

void emm_processTAUReq(EMMCtx emm_h, GenericNASMsg_t *msg){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    uint8_t mobId[20];
    uint32_t esmSize, i;
    uint64_t mobid=0ULL;
    union nAS_ie_member const *optIE=NULL;

    TrackingAreaUpdateRequest_t *tau_msg = (TrackingAreaUpdateRequest_t*)&(msg->plain.eMM);

    /* EPS update type */
    emm->msg_updateType = (tau_msg->ePSUpdateType.v&0x07);
    emm->msg_activeFlag = (gboolean)(tau_msg->ePSUpdateType.v&0x08)>>3;

    /* nASKeySetId */
    emm->msg_ksi = tau_msg->nASKeySetId.v & 0x07;

    /* Old GUTI - EPSMobileId*/
    if(((ePSMobileId_header_t*)tau_msg->oldGUTI.v)->type == 6 ){    /*GUTI*/
        memcpy(&(emm->msg_guti), (guti_t *)(tau_msg->oldGUTI.v+1), 10);
    }

    /*Optionals*/
    /*UE network capability: 0x58*/
    nas_NASOpt_lookup(tau_msg->optionals, 22, 0x58, &optIE);
    if(optIE){
        memcpy(emm->ueCapabilities, optIE->tlv_t4.v, optIE->tlv_t4.l);
        emm->ueCapabilitiesLen = optIE->tlv_t4.l;
    }
    /*MS network capability: 0x58*/
    nas_NASOpt_lookup(tau_msg->optionals, 22, 0x31, &optIE);
    if(optIE){
        memcpy(emm->msNetCap, optIE->tlv_t4.v, optIE->tlv_t4.l);
        emm->msNetCapLen = optIE->tlv_t4.l;
    }
    /* Additional Update type: 0xF*/
    nas_NASOpt_lookup(tau_msg->optionals, 22, 0xF, &optIE);
    if(optIE){
        emm->msg_additionalUpdateType = TRUE;
        emm->msg_smsOnly = (gboolean)optIE->v_t1_l.v;
    }
}

void emm_sendTAUAccept(EMMCtx emm_h){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    guint8 *pointer, t3412;
    guint8 out[156], plain[150], guti_b[11], tmsi[5];
    guint16 bearerStatus;
    gsize len=0, tlen;
    NAS_tai_list_t tAIl;
    guti_t guti;
    uint8_t lAI[5], addRes;
    Cause_t *cause;

    memset(out, 0, 156);
    memset(plain, 0, 150);

    /* Build TAU Accept*/
    pointer = plain;
    newNASMsg_EMM(&pointer, EPSMobilityManagementMessages, PlainNAS);

    encaps_EMM(&pointer, TrackingAreaUpdateAccept);

    /*EPS update result*/
    nasIe_v_t1_l(&pointer, emm->updateResult);
    pointer++;

    /* Optionals */
    /* T3412 value */
    nasIe_tv_t3(&pointer, 0x5A, &(emm->t3412), 1);
    /* GUTI */
    emmCtx_newGUTI(emm, &guti);
    guti_b[0]=0xF6;   /*1111 0 110 - spare, odd/even , GUTI id*/
    memcpy(guti_b+1, &guti, 10);
    nasIe_tlv_t4(&pointer, 0x50, guti_b, 11);
    /* TAI list */
    ecmSession_getTAIlist(emm->ecm, &tAIl, &tlen);
    nasIe_tlv_t4(&pointer, 0x54, (uint8_t*)&tAIl, tlen);
    /* EPS Bearer Context Status*/
    /* bearerStatus = hton16(0x0000); */
    /* bearerStatus = hton16(0x2000); */
    /* nasIe_tlv_t4(&pointer, 0x57, (uint8_t *)&bearerStatus, 2); */

    /* EMM cause if the attach type is different
     * This version only accepts EPS services, the combined attach
     * is not supported*/
    if(emm->updateResult == 1 ||
       emm->updateResult == 5){
        /* /\* LAI list HACK *\/ */
        /* memcpy(lAI, &(tAIl.list), 5); */
        /* nasIe_tv_t3(&pointer, 0x13, lAI, 5); */
        /* /\* MS identity : TMSI*\/ */
        /* tmsi[0]=0xf4; */
        /* memcpy(tmsi+1, &(guti.mtmsi), 4); */
        /* nasIe_tlv_t4(&pointer, 0x23, tmsi, 5); */

        if(emm->msg_additionalUpdateType){
            /* Additional Update Result*/
            addRes = 0; /*No Additional Information*/
            if(emm->msg_smsOnly){
                addRes = 2;  /*SMS only*/
            }
            nasIe_v_t1_l(&pointer, addRes);
            nasIe_v_t1_h(&pointer, 0xF);
        }
    }
    /*CHECK order*/
    /* else if(emm->updateResult == 1 && */
    /*          emm->msg_updateType == 2 && */
    /*          !emm->msg_additionalUpdateType){ */
    /*     cause = EMM_CSDomainNotAvailable; */
    /*     nasIe_tv_t3(&pointer, 0x53, (uint8_t*)&cause, 1); */
    /*     log_msg(LOG_WARNING, 0, "Attach with non EPS service requested. " */
    /*             "CS Fallback not supported"); */
    /* } */

    newNASMsg_sec(emm->parser, out, &len,
                  EPSMobilityManagementMessages,
                  IntegrityProtectedAndCiphered,
                  NAS_DownLink,
                  plain, pointer-plain);

    ecm_send(emm->ecm, out, len);
}


void emm_sendTAUReject(EMMCtx emm_h){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    guint8 *pointer, t3412;
    guint8 out[156], plain[150], guti_b[11];
    EMMCause_t cause;
    gsize len=0, tlen;
    NAS_tai_list_t tAIl;
    guti_t guti;

    memset(out, 0, 156);
    memset(plain, 0, 150);

    /* Build TAU Reject*/
    pointer = plain;
    newNASMsg_EMM(&pointer, EPSMobilityManagementMessages, PlainNAS);

    encaps_EMM(&pointer, TrackingAreaUpdateReject);

    /*EPS update result*/
    /* nasIe_v_t1_l(&pointer, 1); /\* Combined TA/LA updated *\/ */
    cause = EMM_ImplicitlyDetached;
    nasIe_v_t3(&pointer, (uint8_t*)&cause, 1); /* TA updated */

    newNASMsg_sec(emm->parser, out, &len,
                  EPSMobilityManagementMessages,
                  IntegrityProtectedAndCiphered,
                  NAS_DownLink,
                  plain, pointer-plain);

    ecm_send(emm->ecm, out, len);
    /* nas_incrementNASCount(emm->parser, NAS_DownLink); */
}

guint emm_checkIdentity(EMMCtx emm_h){
    EMMCtx_t *self = (EMMCtx_t*)emm_h;
    guint res = 1;

    if(self->imsi == 0ULL){ /* !isIMSIavailable(self) */
        emm_sendIdentityReq(self);
        res = 0;
    }
    return res;
}

void emm_removeCurrentSC(EMMCtx emm_h){
    EMMCtx_t *self = (EMMCtx_t*)emm_h;

    /* Move current SC to Old SC and
     * set new NAS Key Set ID */
    self->old_ksi = self->ksi;
    self->ksi = 7;
    self->sci = FALSE;

    self->nasUlCountForSC=0;
}

guint emm_checkAuthInformation(EMMCtx emm_h){
    EMMCtx_t *self = (EMMCtx_t*)emm_h;
    guint res = 1;

    if(!self->authQuadrsLen>0){
        s6a_GetAuthInformation(self->s6a, self, emm_sendAuthRequest, self);
        res = 0;
    }
    return res;
}

guint emm_checkSCAvailability(EMMCtx emm_h){
    EMMCtx_t *self = (EMMCtx_t*)emm_h;
    guint res = 1;

    if(self->ksi == 7                   /* SC invalid*/
       || self->ksi != self->msg_ksi){  /* SC doesn't match*/
        emm_sendAuthRequest(self);
        res = 0;
    }
    return res;
}

guint emm_checkSCActive(EMMCtx emm_h){
    EMMCtx_t *self = (EMMCtx_t*)emm_h;
    guint res = 1;

    if(!self->sci){
        emm_sendSecurityModeCommand(self);
        res = 0;
    }
    return res;
}

void emm_triggerAKAprocedure(EMMCtx emm_h){
    EMMCtx_t *self = (EMMCtx_t*)emm_h;
    emm_removeCurrentSC(self);
    if(!emm_checkIdentity(self)){
        log_msg(LOG_DEBUG, 0, "Sent Identity Request");
        emmChangeState(self, EMM_CommonProcedureInitiated);
        return;
    }
    if(!emm_checkAuthInformation(self)){
        log_msg(LOG_DEBUG, 0,"Getting info from HSS");
        return;
    }
    if(!emm_checkSCAvailability(self)){
        log_msg(LOG_DEBUG, 0,"Sent Authentication Request");
        /* Set T3460 */
        emmChangeState(self, EMM_CommonProcedureInitiated);
        return;
    }
    g_error("Authentication failure and not sending Auth Req?");
}

void emm_sendIdentityReq(EMMCtx emm_h){
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

void emm_getBearers(EMMCtx emm_h, GList **bearers){
    EMMCtx_t *self = (EMMCtx_t*)emm_h;
    esm_getBearers(self->esm, bearers);
}
