/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   ECMSession_Idle.h
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  ECM Session Idle State
 *
 */

#ifndef ECMSESSION_IDLE_HFILE
#define ECMSESSION_IDLE_HFILE

#include "ECMSession_State.h"

typedef struct{
    ECMSESSIONSTATE
}ECMSession_Idle;

void linkECMSessionIdle(ECMSession_State* s);

#endif /* ECMSESSION_IDLE_HFILE */
