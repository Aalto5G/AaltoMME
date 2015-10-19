/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   EMM_DeregisteredInitiated.c
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  EMM State
 *
 */

#include "EMM_DeregisteredInitiated.h"
#include "logmgr.h"
#include "EMM_FSMConfig.h"
#include "NAS.h"
#include "EMM_Timers.h"


static void emmProcessMsg(gpointer emm_h, GenericNASMsg_t* msg){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    emm_log(emm, LOG_ERR, 0, "Not Implemented");
}

static void emm_processSecMsg(gpointer emm_h, gpointer buff, gsize len){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;

    emm_log(emm, LOG_ERR, 0, "Received unexpected NAS message with security header");

}

static void emm_processSrvReq(gpointer emm_h, gpointer buf, gsize len){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    emm_log(emm, LOG_WARNING, 0, "Received Service request, not supported in this context");
}

static void emm_processError(gpointer emm_h, GError *err){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    emm_log(emm, LOG_WARNING, 0, "Received Error, not supported");
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

void linkEMMDeregisteredInitiated(EMM_State* s){
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
