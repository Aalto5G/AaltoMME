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
#include "NAS_ESM.h"
#include <string.h>
#include "EMM_Timers.h"

static void processAttachComplete(EMMCtx_t *emm, GenericNASMsg_t *msg);

static void emmProcessMsg(gpointer emm_h, GenericNASMsg_t* msg){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    emm_log(emm, LOG_ERR, 0, "Not Implemented");
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
    emm_log(emm, LOG_DEBUG, 0, "Local sqn %#x, packet sqn: %#x",
            nas_getLastCount(emm->parser, NAS_UpLink),
            ((guint8*)buf)[5]);
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

    if(!isAuth && nas_isAuthRequired(msg.plain.eMM.messageType)){
        emm_log(emm, LOG_INFO, 0, "Received Message with wrong MAC");
        return;
    }

    if(p == EPSSessionManagementMessages ||
       msg.header.protocolDiscriminator.v == EPSSessionManagementMessages){
        esm_processMsg(emm->esm, &(msg.plain.eSM));
        return;
    }

    switch(msg.plain.eMM.messageType){
    case AttachComplete:
        emm_stopTimer(emm, T3450);
        emm_log(emm, LOG_DEBUG, 0, "Received AttachComplete");
        processAttachComplete(emm, &msg);
        break;
    case TrackingAreaUpdateComplete:
        emm_stopTimer(emm, T3450);
        emm_log(emm, LOG_INFO, 0, "Received TrackingAreaUpdateComplete");

        if(!emm->s1BearersActive){
            /* Disconnect ECM */
            ecm_sendUEContextReleaseCommand(emm->ecm, CauseNas, CauseNas_normal_release);
        }
        emmChangeState(emm, EMM_Registered);
        break;
    case DetachRequest:
        emm_stopTimer(emm, T3450);
        emm_log(emm, LOG_DEBUG, 0, "Received DetachRequest");
        processDetachReq(emm, &msg);
        if(emm->ksi != emm->msg_ksi){
            emm_log(emm, LOG_ALERT, 0, "DetachRequest, ksi mismatch");
            return;
        }
        esm_detach(emm->esm, emm_detachAccept, emm);
        break;
    /* /\*HACK: 2 extra cases*\/ */
    /* case AttachRequest: */
    /*     emm_log(emm, LOG_ALERT, 0, "Wrong State (SPI): HACK"); */
    /*     processAttach(emm, &msg); */

    /*     if(!isAuth){ */
    /*         emm_triggerAKAprocedure(emm); */
    /*         return; */
    /*     } */

    /*     emm->nasUlCountForSC = nas_getLastCount(emm->parser, NAS_UpLink); */
    /*     emmChangeState(emm, EMM_SpecificProcedureInitiated); */
    /*     emm_processFirstESMmsg(emm); */
    /*     break; */
    /* case TrackingAreaUpdateRequest: */
    /*     emm_log(emm, LOG_ALERT, 0, "Wrong State (SPI): HACK"); */
    /*     emm_processTAUReq(emm, &msg); */

    /*     if(!isAuth){ */
    /*         emm_triggerAKAprocedure(emm); */
    /*         return; */
    /*     } */
    /*     emm->nasUlCountForSC = nas_getLastCount(emm->parser, NAS_UpLink); */

    /*     /\* HACK: Send reject to detach user and trigger reattach*\/ */
    /*     emm_sendTAUReject(emm); */
    /*     emmChangeState(emm, EMM_Deregistered); */
    /*     break; */
    /*     /\* End of HACK*\/ */

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

static void emmAttachAccept(gpointer emm_h, gpointer esm_msg, gsize msgLen, GList *bearers){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    guint8 *pointer, out[256], plain[250], count, t3412, addRes, guti_b[11], lAI[5], tmsi[5];
    guti_t guti;
    gsize len, tlen;
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
        nasIe_v_t1_l(&pointer,  emm->attachResult);
        pointer++; /*Spare half octet*/
        /* T3412 value */
        nasIe_v_t3(&pointer, &(emm->t3412), 1);
        /* TAI list */
        ecmSession_getTAIlist(emm->ecm, &tAIl, &tlen);
        nasIe_lv_t4(&pointer, (uint8_t*)&tAIl, tlen);
        /* ESM message container */
        nasIe_lv_t6(&pointer, esm_msg, msgLen);

        /* GUTI */
        if(emm->guti.mtmsi==0){
            /* This is a sanity check. The guti shall be allocated before
               this point*/
            emm_log(emm, LOG_WARNING, 0,
                "No GUTI assigned, allocating a new one");
            emmCtx_newGUTI(emm, NULL);
        }
        guti_b[0]=0xF6;   /*1111 0 110 - spare, odd/even , GUTI id*/
        memcpy(guti_b+1, &(emm->guti), sizeof(guti_t));
        nasIe_tlv_t4(&pointer, 0x50, guti_b, 11);

        /* EMM cause if the attach type is different
         * This version only accepts EPS services or Combined attach with SMS
         * only, the combined attach of EPS services and non EPS serivces is not
         * supported
         */
        if(emm->attachResult == 2){
            if(!(emm->msg_additionalUpdateType &&
                 emm->msg_smsOnly)){
                /* LAI list HACK */
                memcpy(lAI, &(tAIl.list), 5);
                nasIe_tv_t3(&pointer, 0x13, lAI, 5);
                /* MS identity : TMSI*/
                tmsi[0]=0xf4;
                memcpy(tmsi+1, &(guti.mtmsi), 4);
                nasIe_tlv_t4(&pointer, 0x23, tmsi, 5);
            }

            if(emm->msg_additionalUpdateType){
                /* Additional Update Result*/
                addRes = 0; /*No Additional Information*/
                if(emm->msg_smsOnly){
                    addRes = 2;  /*SMS only*/
                }
                nasIe_v_t1_l(&pointer, addRes);
                nasIe_v_t1_h(&pointer, 0xf);
            }
        }
        /*CHECK order*/
        /* else if(emm->attachResult == 1 && */
        /*          emm->msg_attachType == 2 && */
        /*          !emm->msg_additionalUpdateType){ */
        /*     cause = EMM_CSDomainNotAvailable; */
        /*     nasIe_tv_t3(&pointer, 0x53, (uint8_t*)&cause, 1); */
        /*     emm_log(emm, LOG_WARNING, 0, "Attach with non EPS service requested. " */
        /*             "CS Fallback not supported"); */
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
    emm->s1BearersActive = TRUE;
    emm_setTimer(emm, T3450, plain, pointer-plain);
    ecm_sendCtxtSUReq(emm->ecm, out, len, bearers);
}

static void emm_processSrvReq(gpointer emm_h, gpointer buf, gsize len){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    emm_log(emm, LOG_WARNING, 0, "Received Service request, not supported");
    /* HACK */
    emm_stop(emm);
}


static void emm_processError(gpointer emm_h, GError *err){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    emm_log(emm, LOG_WARNING, 0, "Received Error, not supported");
}


static void emm_processTimeout(gpointer emm_h, gpointer buf, gsize len,
                               EMM_TimerCode c){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    guint8 out[1500];
    gsize olen=0;
    NASMessageType_t type = (NASMessageType_t)((guint8*)buf)[1];

    switch (c) {
    case T3450: /* Attach Accept, TAU with GUTI*/
        emm_log(emm, LOG_INFO, 0, "%s expiration", EMM_TimerStr[c]);
        if(emm->sci){
            newNASMsg_sec(emm->parser, out, &olen,
                          EPSMobilityManagementMessages,
                          IntegrityProtectedAndCiphered,
                          NAS_DownLink,
                          buf, len);
            ecm_send(emm->ecm, out, olen);
        }
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
    case T3450:  /* Attach Accept, TAU with GUTI*/
        emm_log(emm, LOG_NOTICE, 0, "%s Max expirations reached. "
                "Deregistering.", EMM_TimerStr[c]);
        if(type == AttachComplete){
            emm_stop(emm);
        }else{
            emmChangeState(emm, EMM_Registered);
        }
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


void linkEMMSpecificProcedureInitiated(EMM_State* s){
    s->processMsg = emmProcessMsg;
    /* s->authInfoAvailable = emmAuthInfoAvailable; */
    s->attachAccept = emmAttachAccept;
    s->processSecMsg = emm_processSecMsg;
    s->processSrvReq = emm_processSrvReq;
    s->sendESM = emm_internalSendESM;
    s->processError = emm_processError;
    s->processTimeout = (EMM_eventTimeout) emm_processTimeout;
    s->processTimeoutMax = (EMM_eventTimeout) emm_processTimeoutMax;
}

static void processAttachComplete(EMMCtx_t *emm, GenericNASMsg_t *msg){
    GenericNASMsg_t esm_msg;
    AttachComplete_t *complete;

    complete = (AttachComplete_t*)&(msg->plain.eMM);

    dec_NAS(&esm_msg,
            complete->eSM_MessageContainer.v,
            complete->eSM_MessageContainer.l);
    emm->attachStarted = FALSE;
    emm_log(emm, LOG_INFO, 0, "NAS Attach");
    emmChangeState(emm, EMM_Registered);
    esm_processMsg(emm->esm, &(esm_msg.plain.eSM));
}
