/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   NAS_EMM.h
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  NAS FSM header
 *
 * This module implements the NAS EMM interface state machine on the MME EndPoint.
 */

#ifndef NAS_EMM_H
#define NAS_EMM_H

#include "MME.h"
#include "S1AP.h"

#include <glib.h>

/**
 * @brief NAS EMM constructor
 * @param [in]  emm EMM stack handler
 * @return emm stack handler
 *
 *  Allocates the EMM stack. Use emm_free to delete the structure.
 */
gpointer emm_init(gpointer ecm);

/**
 * @brief Dealocates the EMM stack handler
 * @param [in]  emm_h EMM stack handler to be removed.
 */
void emm_free(gpointer emm_h);

/**
 * @brief NAS processing function
 * @param [in]  emm_h EMM Stack handler
 * @param [in]  msg pointer to the message to be processed
 * @param [in]  len message lenght
 *
 *  Function to process the EMM message. Used by the lower layer EMM.
 */
void emm_processMsg(gpointer emm_h, gpointer msg, gsize len);

/**
 * @brief get KeNB
 * @param [in]   emm_h EMM Stack handler
 * @param [out]  kasme    derived key - 256 bits
 */
void emm_getKeNB(const EMMCtx emm, uint8_t *keNB);

void emm_getUESecurityCapabilities(const EMMCtx emm, UESecurityCapabilities_t *cap);

void emm_getUEAMBR(const EMMCtx emm, UEAggregateMaximumBitrate_t *ambr);



#endif /* NAS_EMM_H */
