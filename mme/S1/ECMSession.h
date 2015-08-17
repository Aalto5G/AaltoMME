/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
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
ECMSession  ecmSession_init(S1Assoc s1, guint16 r_sid);

void ecmSession_free(ECMSession h);

/* API to NAS */
void ecm_send(ECMSession h, gpointer msg, size_t len);

#endif /* ECMSession_HFILE */
