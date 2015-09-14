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
