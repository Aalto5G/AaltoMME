/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S11_State.h
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
	ESM_event processMsg;	    /*  */ \


typedef struct{
	ESMSTATE;
}ESM_State;

/* void notExpected(gpointer); */

#endif /* ESM_STATE_HFILE */
