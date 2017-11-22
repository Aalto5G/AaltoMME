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
 * @file   EMMCtx.h
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  EMMCtx Information
 *
 * This module implements the EMMCtx. This header is private.
 */

#ifndef EMM_CTX_H
#define EMM_CTX_H

#include "EMMCtx_iface.h"
#include "Subscription.h"
#include "glib.h"
#include "NAS_Definitions.h"
#include "EMM_State.h"
#include "EMM_FSMConfig.h"
#include "timermgr.h"

#define emm_log(self, p, en, ...) emm_log_(self, p, __FILE__, __func__, __LINE__, en, __VA_ARGS__)

typedef struct{
    guint8       rAND[16];
    guint8       xRES[8];
    guint8       ck[16];
    guint8       ik[16];
    guint8       aUTN[16];
}AuthQuintuplet;

typedef struct{
    guint8       rAND[16];
    guint8       xRES[8];
    guint8       aUTN[16];
    guint8       kASME[32];
}AuthQuadruplet;

typedef struct{
    gpointer     ecm;           /**< Lower layer */
    gpointer     esm;           /**< Higher layer */
    gpointer     s6a;
    EMM_State    *state;
    EMMState     stateName;

    TimerMgr     tm;
    Timer        *activeTimers;

    guint64      imsi;
    guint64      msisdn;
    guint64      imeisv;
    guti_t       guti;

    Subscription subs;

    struct sockaddr sgwIP;
    socklen_t    sgwIPLen;

    struct sockaddr pgwIP;
    socklen_t    pgwIPLen;

    gpointer     s11;

    /*Timers*/
    guint8       t3412;

    gboolean     attachStarted;
    gboolean     s1BearersActive;
    guint8       attachResult;
    guint8       updateResult;
    GPtrArray    *pendingESMmsg;
    /* Message processing helpers */
    guint8       msg_attachType;
    guint8       msg_detachType;
    guint        msg_updateType;
    gboolean     msg_activeFlag;
    guint8       msg_ksi;
    guti_t       msg_guti;
    gboolean     msg_additionalUpdateType;
    gboolean     msg_smsOnly;
    gboolean     msg_bearerCtxI;
    guint8       msg_bearerCtx[2];

    /* **** Start of MM context **** */
    guint8       sn[3];         /* Serving network (TBCD PLMN) */
    guint16      tac;           /* Tracking Area Code */
    guint8       type;          /**< MM type */
    guint8       securityMode;
    gboolean     nhi;           /**< Next Hop Indicator */
    gboolean     drxi;          /**< DRX Indicator */
    gboolean     uambri;        /**< Used AMBR Indicator */
    gboolean     sambri;        /**< Subscribed AMBR Indicator */
    gboolean     osci;          /**< Old Security Context indicator */
    guint8       ksi;
    guint8       nasIntAlg;
    guint8       nasCipAlg;
    NAS          parser;
    gboolean     sci;          /**< Security Context indicator */
    guint32      nasUlCountForSC;
    guint8       kasme[32];
    gsize        authQuadrsLen;
    GPtrArray    *authQuadrs;
    gsize        authQuintsLen;
    GPtrArray    *authQuints;

    guint8       drx[2];
    guint8       nh[32];
    guint8       ncc;           /**< Next Hop Chaining Count*/

    guint32      subs_ambr_ul;
    guint32      subs_ambr_dl;
    guint32      used_ambr_ul;
    guint32      used_ambr_dl;

    /* Network Capability */
    gsize        ueCapabilitiesLen;
    guint8       ueCapabilities[15];
    /* MS Network Capability */
    gsize        msNetCapLen;
    guint8       msNetCap[10];
    /* MEI*/

    gboolean     hnna;          /**< HO-To-Non-3GPPAccess Not Allowed */
    gboolean     ena;           /**< E-UTRAN Not Allowed */
    gboolean     ina;           /**< I-HSPA-Evolution Not Allowed */
    gboolean     gana;          /**< GAN Not Allowed */
    gboolean     gena;          /**< GERAN Not Allowed */
    gboolean     una;           /**< UTRAN Not Allowed */

    guint8       old_ksi;
    guint8       old_ncc;       /**< Next Hop Chaining Count*/
    guint8       old_kasme[32];
    guint8       old_nh[32];

    /*Voice Domain Preference and UE's Usage Setting*/
}EMMCtx_t;

void emm_log_(EMMCtx s, int pri, char *fn, const char *func, int ln,
              int en, char *fmt, ...);

EMMCtx emmCtx_init();

void emmCtx_free(EMMCtx s);

void emm_setState(EMMCtx emm_h, EMM_State *s, EMMState stateName);

void emmCtx_setNewAuthQuadruplet(EMMCtx emm, AuthQuadruplet *a);

const AuthQuadruplet *emmCtx_getFirstAuthQuadruplet(EMMCtx emm);

void emmCtx_removeFirstAuthQuadruplet(EMMCtx emm);

void emmCtx_freeAuthQuadruplets(EMMCtx emm);

const guint8 *emmCtx_getServingNetwork_TBCD(const EMMCtx emm);

void emmCtx_setMSISDN(EMMCtx emm, guint64 msisdn);

Subscription emmCtx_getSubscription(const EMMCtx emm);

void emmCtx_newGUTI(EMMCtx emm, guti_t *guti);

guint32 *emmCtx_getM_TMSI_p(const EMMCtx emm);

void emmCtx_getSGW(const EMMCtx emm, struct sockaddr *rAddr, socklen_t *rAddrLen);

void emmCtx_getPGW(const EMMCtx emm, struct sockaddr *rAddr, socklen_t *rAddrLen);


#endif /* EMM_CTX_H*/
