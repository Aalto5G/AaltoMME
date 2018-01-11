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
 * @file   S11_FSMConfig.h
 * @Author Vicent Ferrer
 * @date   June, 2015
 * @brief  Functions to modify S11 FSM
 *
 */

#ifndef S11_FSMCONFIG_HFILE
#define S11_FSMCONFIG_HFILE

#include <glib.h>
#include "S11_State.h"

typedef enum{
	noCtx,           /**< */
	wCtxRsp,         /**< */
	ulCtx,          /**< */
	wModBearerRsp,   /**< */
	ctx,             /**< */
	wDel,            /**< */
}S11State;

void s11ConfigureFSM();

void s11DestroyFSM();


void s11changeState(gpointer, S11State);

#endif /* S11_FSMCONFIG_HFILE */
