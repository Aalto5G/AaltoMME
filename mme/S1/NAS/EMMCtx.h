/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
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
#include "timermgr.h"

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

    TimerMgr     tm;
    Timer        *activeTimers;

    guint64      imsi;
    guint64      msisdn;
    guint64      imeisv;
    guti_t       guti;

    Subscription subs;

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

EMMCtx emmCtx_init();

void emmCtx_free(EMMCtx s);

void emm_setState(gpointer emm_h, EMM_State *s);

const guint64 emmCtx_getIMSI(const EMMCtx emm);

const guint64 emmCtx_getMSISDN(const EMMCtx emm);

void emmCtx_setNewAuthQuadruplet(EMMCtx emm, AuthQuadruplet *a);

const AuthQuadruplet *emmCtx_getFirstAuthQuadruplet(EMMCtx emm);

void emmCtx_freeAuthQuadruplet(EMMCtx emm);

const guint8 *emmCtx_getServingNetwork_TBCD(const EMMCtx emm);

void emmCtx_setMSISDN(EMMCtx emm, guint64 msisdn);

Subscription emmCtx_getSubscription(const EMMCtx emm);

void emmCtx_newGUTI(EMMCtx emm, guti_t *guti);

const guti_t * emmCtx_getGUTI(const EMMCtx emm);

guint32 *emmCtx_getM_TMSI_p(const EMMCtx emm);

void emmCtx_replaceEMM(EMMCtx_t **emm, EMMCtx_t *old_emm);


#endif /* EMM_CTX_H*/
