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
 * @file   ESM_State.h
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  Interface to ESM State
 *
 * This module defines the generic state used in ESM State machine and
 * other common structures.
 */



#ifndef ESM_STATE_HFILE
#define ESM_STATE_HFILE

#include <glib.h>

typedef void (*ESM_event)(gpointer);

#define ESMSTATE \
    ESM_event processMsg;       /*  */ \
    ESM_event activateDefault;   /*  */ \


typedef struct{
    ESMSTATE;
}ESM_State;

/* void notExpected(gpointer); */

#endif /* ESM_STATE_HFILE */
