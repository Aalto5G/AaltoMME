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
 * @file   NAS_ESM.h
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  NAS FSM header
 *
 * This module implements the NAS ESM interface state machine on the MME EndPoint.
 */

#ifndef NAS_ESM_H
#define NAS_ESM_H

#include "MME.h"
#include "ECMSession_priv.h"
#include "ESM_State.h"

#include <stdint.h>
#include <glib.h>

typedef gpointer ESM;

/**
 * @brief NAS ESM constructor
 * @param [in]  emm EMM stack handler
 * @return esm stack handler
 *
 *  Allocates the ESM stack. Use esm_free to delete the structure.
 */
gpointer esm_init(gpointer emm);

/**
 * @brief Dealocates the ESM stack handler
 * @param [in]  esm_h ESM stack handler to be removed.
 */
void esm_free(ESM esm_h);

void esm_errorEMM(gpointer esm_h);

/**
 * @brief NAS processing function
 * @param [in]  esm_h ESM Stack handler
 * @param [in]  msg pointer to the message to be processed
 *
 *  Function to process the ESM message. Used by the lower layer EMM.
 */
void esm_processMsg(gpointer esm_h, ESM_Message_t* msg);

void esm_modifyE_RABList(ESM esm_h,  E_RABsToBeModified_t* l,
                         void (*cb)(gpointer), gpointer args);

void esm_UEContextReleaseReq(ESM esm_h, void (*cb)(gpointer), gpointer args);

void esm_detach(ESM esm_h, void(*cb)(gpointer), gpointer args);

void esm_getSessions(ESM esm_h, GList **sessions);

void esm_getBearers(ESM esm_h, GList **bearers);


#endif /* NAS_ESM_H */
