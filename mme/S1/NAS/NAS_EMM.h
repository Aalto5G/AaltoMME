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
#include "ECMSession_priv.h"
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


void emm_registerECM(EMMCtx emm_h, gpointer ecm);

void emm_deregister(EMMCtx emm_h);
/**
 * @brief NAS processing function
 * @param [in]  emm_h EMM Stack handler
 * @param [in]  msg pointer to the message to be processed
 * @param [in]  len message lenght
 *
 *  Function to process the EMM message. Used by the lower layer EMM.
 */
void emm_processMsg(gpointer emm_h, gpointer msg, gsize len);

void emm_getGUTIfromMsg(gpointer buffer, gsize len, guti_t* guti);

/**
 * @brief get KeNB
 * @param [in]   emm_h EMM Stack handler
 * @param [out]  kasme    derived key - 256 bits
 */
void emm_getKeNB(const EMMCtx emm, uint8_t *keNB);

/**
 * @brief get Next Hop
 * @param [in]   emm_h EMM Stack handler
 * @param [out]  nh    Next Hop derived key - 256 bits
 * @param [out]  ncc   Next Hop Chaining Count - 3 bits
 */
void emm_getNH(const EMMCtx emm, guint8 *nh, guint8 *ncc);

void emm_getUESecurityCapabilities(const EMMCtx emm, UESecurityCapabilities_t *cap);

void emm_getUEAMBR(const EMMCtx emm, UEAggregateMaximumBitrate_t *ambr);

void emm_modifyE_RABList(EMMCtx emm,  E_RABsToBeModified_t* l,
                         void (*cb)(gpointer), gpointer args);

/* void emm_setE_RABSetupuListCtxtSURes(EMMCtx emm, E_RABSetupListCtxtSURes_t* l); */

/* void emm_setE_RABToBeSwitchedDLList(EMMCtx emm, E_RABToBeSwitchedDLList_t* l); */

void emm_UEContextReleaseReq(EMMCtx emm, void (*cb)(gpointer), gpointer args);

guint32 *emm_getM_TMSI_p(EMMCtx emm);

void emm_triggerAKAprocedure(EMMCtx emm_h);

void emm_getEPSSessions(EMMCtx emm_h, GList **sessions);

void emm_getBearers(EMMCtx emm_h, GList **bearers);

const guint64 emm_getIMSI(const EMMCtx emm_h);

#endif /* NAS_EMM_H */
