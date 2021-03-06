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
 * @file   ESM_Inactive.c
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  ESM State
 *
 */

#include "ESM_Inactive.h"
#include "logmgr.h"
#include "ESM_FSMConfig.h"

static void processMsg(gpointer self){
	log_msg(LOG_ERR, 0, "Not Implemented");
}

static void activateDefault(gpointer self){
	/* ESM_BearerContext_t bearer; */
	/* self->s11 = S11_newUserAttach(esm_getS11iface(self->esm), bearer->esm->emm, self, */
	/*                               esm_sendActivateDefaultEPSBearerCtxtReq, self); */
	//esm_DefaultEPSBearerContextActivation(self);
	esmChangeState(self, ActivePending);
	/*esm_send(self);*/
}


void linkESMInactive(ESM_State* s){
	s->processMsg = processMsg;
	s->activateDefault = activateDefault;
}
