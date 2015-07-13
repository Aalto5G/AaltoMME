/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   ESM_FSMConfig.h
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  Functions to modify ESM FSM
 *
 */

#ifndef ESM_FSMCONFIG_HFILE
#define ESM_FSMCONFIG_HFILE

#include <glib.h>
#include "ESM_BearerContext.h"

/**
 * ESM states, also know as bearer context
 */
typedef enum{
	Inactive,
	ActivePending,
	Active,
	ModifyPending,
	InactivePending,
}ESMState;

void esmConfigureFSM();

void esmDestroyFSM();


void esmChangeState(gpointer, ESMState);

#endif /* ESM_FSMCONFIG_HFILE */
