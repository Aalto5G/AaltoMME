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
 * @file   ECMSession.h
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  ECM logic
 *
 * This Module implements the ECM session logic, the user associated messages
 * of S1AP
 */

#ifndef ECMSession_HFILE
#define ECMSession_HFILE

#include <glib.h>

#include "MME.h"
#include "S1Assoc.h"
#include "S1AP.h"

typedef gpointer ECMSession;

/* API to S1AP */
ECMSession ecmSession_init(S1Assoc s1, S1AP_Message_t *s1msg, int r_sid);

void ecmSession_free(ECMSession h);

void ecmSession_reset(ECMSession h);

S1Assoc ecmSession_getS1Assoc(ECMSession h);

void ecmSession_pathSwitchReq(ECMSession h, S1Assoc newAssoc,
                           S1AP_Message_t *s1msg, int r_sid);

void ecmSession_processMsg(ECMSession h, S1AP_Message_t *s1msg, int r_sid);

const guint32 ecmSession_getMMEUEID(const ECMSession h);

guint32 *ecmSession_getMMEUEID_p(const ECMSession h);

guint32 *ecmSession_geteNBUEID_p(const ECMSession h);

const guint32 ecmSession_getLocalSid(const ECMSession h);


#endif /* ECMSession_HFILE */
