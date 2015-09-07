/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   NAS_ESM.c
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  NAS ESM implementation
 *
 * This module implements the NAS ESM interface state machine on the MME EndPoint.
 */

#include "NAS_ESM_priv.h"
#include "NAS_EMM_priv.h"
#include "NAS.h"
#include "logmgr.h"
#include "ESM_BearerContext.h"
#include "EPS_Session.h"
#include "ECMSession_priv.h"
#include "MME_S1_priv.h"
#include "S1Assoc_priv.h"

#include <string.h>
//#include <stdint.h>

void parse_PDNConnectivityRequest(ESM_t *self, GenericNASMsg_t *msg);

gpointer esm_init(gpointer emm){
	ESM_t *self = g_new0(ESM_t, 1);
	self->emm = emm;
	self->next_ebi = 5;
	self->bearers =  g_hash_table_new_full(g_int_hash,
	                                       g_int_equal,
	                                       NULL,
	                                       NULL);
	self->sessions = g_hash_table_new_full(NULL,
	                                       NULL,
	                                       NULL,
	                                       (GDestroyNotify) ePSsession_free);
	self->s11_iface = emm_getS11(emm);
	return self;
}

void esm_free(gpointer esm_h){
	ESM_t *self = (ESM_t*)esm_h;
	g_hash_table_destroy(self->sessions);
	g_hash_table_destroy(self->bearers);
	g_free(self);
}

gpointer esm_getS11iface(ESM esm_h){
	ESM_t *self = (ESM_t*)esm_h;
	return self->s11_iface;
}


void esm_processMsg(gpointer esm_h, gpointer buffer, size_t len){
	ESM_t *self = (ESM_t*)esm_h;
	gpointer bearer;
	GenericNASMsg_t msg;
	EPS_Session s;
	dec_NAS(&msg, buffer, len);
	
	g_assert((ProtocolDiscriminator_t)msg.header.protocolDiscriminator.v
	         == EPSSessionManagementMessages);

	switch(msg.plain.eSM.messageType){
	/*Network Initiated*/
	case ActivateDefaultEPSBearerContextAccept:
	case ActivateDefaultEPSBearerContextReject:
	case ActivateDedicatedEPSBearerContextAccept:
	case ActivateDedicatedEPSBearerContextReject:
	case ModifyEPSBearerContextAccept:
	case ModifyEPSBearerContextReject:
	case DeactivateEPSBearerContextAccept:
		if (!g_hash_table_lookup_extended (self->bearers,
		                                   &(msg.plain.eSM.bearerIdendity),
		                                   &bearer,
		                                   NULL)){
			log_msg(LOG_WARNING, 0, "Received wrong EBI");
			return;
		}
		esm_bc_processMsg(bearer, &(msg.plain.eSM));
		break;
	/* UE Requests*/
	case PDNConnectivityRequest:
		log_msg(LOG_WARNING, 0, "Received PDNConnectivityRequest");

		bearer = esm_bc_init(self->emm, self->next_ebi);
		self->next_ebi++;
		g_hash_table_insert(self->bearers, esm_bc_getEBIp(bearer), bearer);

		s = ePSsession_init(self,
		                    emmCtx_getSubscription(self->emm),
		                    bearer);
		g_hash_table_add(self->sessions, s);

		ePSsession_parsePDNConnectivityRequest(s, &msg);
		ePSsession_activateDefault(s);

	case PDNDisconnectRequest:
	case BearerResourceAllocationRequest:
	case BearerResourceModificationRequest:
		break;
	/* Miscelaneous*/
	case ESMInformationResponse:
	case ESMStatus:
		break;
	default:
		break;
	}
}

uint32_t esm_getDNSsrv(ESM esm_h){
	ESM_t *self = (ESM_t*)esm_h;
	EMMCtx_t *emm = (EMMCtx_t *)self->emm;
	ECMSession_t *ecm = (ECMSession_t*)emm->ecm;
	struct mme_t *mme = s1_getMME(s1Assoc_getS1(ecm->assoc));
	return mme->uE_DNS;
}
