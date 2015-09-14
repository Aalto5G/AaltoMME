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



#ifndef S1ASSOC_STATE_HFILE
#define S1ASSOC_STATE_HFILE

#include <glib.h>
#include "S1AP.h"

typedef void (*S1_event1)(gpointer);
typedef void (*S1_processMsgEvent)(gpointer, S1AP_Message_t *, int);
typedef void (*S1_disconnect)(gpointer, void (*cb)(gpointer), gpointer);

#define S1STATE \
    S1_processMsgEvent processMsg;      /*  */ \
    S1_disconnect      disconnect;



typedef struct{
    S1STATE;
}S1Assoc_State;

/* void notExpected(gpointer); */

#endif /* S1ASSOC_STATE_HFILE */
