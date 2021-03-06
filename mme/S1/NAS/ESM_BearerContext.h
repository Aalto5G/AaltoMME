/* AaltoMME - Mobility Management Entity for LTE networks
 * Copyright (C) 2013 Vicent Ferrer Guash & Jesus Llorente Santos
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

typedef gpointer ESM_BearerContext;

/**
 * @brief NAS ESM constructor
 * @param [in]  emm EMM stack handler
 * @return esm stack handler
 *
 *  Allocates the ESM stack. Use esm_free to delete the structure.
 */
ESM_BearerContext esm_bc_init(gpointer emm, uint8_t ebi);

/**
 * @brief Dealocates the ESM stack handler
 * @param [in]  esm_h ESM stack handler to be removed.
 */
void esm_bc_free(ESM_BearerContext esm_h);

/**
 * @brief sends the appropiate message to setup the bearer
 * @param [in]  esm_h ESM stack handler
 */
void esm_activateDefault(ESM_BearerContext esm_h);

/* API to config*/

void esm_bc_setState(ESM_BearerContext self, ESM_State *s);

void esm_bc_processMsg(ESM_BearerContext self, const ESM_Message_t * msg);

/* API to ESM*/
guint8 *esm_bc_getEBIp(ESM_BearerContext bc_h);

const guint8 esm_bc_getEBI(const ESM_BearerContext bc_h);

void esm_bc_setS1uSGWfteid(ESM_BearerContext bc_h, gpointer fteid_h, gsize len);

void esm_bc_getS1uSGWfteid(const ESM_BearerContext bc_h, gpointer fteid_h, gsize *len);

void esm_bc_setS1ueNBfteid(ESM_BearerContext bc_h, gpointer fteid_h);

void esm_bc_setS5S8uPGWfteid(ESM_BearerContext bc_h, gpointer fteid_h, gsize len);

void esm_bc_getS1ueNBfteid(const ESM_BearerContext bc_h, gpointer fteid_h, gsize *len);

#endif /* ESM_Bearer_Context_H */
