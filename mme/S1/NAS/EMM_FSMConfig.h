/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   EMM_FSMConfig.h
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  Functions to modify EMM FSM
 *
 */

#ifndef EMM_FSMCONFIG_HFILE
#define EMM_FSMCONFIG_HFILE

#include <glib.h>
#include "EMMCtx.h"

/**
 * EMM STATES, More info in 3gpp 24.301 clause 5.1.3.4
 */
typedef enum{
    EMM_Deregistered,
    EMM_Registered,
    EMM_SpecificProcedureInitiated,
    EMM_CommonProcedureInitiated,
    EMM_DeregisteredInitiated,
}EMMState;

void emmConfigureFSM();

void emmDestroyFSM();


void emmChangeState(gpointer self, EMMState s);

void emmNotImplemented(gpointer self);

#endif /* EMM_FSMCONFIG_HFILE */
