/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
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

gpointer s11u_newUser(gpointer s11, struct user_ctx_t *user);

void s11u_freeUser(gpointer self);


void processMsg(gpointer self, const struct t_message *msg);

void attach(gpointer self, void(*cb)(gpointer), gpointer args);

void detach(gpointer self, void(*cb)(gpointer), gpointer args);

void modBearer(gpointer self, void(*cb)(gpointer), gpointer args);


gboolean s11u_hasPendingResp(gpointer self);

int *s11u_getTEIDp(gpointer self);


/* API to config*/

void s11u_setState(gpointer self, S11_State *s);


/*API to the States*/

void returnControl(gpointer u);

void returnControlAndRemoveSession(gpointer u);

void parseIEs(gpointer u);

const int getMsgType(const gpointer u);


void sendCreateSessionReq(gpointer u);

void sendModifyBearerReq(gpointer u);

void sendDeleteSessionReq(gpointer u);


const gboolean accepted(gpointer u);

const int cause(gpointer u);

void parseCtxRsp(gpointer u, GError **err);

void parseModBearerRsp(gpointer u, GError **err);

void parseDelCtxRsp(gpointer u, GError **err);

void s11u_setS11fteid(gpointer u, gpointer fteid_h);

#endif /* S11_USER_HFILE */
