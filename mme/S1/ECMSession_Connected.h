/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   ECMSession_Connected.h
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  ECM Session Connected State
 *
 */

#ifndef ECMSESSION_CONNECTED_HFILE
#define ECMSESSION_CONNECTED_HFILE

#include "ECMSession_State.h"

typedef struct{
	ECMSESSIONSTATE
}ECMSession_Connected;

void linkECMSessionConnected(ECMSession_State* s);

#endif /* ECMSESSION_CONNECTED_HFILE */
