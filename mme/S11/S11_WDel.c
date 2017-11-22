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
 * @file   S11_WDel.c
 * @Author Vicent Ferrer
 * @date   June, 2015
 * @brief  S11 State
 *
 */

#include "S11_WDel.h"
#include "logmgr.h"
#include "S11_FSMConfig.h"
#include "S11_User.h"
#include "gtp.h"


static void s11u_processMsg(gpointer self){
    GError *err = NULL;
    parseIEs(self);
    switch(getMsgType(self)){
    case GTP2_DELETE_SESSION_RSP:
        if(!accepted(self)){
            log_msg(LOG_WARNING, 0, "Create Session request rejected "
                    "Cause %d", cause(self));
        }
        parseDelCtxRsp(self, &err);
        if(err!=NULL){
            log_msg(LOG_ERR, 0, err->message);
            g_error_free (err);
            return;
        }
        s11changeState(self, noCtx);
        returnControlAndRemoveSession(self);
        break;
    default:
        log_msg(LOG_DEBUG, 0, "Msg for this state not Implemented");
        break;
    }
}

static void s11u_attach(gpointer self){

}

static void s11u_detach(gpointer self){

}

static void s11u_modBearer(gpointer self){

}

static void s11u_releaseAccess(gpointer self){
    log_msg(LOG_ERR, 0, "Not Implemented");
}


void linkWDel(S11_State* s){
    s->processMsg = s11u_processMsg;
    s->attach = s11u_attach;
    s->detach = s11u_detach;
    s->modBearer = s11u_modBearer;
    s->releaseAccess = s11u_releaseAccess;
}
