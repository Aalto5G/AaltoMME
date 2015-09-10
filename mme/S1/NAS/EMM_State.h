/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
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

#define EMMSTATE \
    EMM_eventMsg       processMsg;          /*  */ \
    EMM_event          authInfoAvailable;   /*  */ \
    EMM_sendMsg        attachAccept;         /*  */	\
    EMM_eventSecMsg    processSecMsg        /*  */ \

typedef struct{
    EMMSTATE;
}EMM_State;

/* void notExpected(gpointer); */

#endif /* EMM_STATE_HFILE */
