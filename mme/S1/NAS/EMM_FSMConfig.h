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
 * @file   EMM_FSMConfig.h
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  Functions to modify EMM FSM
 *
 */

#ifndef EMM_FSMCONFIG_HFILE
#define EMM_FSMCONFIG_HFILE

#include <glib.h>

/**
 * EMM STATES, More info in 3gpp 24.301 clause 5.1.3.4
 */
typedef enum{
    EMM_Deregistered,
    EMM_Registered,
    EMM_SpecificProcedureInitiated,
    EMM_CommonProcedureInitiated,
    EMM_DeregisteredInitiated,
}EMMState;

static const char *EMMStateName[] = {"EMM-DER", /**< EMM-Deregistered */
                                     "EMM-REG", /**< EMM-Registered */
                                     "EMM-SPI", /**< EMM-SpecificProcedureInitiated */
                                     "EMM-CPI", /**< EMM-CommonProcedureInitiated */
                                     "EMM-DEI", /**< EMM-DeregisteredInitiated" */
                                     };

void emmConfigureFSM();

void emmDestroyFSM();


void emmChangeState(gpointer self, EMMState s);

void emmNotImplemented(gpointer self);

#endif /* EMM_FSMCONFIG_HFILE */
