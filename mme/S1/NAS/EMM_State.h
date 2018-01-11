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
 * @file   EMM_State.h
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  Interface to EMM State
 *
 * This module defines the generic state used in EMM State machine and
 * other common structures.
 */



#ifndef EMM_STATE_HFILE
#define EMM_STATE_HFILE

#include <glib.h>
#include "NAS.h"

typedef void (*EMM_event)(gpointer);
typedef void (*EMM_eventMsg)(gpointer, GenericNASMsg_t*);
typedef void (*EMM_eventSecMsg)(gpointer, gpointer, gsize);
typedef void (*EMM_sendMsg)(gpointer, gpointer, gsize, GList *);
typedef void (*EMM_sendESM)(gpointer, gpointer, gsize, GError **);
typedef void (*EMM_errorEvent)(gpointer, GError *);
typedef void (*EMM_eventTimeout)(gpointer, gpointer, gsize, guint);

#define EMMSTATE \
    EMM_eventMsg       processMsg;          /*  */  \
    EMM_event          authInfoAvailable;   /*  */  \
    EMM_sendMsg        attachAccept;        /*  */	\
    EMM_eventSecMsg    processSecMsg;       /*  */	\
    EMM_eventSecMsg    processSrvReq;       /*  */	\
    EMM_sendESM        sendESM;             /*  */	\
    EMM_errorEvent     processError;        /*  */	\
    EMM_eventTimeout   processTimeout;      /*  */  \
    EMM_eventTimeout   processTimeoutMax    /*  */

typedef struct{
    EMMSTATE;
}EMM_State;

/* void notExpected(gpointer); */

#endif /* EMM_STATE_HFILE */
