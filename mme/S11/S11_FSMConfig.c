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
 * @file   S11_FSMConfig.c
 * @Author Vicent Ferrer
 * @date   June, 2015
 * @brief  Functions to modify S11 FSM
 *
 */


//#include "MME_S11.c"
#include "S11_FSMConfig.h"
#include "S11_User.h"

#include "S11_NoCtx.h"
#include "S11_WCtxRsp.h"
#include "S11_UlCtx.h"
#include "S11_WModBearerRsp.h"
#include "S11_Ctx.h"
#include "S11_WDel.h"

S11_State *s11_states;

void s11ConfigureFSM(){
    s11_states = g_new(S11_State, 6);

    linkNoCtx(&s11_states[0]);
    linkWCtxRsp(&s11_states[1]);
    linkUlCtx(&s11_states[2]);
    linkWModBearerRsp(&s11_states[3]);
    linkCtx(&s11_states[4]);
    linkWDel(&s11_states[5]);
}

void s11DestroyFSM(){
    g_free(s11_states);
}


void s11changeState(gpointer session, S11State s){
    s11u_setState(session, &(s11_states[s]));
}
