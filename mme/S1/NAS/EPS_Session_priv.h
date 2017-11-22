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
 * This module implements the EPS_Session. This header is private
 */

#ifndef EPS_SESSION_PRIV_H
#define EPS_SESSION_PRIV_H

#include <glib.h>

#include "EPS_Session.h"
#include "Subscription.h"
#include "ESM_FSMConfig.h"
#include "ESM_BearerContext.h"
#include "NAS_ESM_priv.h"


typedef struct{
    ESM_BearerContext      defaultBearer;
    ESM_t                  *esm;
    gpointer               s11;

    Subscription           subs;  /**< Subscription information*/
    GString                *APN;
    /* apn_restriction; */
    GString                *subscribedAPN;
    guint8                 pdn_addr_type;
    guint8                 pdn_addr[20];
    guint16                charging_characteristics;
    GString                *apn_io_replacement;

    struct qos_t           qos;
    guint64                apn_ambr_dl;
    guint64                apn_ambr_ul;

    GHashTable             *bearers;

    guint8                 pco[0xff+2];   /* TLV Protocol Configuration Options*/
    guint32                current_pti;
}EPS_Session_t;

gboolean ePSsession_getPCO(EPS_Session s, gpointer pco, gsize *len);

void ePSsession_setPCO(EPS_Session s, gconstpointer pco, gsize len);

void ePSsession_setPDNAddress(EPS_Session s, gpointer paa, gsize len);

const char* ePSsession_getPDNAddrStr(EPS_Session s, gpointer str, gsize maxlen);

const guint8 ePSsession_getPDNType(EPS_Session s);




#endif /* EPS_SESSION_PRIV_H*/
