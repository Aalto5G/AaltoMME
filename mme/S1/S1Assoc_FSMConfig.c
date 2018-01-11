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
 * @file   S1Assoc_FSMConfig.c
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  Functions to modify S1Assoc FSM
 *
 */

#include "S1Assoc_priv.h"
#include "S1Assoc_FSMConfig.h"
#include "ECMSession_FSMConfig.h"
#include "ECMSession_priv.h"

/* Include States*/
#include "S1Assoc_NotConfigured.h"
#include "S1Assoc_Active.h"

#include "logmgr.h"

S1Assoc_State *s1_states;

void s1ConfigureFSM(){
    s1_states = g_new(S1Assoc_State, 2);
    ecm_ConfigureFSM();

    linkS1AssocNotConfigured(&s1_states[0]);
    linkS1AssocActive(&s1_states[1]);
}

void s1DestroyFSM(){
    ecm_DestroyFSM();
    g_free(s1_states);
}


void s1ChangeState(gpointer s1, S1AssocState s){
    s1Assoc_setState(s1, &(s1_states[s]), s);
}

void s1notImplemented(gpointer self){
    log_msg(LOG_ERR, 0, "Not Implemented");
}
