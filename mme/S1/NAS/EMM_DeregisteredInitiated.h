/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   EMM_DeregisteredInitiated.h
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  EMM State
 *
 */

#ifndef EMM_DEREGISTEREDINITIATED_HFILE
#define EMM_DEREGISTEREDINITIATED_HFILE

#include "EMM_State.h"

typedef struct{
	EMMSTATE;
}EMMDeregisteredInitiated;

void linkEMMDeregisteredInitiated(EMM_State* s);

#endif /* EMM_DEREGISTEREDINITIATED_HFILE */
