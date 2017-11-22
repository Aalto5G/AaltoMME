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
 * @file   NAS_EMM_priv.h
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  EMM logic
 *
 */

#ifndef NAS_EMM_PRIV_HFILE
#define NAS_EMM_PRIV_HFILE

#include <glib.h>
#include "NAS_EMM.h"
#include "EMM_Timers.h"

/* API to NAS */

gpointer emm_getS11(EMMCtx emm_h);

void emm_processS6aError(EMMCtx emm_h, GError *err);

void processAttach(gpointer emm_h,  GenericNASMsg_t* msg);

int emm_selectAttachType(EMMCtx_t * emm);

void emm_sendAttachReject(EMMCtx emm_h, guint cause,
                          gpointer esm_msg, gsize msgLen);

void emm_sendServiceReject(EMMCtx emm_h, EMMCause_t eMMcause);

/**@brief Send the Authentication Request message
 * @param [in] em_h     EMM handler
 * @param [in] msg      Message buffer pointer
 * @param [in] len      Lenght of the message buffer
 *
 * This function sends the downlinkNASTransport S1AP message
 * to forward a NAS message
 * */
void emm_sendAuthRequest(EMMCtx emm_h);

void emm_sendSecurityModeCommand(EMMCtx emm_h);

void emm_selectGateways(EMMCtx emm_h);

void emm_processFirstESMmsg(EMMCtx emm_h);

void emm_attachAccept(EMMCtx emm_h, gpointer esm_msg, gsize len, GList *bearers);

void emm_sendESM(const EMMCtx emm, const gpointer msg, const gsize len, GError **e);

void emm_internalSendESM(const EMMCtx emm, const gpointer msg, const gsize len, GError **e);

void emm_sendUEContextReleaseCommand(EMMCtx emm, cause_choice_t choice, uint32_t cause);

void processDetachReq(EMMCtx_t *emm, GenericNASMsg_t *msg);

void emm_processTAUReq(EMMCtx emm_h, GenericNASMsg_t *msg);

void emm_sendTAUAccept(EMMCtx emm_h);

void emm_sendTAUReject(EMMCtx emm_h, EMMCause_t cause);

void emm_sendIdentityReq(EMMCtx emm_h);

void emm_setSecurityQuadruplet(EMMCtx emm_h);

void emm_detachAccept(gpointer emm_h);

#endif /* NAS_EMM_PRIV_HFILE */
