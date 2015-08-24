/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
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



#ifndef ESM_STATE_HFILE
#define ESM_STATE_HFILE

#include <glib.h>
#include "S1AP.h"

typedef void (*S1_event1)(gpointer);
typedef void (*S1_processMsgEvent)(gpointer, S1AP_Message_t *, int);

#define S1STATE \
    S1_processMsgEvent processMsg;      /*  */ \
    S1_event1          disconnect;


typedef struct{
    S1STATE;
}S1Assoc_State;

/* void notExpected(gpointer); */

#endif /* ESM_STATE_HFILE */
