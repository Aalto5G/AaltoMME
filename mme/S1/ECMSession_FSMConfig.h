/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   ECMSession_FSMConfig.h
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  Functions to modify S1Assoc FSM
 *
 */

#ifndef ECMSESSION_FSMCONFIG_HFILE
#define ECMSESSION_FSMCONFIG_HFILE

#include <glib.h>

/**
 * ECM states
 */
typedef enum{
    ECM_Idle,
    ECM_Connected,
}ECMSessionState;

void ecm_ConfigureFSM();

void ecm_DestroyFSM();


void ecm_ChangeState(gpointer self, ECMSessionState s);

void ecm_notImplemented(gpointer self);

#endif /* ECMSESSION_FSMCONFIG_HFILE */
