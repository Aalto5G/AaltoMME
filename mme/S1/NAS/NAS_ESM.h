/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
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

/**
 * @brief NAS processing function
 * @param [in]  esm_h ESM Stack handler
 * @param [in]  msg pointer to the message to be processed
 *
 *  Function to process the ESM message. Used by the lower layer EMM.
 */

void esm_processMsg(gpointer esm_h, ESM_Message_t* msg);

void esm_setE_RABSetupuListCtxtSURes(ESM esm, E_RABSetupListCtxtSURes_t* l);

void esm_UEContextReleaseReq(ESM esm_h, void (*cb)(gpointer), gpointer args);

void esm_detach(ESM esm_h, void(*cb)(gpointer), gpointer args);

#endif /* NAS_ESM_H */
