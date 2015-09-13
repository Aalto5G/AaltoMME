/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
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
