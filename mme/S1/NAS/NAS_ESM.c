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

#include "NAS_ESM.h"
#include "NAS.h"
#include "logmgr.h"
#include "ESM_BearerContext.h"

//#include <stdint.h>

typedef struct{
	gpointer    emm;
	GHashTable* bearers;
	uint8_t     next_ebi;
}ESM_t;

gpointer esm_init(gpointer emm){
	ESM_t *self = g_new0(ESM_t, 1);
	self->emm = emm;
	self->next_ebi = 5;
	self->bearers =  g_hash_table_new_full( g_int_hash,
	                                        g_int_equal,
	                                        NULL,
	                                        (GDestroyNotify) esm_bc_free);
	return self;
}

void esm_free(gpointer esm_h){
	ESM_t *self = (ESM_t*)esm_h;
	g_free(self);
}

void esm_processMsg(gpointer esm_h, gpointer buffer, size_t len){
	ESM_t *self = (ESM_t*)esm_h;
	gpointer bearer;
	GenericNASMsg_t msg;
	dec_NAS(&msg, buffer, len);
	
	g_assert((ProtocolDiscriminator_t)msg.header.protocolDiscriminator.v == EPSSessionManagementMessages);

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
		log_msg(LOG_WARNING, 0, "ESM PDNConnectivityRequest Received !!");
		bearer = esm_bc_init(self->emm, self->next_ebi);
		self->next_ebi++;
		g_hash_table_insert(self->bearers, esm_bc_getEBIp(bearer), bearer);
		esm_activateDefault(bearer);
	case PDNDisconnectRequest:
	case BearerResourceAllocationRequest:
	case BearerResourceModificationRequest:
		break;
	/* Miscelaneous*/
	case ESMInformationResponse:
	case ESMStatus:
		break;
	defaul:
		break;
	}
}
