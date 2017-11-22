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
 * @file   Subscription.h
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  Subscription Information
 *
 * This module implements the Subscription Information retrived from the HSS
 */

#ifndef SUBSCRIPTION_H
#define SUBSCRIPTION_H

#include <glib.h>
#include <stdlib.h>

#include "gtp.h"

typedef void* Subscription;
typedef void* PDNCtx;

/**
 * @brief Subscription Constructor
 * @param [in] Subscriber IMSI
 * @return empty subscription
 *
 *  Allocates the Subscription handler
 */
Subscription subs_init();

/**
 * @brief Dealocates the Subscription Handler
 * @param [in]  s Subscription handler to be removed.
 */
void subs_free(Subscription s);

/**
 * @brief
 * @param [in]  s
 */
void subs_cpyQoS_GTP(Subscription s, struct qos_t *qos);


const uint64_t subs_getMSISDN(Subscription s);

const uint64_t subs_getIMEISV(Subscription s);

const size_t subs_getAPNlen(Subscription s);

guint8* subs_getEncodedAPN(const Subscription s, gpointer buffer, gsize maxLen, gsize *len);

const uint8_t subs_getPDNType(const Subscription s);

void subs_getPDNAddr(const Subscription s, struct PAA_t *paa, gsize *len);

void subs_setPDNaddr(Subscription s, const struct PAA_t *paa);

const char* subs_getAPN(Subscription s);

PDNCtx subs_newPDNCtx(Subscription s);

void subs_setUEAMBR(Subscription s, guint64 ue_ambr_ul, guint64 ue_ambr_dl);

void subs_getUEAMBR(const Subscription s, guint64 *ue_ambr_ul, guint64 *ue_ambr_dl);

void pdnCtx_setDefaultBearerQoS(PDNCtx pdn, struct qos_t *qos);

void pdnCtx_setPDNtype(PDNCtx _pdn, guint8 t);

void pdnCtx_setAPN(PDNCtx _pdn, const char* apn);

#endif /* SUBSCRIPTION_H*/
