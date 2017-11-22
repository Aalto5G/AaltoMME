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
 * @file   S1Assoc_FSMConfig.h
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  Functions to modify S1Assoc FSM
 *
 */

#ifndef S1ASSOC_FSMCONFIG_HFILE
#define S1ASSOC_FSMCONFIG_HFILE

#include <glib.h>

/**
 * S1Assoc states
 */
typedef enum{
    S1_NotConfigured,
    S1_Active,
}S1AssocState;


static const char *S1AssocStateName[] = {"S1-NoConf",
                                         "S1-Active"};

void s1ConfigureFSM();

void s1DestroyFSM();


void s1ChangeState(gpointer self, S1AssocState s);

void s1notImplemented(gpointer self);

#endif /* S1Assoc_FSMCONFIG_HFILE */
