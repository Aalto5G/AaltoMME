/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   EMM_SpecificProcedureInitiated.c
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  EMM State
 *
 */

#include "EMM_SpecificProcedureInitiated.h"
#include "logmgr.h"
#include "EMM_FSMConfig.h"
#include "NAS.h"
#include "ECMSession_priv.h"
#include "NAS_EMM_priv.h"
#include <string.h>

static void processAttachComplete(EMMCtx_t *emm, GenericNASMsg_t *msg);

static void emmProcessMsg(gpointer emm_h, GenericNASMsg_t* msg){
    log_msg(LOG_ERR, 0, "Not Implemented");
}

static void emm_processSecMsg(gpointer emm_h, gpointer buf, gsize len){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    GenericNASMsg_t msg;
    SecurityHeaderType_t s;
    ProtocolDiscriminator_t p;
    guint8 isAuth = 0, res;
    nas_getHeader(buf, len, &s, &p);


    if(!emm->sci){
        g_error("Wrong state to be with a security context");
    }

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

    if(!isAuth && nas_isAuthRequired(msg.plain.eMM.messageType)){
        log_msg(LOG_INFO, 0, "Received Message with wrong MAC");
        return;
    }

    if(p == EPSSessionManagementMessages ||
       msg.header.protocolDiscriminator.v == EPSSessionManagementMessages){
        esm_processMsg(emm->esm, &(msg.plain.eSM));
        return;
    }

    switch(msg.plain.eMM.messageType){
    case AttachComplete:
        log_msg(LOG_DEBUG, 0, "Received AttachComplete");
        processAttachComplete(emm, &msg);
        break;
    case TrackingAreaUpdateComplete:
        log_msg(LOG_INFO, 0, "Received TrackingAreaUpdateComplete, not implemented");
        emmChangeState(emm, EMM_Registered);
        break;

    /*HACK: 2 extra cases*/
    case AttachRequest:
        processAttach(emm, &msg);

        if(!isAuth){
            emm_triggerAKAprocedure(emm);
            return;
        }

        emm->nasUlCountForSC = nas_getLastCount(emm->parser, NAS_UpLink);
        emmChangeState(emm, EMM_SpecificProcedureInitiated);
        emm_processFirstESMmsg(emm);
        break;
    case TrackingAreaUpdateRequest:
        emm_processTAUReq(emm, &msg);

        if(!isAuth){
            emm_triggerAKAprocedure(emm);
            return;
        }
        emm->nasUlCountForSC = nas_getLastCount(emm->parser, NAS_UpLink);

        /* HACK: Send reject to detach user and trigger reattach*/
        emm_sendTAUReject(emm);
        emmChangeState(emm, EMM_Deregistered);
        break;
        /* End of HACK*/

    default:
        log_msg(LOG_WARNING, 0,
                "NAS Message type (%u) not recognized in this context",
                msg.plain.eMM.messageType);
    }

}

static void emmAttachAccept(gpointer emm_h, gpointer esm_msg, gsize msgLen, GList *bearers){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    guint8 *pointer, out[256], plain[250], count, t3412, guti_b[11];
    guti_t guti;
    guint32 len;
    gsize tlen;
    NAS_tai_list_t tAIl;
    EMMCause_t cause;

    memset(out, 0, 156);
    memset(plain, 0, 150);
    pointer = plain;

    if (emm->attachStarted == TRUE){
        emm->attachStarted = FALSE;
        /* Build Attach Accept*/
        newNASMsg_EMM(&pointer, EPSMobilityManagementMessages, PlainNAS);

        encaps_EMM(&pointer, AttachAccept);

        /* EPS attach result.*/
        /* nasIe_v_t1_l(&pointer,  1); /\* EPS only*\/ */
        nasIe_v_t1_l(&pointer,  emm->attachType);
        pointer++; /*Spare half octet*/
        /* T3412 value */
        t3412 = 0x23; /* 3 min*/
        /* t3412 = 0x49; /\* 54 min, default *\/ */
        nasIe_v_t3(&pointer, &t3412, 1);
        /* TAI list */
        ecmSession_getTAIlist(emm->ecm, &tAIl, &tlen);
        nasIe_lv_t4(&pointer, (uint8_t*)&tAIl, tlen);
        /* ESM message container */
        nasIe_lv_t6(&pointer, esm_msg, msgLen);

        /* GUTI */
        emmCtx_newGUTI(emm, &guti);
        guti_b[0]=0xF6;   /*1111 0 110 - spare, odd/even , GUTI id*/
        memcpy(guti_b+1, &guti, 10);
        nasIe_tlv_t4(&pointer, 0x50, guti_b, 11);

        /* EMM cause if the attach type is different
         * This version only accepts EPS services, the combined attach
         * is not supported*/
        /* if(emm->attachType == 2){ */
        /*     cause = EMM_CSDomainNotAvailable; */
        /*     nasIe_tv_t3(&pointer, 0x53, (uint8_t*)&cause, 1); */
        /* } */

        newNASMsg_sec(emm->parser, out, &len,
                      EPSMobilityManagementMessages,
                      IntegrityProtectedAndCiphered,
                      NAS_DownLink,
                      plain, pointer-plain);
        /* nas_incrementNASCount(emm->parser, NAS_DownLink); */
    }else{
        return;
    }
    ecm_sendCtxtSUReq(emm->ecm, out, len, bearers);
}

void linkEMMSpecificProcedureInitiated(EMM_State* s){
    s->processMsg = emmProcessMsg;
    /* s->authInfoAvailable = emmAuthInfoAvailable; */
    s->attachAccept = emmAttachAccept;
    s->processSecMsg = emm_processSecMsg;
    s->sendESM = emm_internalSendESM;
}

static void processAttachComplete(EMMCtx_t *emm, GenericNASMsg_t *msg){
    GenericNASMsg_t esm_msg;
    AttachComplete_t *complete;

    complete = (AttachComplete_t*)&(msg->plain.eMM);

    dec_NAS(&esm_msg,
            complete->eSM_MessageContainer.v,
            complete->eSM_MessageContainer.l);
    emm->attachStarted = FALSE;
    log_msg(LOG_INFO, 0, "UE (IMSI: %llu) NAS Attach", emmCtx_getIMSI(emm));
    emmChangeState(emm, EMM_Registered);
    esm_processMsg(emm->esm, &(esm_msg.plain.eSM));
}
