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
 * @file   S11_State.h
 * @Author Vicent Ferrer
 * @date   June, 2015
 * @brief  Interface to S11 State
 *
 * This module defines the generic state used in S11 State machine and
 * other common structures.
 */



#ifndef S11_STATE_HFILE
#define S11_STATE_HFILE

#include <glib.h>

typedef void (*S11_event)(gpointer);

#define S11STATE \
	S11_event processMsg;	    /*  */ \
	S11_event attach;	    /*  */ \
	S11_event detach;	    /*  */ \
	S11_event modBearer;	/* Modify Bearer */ \
	S11_event releaseAccess

typedef struct{
	S11STATE;
}S11_State;

/* void notExpected(gpointer); */

#endif /* S11_STATE_HFILE */
