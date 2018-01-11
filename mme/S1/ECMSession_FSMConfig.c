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
 * @file   ECMSession_FSMConfig.c
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  Functions to modify ECMSession FSM
 *
 */

#include "S1Assoc.h"
#include "ECMSession_FSMConfig.h"
#include "ECMSession_priv.h"
/* Include States*/
#include "ECMSession_Idle.h"
#include "ECMSession_Connected.h"

#include "EMM_FSMConfig.h"

#include "logmgr.h"

ECMSession_State *ecm_states;


void ecm_ConfigureFSM(){
    ecm_states = g_new(ECMSession_State, 2);

    emmConfigureFSM();

    linkECMSessionIdle(&ecm_states[0]);
    linkECMSessionConnected(&ecm_states[1]);
}

void ecm_DestroyFSM(){
    emmDestroyFSM();
    g_free(ecm_states);
}


void ecm_ChangeState(gpointer ecm, ECMSessionState s){
    ecmSession_setState(ecm, &(ecm_states[s]), s);
}

void ecm_notImplemented(gpointer self){
    log_msg(LOG_ERR, 0, "Not Implemented");
}
