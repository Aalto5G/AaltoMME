/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
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
 * @param [in]  s Subscription handler to be removed.
 */
void subs_cpyQoS_GTP(Subscription s, struct qos_t *qos);


const uint64_t subs_getMSISDN(Subscription s);
	
const uint64_t subs_getIMEISV(Subscription s);

const size_t subs_getAPNlen(Subscription s);

const guint8* subs_getEncodedAPN(const Subscription s, gpointer buffer, gsize maxLen, gsize *len);

const char* subs_getAPN(Subscription s);

PDNCtx subs_newPDNCtx(Subscription s);

void subs_setUEAMBR(Subscription s, guint64 ue_ambr_ul, guint64 ue_ambr_dl);

void pdnCtx_setDefaultBearerQoS(PDNCtx pdn, struct qos_t *qos);

#endif /* SUBSCRIPTION_H*/
