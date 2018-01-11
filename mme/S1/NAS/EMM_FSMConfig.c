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
 * @file   EMM_FSMConfig.c
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  Functions to modify EMM FSM
 *
 */


#include "EMM_FSMConfig.h"

#include "EMM_Deregistered.h"
#include "EMM_Registered.h"
#include "EMM_SpecificProcedureInitiated.h"
#include "EMM_CommonProcedureInitiated.h"
#include "EMM_DeregisteredInitiated.h"

#include "ESM_FSMConfig.h"
#include "EMMCtx.h"

#include "logmgr.h"

EMM_State *emm_states;

void emmConfigureFSM(){
    emm_states = g_new(EMM_State, 5);

    esmConfigureFSM();

    linkEMMDeregistered(&emm_states[0]);
    linkEMMRegistered(&emm_states[1]);
    linkEMMSpecificProcedureInitiated(&emm_states[2]);
    linkEMMCommonProcedureInitiated(&emm_states[3]);
    linkEMMDeregisteredInitiated(&emm_states[4]);
}

void emmDestroyFSM(){
    esmDestroyFSM();
    g_free(emm_states);
}

void emmChangeState(gpointer ctx, EMMState s){
    emm_setState(ctx, &(emm_states[s]), s);
}

void emmNotImplemented(gpointer self){
    emm_log(self, LOG_ERR, 0, "Not Implemented");
}
