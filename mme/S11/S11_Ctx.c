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
 * @file   S11_Ctx.c
 * @Author Vicent Ferrer
 * @date   June, 2015
 * @brief  S11 State
 *
 */

#include "S11_Ctx.h"
#include "logmgr.h"
#include "S11_FSMConfig.h"
#include "S11_User.h"

static void s11u_processMsg(gpointer self){
    log_msg(LOG_ERR, 0, "Not Implemented");
}

static void s11u_attach(gpointer self){
    log_msg(LOG_ERR, 0, "Not Implemented");
}

static void s11u_detach(gpointer self){
    log_msg(LOG_DEBUG, 0, "Deleting Bearer Context");
    sendDeleteSessionReq(self);
    s11changeState(self, wDel);
}

static void s11u_modBearer(gpointer self){
    log_msg(LOG_DEBUG, 0, "Sending Modify Bearer Request");
    sendModifyBearerReq(self);
    s11changeState(self, wModBearerRsp);
}

static void s11u_releaseAccess(gpointer self){
    log_msg(LOG_DEBUG, 0, "Sending Release Access Bearers Request");
    sendReleaseAccessBearersReq(self);
    s11changeState(self, wModBearerRsp);
}


void linkCtx(S11_State* s){
    s->processMsg = s11u_processMsg;
    s->attach = s11u_attach;
    s->detach = s11u_detach;
    s->modBearer = s11u_modBearer;
    s->releaseAccess = s11u_releaseAccess;
}
