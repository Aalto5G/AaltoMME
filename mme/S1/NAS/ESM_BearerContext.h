/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   ESM_BearerContext.h
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  NAS FSM header
 *
 * This module implements the NAS ESM interface state machine on the MME EndPoint.
 */

#ifndef ESM_BEARER_CONTECT_H
#define ESM_BEARER_CONTECT_H

#include "MME.h"
#include "ESM_State.h"
#include "NAS.h"

#include <stdint.h>
#include <glib.h>

/**
 * @brief NAS ESM constructor
 * @param [in]  emm EMM stack handler
 * @return esm stack handler
 *
 *  Allocates the ESM stack. Use esm_free to delete the structure.
 */
gpointer esm_bc_init(gpointer emm, uint8_t ebi);

/**
 * @brief Dealocates the ESM stack handler
 * @param [in]  esm_h ESM stack handler to be removed.
 */
void esm_bc_free(gpointer esm_h);

/**
 * @brief sends the appropiate message to setup the bearer
 * @param [in]  esm_h ESM stack handler
 */
void esm_activateDefault(gpointer esm_h);

/* API to config*/

void esm_bc_setState(gpointer self, ESM_State *s);

void esm_bc_processMsg(gpointer self, const ESM_Message_t * msg);

/* API to ESM*/
uint8_t *esm_bc_getEBIp(gpointer bc_h);

#endif /* ESM_Bearer_Context_H */
