/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   ECMSession.c
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  
 *
 * 
 */

#include <glib.h>

#include "ECMSession.h"
#include "ECMSession_priv.h"
#include "S1AP.h"
#include "logmgr.h"

#include "hmac_sha2.h"


/* API to S1AP */
ECMSession ecmSession_init(S1Assoc s1, guint16 r_sid){
	ECMSession_t *self = g_new0(ECMSession_t, 1);
	self->r_sid = r_sid;
	return self;
}

void ecmSession_free(ECMSession h){
	ECMSession_t *self = (ECMSession_t *)h;
	g_free(self);
}

/* API to NAS */
void ecm_send(ECMSession h, gpointer msg, size_t len){
	
}
