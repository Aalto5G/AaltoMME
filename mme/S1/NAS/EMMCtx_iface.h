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

void emmCtx_getTAI(const EMMCtx emm, guint8 (*sn)[3], guint16 *tac);


#endif /* EMM_CTX_IFACE_H*/
