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
 * @file   ECMSession_State.h
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  Interface to ECM Session State
 *
 * This module defines the generic state used in ECM Session State
 * machine and other common structures.
 */



#ifndef ECM_STATE_HFILE
#define ECM_STATE_HFILE

#include <glib.h>
#include "S1AP.h"

typedef void (*ECM_event1)(gpointer, uint32_t, uint32_t);
typedef void (*ECM_processMsgEvent)(gpointer, S1AP_Message_t *, int);

#define ECMSESSIONSTATE                                \
    ECM_processMsgEvent processMsg;      /*  */ \
    ECM_event1          release;


typedef struct{
    ECMSESSIONSTATE;
}ECMSession_State;

/* void notExpected(gpointer); */

#endif /* ESM_STATE_HFILE */
