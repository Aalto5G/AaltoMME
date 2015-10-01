/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S11_WCtxRsp.c
 * @Author Vicent Ferrer
 * @date   June, 2015
 * @brief  S11 State
 *
 */

#include "S11_WCtxRsp.h"
#include "logmgr.h"
#include "S11_FSMConfig.h"
#include "S11_User.h"
#include "gtp.h"

static void s11u_processMsg(gpointer self){
    GError *err = NULL;
    parseIEs(self);
    switch(getMsgType(self)){
    case GTP2_CREATE_SESSION_RSP:
        if(!accepted(self)){
            log_msg(LOG_WARNING, 0, "Create Session request rejected "
                    "Cause %d", cause(self));
        }
        parseCtxRsp(self, &err);
        if(err!=NULL){
            log_msg(LOG_ERR, 0, err->message);
            g_error_free (err);
            return;
        }
        s11changeState(self, ulCtx);
        returnControl(self);
        break;
    default:
        log_msg(LOG_DEBUG, 0, "Msg for this state not Implemented");
        break;
    }
}

static void s11u_attach(gpointer self){
    log_msg(LOG_ERR, 0, "Not Implemented");
}

static void s11u_detach(gpointer self){
    log_msg(LOG_ERR, 0, "Not Implemented");
}

static void s11u_modBearer(gpointer self){
    log_msg(LOG_ERR, 0, "Not Implemented");
}

static void s11u_releaseAccess(gpointer self){
    log_msg(LOG_ERR, 0, "Not Implemented");
}


void linkWCtxRsp(S11_State* s){
    s->processMsg = s11u_processMsg;
    s->attach = s11u_attach;
    s->detach = s11u_detach;
    s->modBearer = s11u_modBearer;
    s->releaseAccess = s11u_releaseAccess;
}
