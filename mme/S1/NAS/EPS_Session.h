/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
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

void ePSsession_parsePDNConnectivityRequest(EPS_Session s, GenericNASMsg_t *msg);

void ePSsession_activateDefault(EPS_Session s);

void ePSsession_getPDNAddr(const EPS_Session s, TransportLayerAddress_t* addr);

ESM_BearerContext ePSsession_getDefaultBearer(EPS_Session s);

#endif /* EPS_SESSION_H*/
