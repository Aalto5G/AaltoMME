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
#include "NAS_ESM.h"
#include "EMM_Timers.h"

static int emm_selectUpdateType(EMMCtx_t * emm);

static void emmProcessMsg(gpointer emm_h, GenericNASMsg_t* msg){
    log_msg(LOG_WARNING, 0,
            "NAS Message type (%x) not recognized in this context",
            msg->plain.eMM.messageType);
}

static void emm_processSecMsg(gpointer emm_h, gpointer buf, gsize len){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;

    GenericNASMsg_t msg;
    SecurityHeaderType_t s;
    ProtocolDiscriminator_t p;
    guint8 isAuth = 0, res=0;
    guti_t msg_guti;
    guint8 msg_ksi;

    nas_getHeader(buf, len, &s, &p);

    if(!emm->sci){
        g_error("Wrong state to be with a security context");
    }

    res = nas_authenticateMsg(emm->parser, buf, len, NAS_UpLink, (uint8_t*)&isAuth);
    log_msg(LOG_INFO, 0, "Local sqn %#x, packet sqn: %#x",
            nas_getLastCount(emm->parser, NAS_UpLink),
            ((guint8*)buf)[5]);
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
    case DetachRequest:
        log_msg(LOG_DEBUG, 0, "Received DetachRequest");
        processDetachReq(emm, &msg);
        if(emm->ksi != emm->msg_ksi){
            log_msg(LOG_ALERT, 0, "DetachRequest, ksi mismatch");
            return;
        }
        esm_detach(emm->esm, emm_detachAccept, emm);
        break;
    case TrackingAreaUpdateRequest:
        emm_processTAUReq(emm, &msg);
        if(!emm_selectUpdateType(emm)){
            return;
        }

        if(!isAuth){
            emm_triggerAKAprocedure(emm);
            return;
        }

        emm->nasUlCountForSC = nas_getLastCount(emm->parser, NAS_UpLink);

        emm_sendTAUAccept(emm);
        emmChangeState(emm, EMM_SpecificProcedureInitiated);
        break;
    default:
        log_msg(LOG_WARNING, 0,
                "NAS Message type (%u) not recognized in EMM Registered",
                msg.plain.eMM.messageType);
    }

}

static void emm_processSrvReq(gpointer emm_h, gpointer buf, gsize len){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    SecurityHeaderType_t s;
    ProtocolDiscriminator_t p;
    guint8 isAuth = 0, res=0;
    GList *sessions;

    nas_getHeader(buf, len, &s, &p);

    if(!emm->sci){
        g_error("Wrong state (EMM Reg) to be without a security context");
    }

    res = nas_authenticateMsg(emm->parser, buf, len, NAS_UpLink, (uint8_t*)&isAuth);
    log_msg(LOG_INFO, 0, "Local sqn %#x, packet sqn: %#x",
            nas_getLastCount(emm->parser, NAS_UpLink),
            ((guint8*)buf)[1]&0x1F);
    if(res==0){
        /* EH Send Indication Error*/
        g_error("Received malformed NAS packet");
    }else if(res==2){
        /* EH trigger AKA procedure */
        log_msg(LOG_WARNING, 0, "Wrong SQN Count");
        return;
    }

    if(p != EPSMobilityManagementMessages){
        log_msg(LOG_INFO, 0, "Malformed packet");
        return;
    }
    emm->msg_ksi = (*((guint8*)buf+1)&0xe0)>>5;
    if(emm->ksi != emm->msg_ksi || !isAuth){
        log_msg(LOG_INFO, 0, "Received Message with wrong MAC");
        /* Trigger AKA*/
        return;
    }
    emm->nasUlCountForSC = nas_getLastCount(emm->parser, NAS_UpLink);

    emm->s1BearersActive = TRUE;
    esm_getSessions(emm->esm, &sessions);
    ecm_sendCtxtSUReq(emm->ecm, NULL, 0, sessions);
    g_list_free(sessions);
}


static void emm_processError(gpointer emm_h, GError *err){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    log_msg(LOG_WARNING, 0, "Received Error, not supported in EMM Registered");
}


static void emm_processTimeout(gpointer emm_h, gpointer buf, gsize len,
                               EMM_TimerCode c){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    log_msg(LOG_WARNING, 0, "Timeout %s, not supported in EMM Registered",
            EMM_TimerStr[c]);
}


static void emm_processTimeoutMax(gpointer emm_h, gpointer buf, gsize len,
                                  EMM_TimerCode c){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    log_msg(LOG_WARNING, 0, "Timeout Max %s, not supported in EMM Registered",
            EMM_TimerStr[c]);
}

void linkEMMRegistered(EMM_State* s){
    s->processMsg = emmProcessMsg;
    /* s->authInfoAvailable = emmAuthInfoAvailable; */
    s->attachAccept = NULL;
    s->processSecMsg = emm_processSecMsg;
    s->processSrvReq = emm_processSrvReq;
    s->sendESM = emm_internalSendESM;
    s->processError = emm_processError;
    s->processTimeout = (EMM_eventTimeout) emm_processTimeout;
    s->processTimeoutMax = (EMM_eventTimeout) emm_processTimeoutMax;
}

static int emm_selectUpdateType(EMMCtx_t * emm){
    switch(emm->msg_updateType){
    case 0:
    case 4:
    case 5:
        /* TA updating */
        emm->updateResult = 0;
        /* emm->updateResult = 4; if ISR*/
        return 1;
    case 1:
        /* Combined TA/LA updating*/
        if(emm->msg_additionalUpdateType && emm->msg_smsOnly){
            emm->updateResult = 1;
        }else{
            /* log_msg(LOG_ALERT, 0, */
            /*         "Answering Combined TA/LA updating with TA updated"); */
            /* emm->updateResult = 0; */
            /* HACK */
            emm->updateResult = 1;
        }
        /* emm->updateResult = 5; if ISR*/
        return 1;
    case 2:
        /* Combined TA/LA updating with IMSI attach*/
        if(emm->msg_additionalUpdateType && emm->msg_smsOnly){
            emm->updateResult = 1;
        }else{
            log_msg(LOG_ALERT, 0,
                    "Answering Combined TA/LA updating with IMSI attach "
                    "with TA updated");
            emm->updateResult = 0;
        }
        /* emm->updateResult = 5; if ISR*/
        return 1;
    case 3:
        /* Periodic Updating*/
        emm->updateResult = 0;
        /* emm->updateResult = 4; if ISR*/
        return 1;
    default:
        /*Reserved*/
        /*Reject*/
        log_msg(LOG_ERR, 0,
                "Reserved Updating type received");
        return 0;
    }
}
