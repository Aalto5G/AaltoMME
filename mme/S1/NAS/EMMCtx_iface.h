/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   EMMCtx_iface.h
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  EMMCtx Information Interface
 *
 * This module implements the EMMCtx
 */

#ifndef EMM_CTX_IFACE_H
#define EMM_CTX_IFACE_H

#include <glib.h>
#include "NAS_Definitions.h"

typedef void* EMMCtx;

const guint64 emmCtx_getIMSI(const EMMCtx emm);

const guint64 emmCtx_getMSISDN(const EMMCtx emm);

const guti_t *emmCtx_getGUTI(const EMMCtx emm);

#endif /* EMM_CTX_IFACE_H*/
