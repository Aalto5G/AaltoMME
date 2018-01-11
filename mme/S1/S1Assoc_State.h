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
 * @file   S1Assoc_State.h
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  Interface to S1 Association State
 *
 * This module defines the generic state used in S1 Association State
 * machine and other common structures.
 */



#ifndef S1ASSOC_STATE_HFILE
#define S1ASSOC_STATE_HFILE

#include <glib.h>
#include "S1AP.h"

typedef void (*S1_event1)(gpointer);
typedef void (*S1_processMsgEvent)(gpointer, S1AP_Message_t *, int, GError**);
typedef void (*S1_disconnect)(gpointer, void (*cb)(gpointer), gpointer);

#define S1STATE \
    S1_processMsgEvent processMsg      /*  */ \


typedef struct{
    S1STATE;
}S1Assoc_State;

/* void notExpected(gpointer); */

#endif /* S1ASSOC_STATE_HFILE */
