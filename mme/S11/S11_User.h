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
 * @file   S11_User.h
 * @Author Vicent Ferrer
 * @date   June, 2015
 * @brief  Interface to S11 State
 *
 */

#ifndef S11_USER_HFILE
#define S11_USER_HFILE

#include <glib.h>

#include "MME.h"
#include "Subscription.h"
#include "S11_State.h"
#include "EPS_Session.h"
#include "EMMCtx_iface.h"

gpointer s11u_newUser(gpointer s11, EMMCtx emm, EPS_Session s);

void s11u_freeUser(gpointer self);


void processMsg(gpointer self, const struct t_message *msg);

void attach(gpointer self, void(*cb)(gpointer), gpointer args);

void detach(gpointer self, void(*cb)(gpointer), gpointer args);

void modBearer(gpointer self, void(*cb)(gpointer), gpointer args);

void releaseAccess(gpointer session, void(*cb)(gpointer), gpointer args);


int *s11u_getTEIDp(gpointer self);


/* API to config*/

void s11u_setState(gpointer self, S11_State *s);


/*API to the States*/

void returnControl(gpointer u);

void returnControlAndRemoveSession(gpointer u);

void parseIEs(gpointer u);

const int getMsgType(const gpointer u);

void dl_data_not(gpointer u);

void sendCreateSessionReq(gpointer u);

void sendModifyBearerReq(gpointer u);

void sendDeleteSessionReq(gpointer u);

void sendReleaseAccessBearersReq(gpointer u);

void sendDownlinkDataNotificationAck(gpointer u);

const gboolean accepted(gpointer u);

const int cause(gpointer u);

void parseCtxRsp(gpointer u, GError **err);

void parseModBearerRsp(gpointer u, GError **err);

void parseDelCtxRsp(gpointer u, GError **err);

void s11u_setS11fteid(gpointer u, gpointer fteid_h);

#endif /* S11_USER_HFILE */
