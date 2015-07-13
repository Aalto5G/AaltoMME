/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S11_FSMConfig.h
 * @Author Vicent Ferrer
 * @date   June, 2015
 * @brief  Functions to modify S11 FSM
 *
 */

#ifndef S11_FSMCONFIG_HFILE
#define S11_FSMCONFIG_HFILE

#include <glib.h>
#include "S11_State.h"

typedef enum{
	noCtx,           /**< */
	wCtxRsp,         /**< */
	ulCtx,          /**< */
	wModBearerRsp,   /**< */
	ctx,             /**< */
	wDel,            /**< */
}S11State;

void s11ConfigureFSM();

void s11DestroyFSM();


void s11changeState(gpointer, S11State);

#endif /* S11_FSMCONFIG_HFILE */
