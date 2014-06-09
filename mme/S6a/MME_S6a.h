/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   MME_S6a.h
 * @Author Vicent Ferrer
 * @date   June, 2013
 * @brief  MME S6a interface protocol state machine.
 *
 * This module implements the S6a interface state machine.
 * It is currently only and emulator
 */

#ifndef MME_S6a_HFILE
#define MME_S6a_HFILE

#include "MME.h"
#include "S6a.h"

void s6a_GetAuthVector(struct t_engine_data *engine, struct SessionStruct_t *session);

void s6a_UpdateLocation(struct t_engine_data *engine, struct SessionStruct_t *session);

void refresh_NH(SecurityCtx_t* sec);

#endif /* MME_S6a_HFILE */
