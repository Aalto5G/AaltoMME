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
 * @file   MME_S1.h
 * @Author Vicent Ferrer
 * @date   May, 2013
 * @brief  MME S1 interface protocol state machine.
 *
 * This module implements the S1 interface state machine.
 */

#ifndef MME_S1_HFILE
#define MME_S1_HFILE

#include <glib.h>

#include "MME.h"
#include "S1AP.h"

#define S1AP_NONUESIGNALING_STREAM0 0


/* ======================================================================
 * S1 Type definitions
 * ====================================================================== */

typedef gpointer S1;

/* ======================================================================
 * S1 Tool API
 * ====================================================================== */


/**@brief S1 Interface constructor
 * @param [in] mme pointer to MME structure
 * @returns interface handler
 *
 * This function creates the interface S1, use s1_free to deallocate it.
 * */
gpointer s1_init(gpointer mme);

/**@brief S1 Interface destructor
 * @param [in] s1_h S1 interface handler
 *
 * This function deallocates the interface S1 created using s1_init.
 * */
void s1_free(S1 s1_h);

#endif /* MME_S1_HFILE */
