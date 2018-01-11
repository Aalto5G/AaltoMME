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
 * @file   MME_S6a.h
 * @Author Vicent Ferrer
 * @date   June, 2013
 * @brief  MME S6a interface protocol state machine.
 *
 * This module implements the S6a interface state machine.
 * It is currently only and emulator
 */

#ifndef MME_S6a_HFILE
#define MME_S6a_HFILE

#include <glib.h>

#include "MME.h"
#include "S6a.h"
#include "EMMCtx_iface.h"

#define MME_S6a mme_S6a_quark()

GQuark mme_S6a_quark();

typedef enum{
    S6a_UNKNOWN_EPS_SUBSCRIPTION,
    S6a_UNKNOWN_ERROR,
}S6aCause;

/**************************************************/
/* Interface to MME.c                             */
/**************************************************/

gpointer s6a_init(gpointer mme);

void s6a_free(gpointer s6a);


/**************************************************/
/* Interface to NAS_FSM.c                         */
/**************************************************/

void s6a_GetAuthInformation(gpointer s6a_h, EMMCtx emm,
                            void(*cb)(gpointer),
                            void(*error_cb)(gpointer, GError *err),
                            gpointer args);

void s6a_SynchAuthVector(gpointer s6a_h,  EMMCtx emm, uint8_t *auts,
                         void(*cb)(gpointer),
                         void(*error_cb)(gpointer, GError *err),
                         gpointer args);

void s6a_UpdateLocation(gpointer s6a_h, EMMCtx emm,
                        void(*cb)(gpointer), gpointer args);


#endif /* MME_S6a_HFILE */
