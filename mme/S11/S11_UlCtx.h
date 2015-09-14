/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S11_UlCtx.h
 * @Author Vicent Ferrer
 * @date   June, 2015
 * @brief  S11 State
 *
 */

#ifndef S11_ULCTX_HFILE
#define S11_ULCTX_HFILE

#include "S11_State.h"

typedef struct{
	S11STATE;
}S11_UlCtx;

void linkUlCtx(S11_State* s);

#endif /* S11_ULCTX_HFILE */
