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

#include "S11_User.h"

#define S11STATE \
	void *(processMsg)(gpointer);	    /*  */ \
	void *(attach)(gpointer);	    /*  */ \
	void *(detach)(gpointer);	    /*  */ \
	void *(modBearer)(gpointer);	/* Modify Bearer */ \

typedef{
	S11STATE;
}S11_State;

/* void notExpected(gpointer); */

#endif /* S11_STATE_HFILE */
