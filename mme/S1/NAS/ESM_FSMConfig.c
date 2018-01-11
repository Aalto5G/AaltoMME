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
 * @file   ESM_FSMConfig.c
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  Functions to modify ESM FSM
 *
 */


#include "ESM_FSMConfig.h"

#include "ESM_Inactive.h"
#include "ESM_ActivePending.h"
#include "ESM_Active.h"
#include "ESM_ModifyPending.h"
#include "ESM_InactivePending.h"

#include "logmgr.h"

ESM_State *states;

void esmConfigureFSM(){
	states = g_new(ESM_State, 5);

	linkESMInactive(&states[0]);
	linkESMActivePending(&states[1]);
	linkESMActive(&states[2]);
	linkESMModifyPending(&states[3]);
	linkESMInactivePending(&states[4]);
}

void esmDestroyFSM(){
	g_free(states);
}


void esmChangeState(gpointer session, ESMState s){
	esm_bc_setState(session, &(states[s]));
}

void notImplemented(gpointer self){
	log_msg(LOG_ERR, 0, "Not Implemented");
}
