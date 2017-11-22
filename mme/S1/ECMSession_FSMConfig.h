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
 * @file   ECMSession_FSMConfig.h
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  Functions to modify S1Assoc FSM
 *
 */

#ifndef ECMSESSION_FSMCONFIG_HFILE
#define ECMSESSION_FSMCONFIG_HFILE

#include <glib.h>

/**
 * ECM states
 */
typedef enum{
    ECM_Idle,
    ECM_Connected,
}ECMSessionState;

static const char *ECMStateName[] = {"ECM-Idle",
                                     "ECM-Conn"};

void ecm_ConfigureFSM();

void ecm_DestroyFSM();


void ecm_ChangeState(gpointer self, ECMSessionState s);

void ecm_notImplemented(gpointer self);

#endif /* ECMSESSION_FSMCONFIG_HFILE */
