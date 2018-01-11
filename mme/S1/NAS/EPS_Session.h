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
 * @file   EPS_Session.h
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  EPS_Session Information
 *
 * EPS Session Interface
 */

#ifndef EPS_SESSION_H
#define EPS_SESSION_H

#include "NAS_ESM.h"
#include "Subscription.h"
#include "ESM_BearerContext.h"

typedef void* EPS_Session;

/**
 * @brief EPS_Session Constructor
 * @return empty session
 *
 *  Allocates the EPS_Session handler
 */

EPS_Session ePSsession_init(ESM esm, Subscription _subs, ESM_BearerContext b);

/**
 * @brief Dealocates the EPS_Session Handler
 * @param [in]  s EPS_Session handler to be removed.
 */
void ePSsession_free(EPS_Session s);

void ePSsession_parsePDNConnectivityRequest(EPS_Session s, ESM_Message_t *msg,
                                            gboolean *infoTxRequired);

void ePSsession_activateDefault(EPS_Session s, gboolean infoTxRequired);

void ePSsession_modifyE_RABList(EPS_Session s, E_RABsToBeModified_t* l,
                                void (*cb)(gpointer), gpointer args);

void ePSsession_getPDNAddr(const EPS_Session s, TransportLayerAddress_t* addr);

ESM_BearerContext ePSsession_getDefaultBearer(EPS_Session s);

void ePSsession_UEContextReleaseReq(EPS_Session s,
                                    void (*cb)(gpointer), gpointer args);

void ePSsession_detach(EPS_Session s, void(*cb)(gpointer), gpointer args);

void ePSsession_errorESM(EPS_Session s);


#endif /* EPS_SESSION_H*/
