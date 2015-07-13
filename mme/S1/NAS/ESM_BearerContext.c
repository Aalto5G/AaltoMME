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

#include "ESM_BearerContext.h"
#include "NAS.h"
#include "logmgr.h"

typedef struct{
	gpointer esm;
	uint8_t ebi;
}ESM_BearerContext_t;

gpointer esm_bc_init(gpointer esm, uint8_t ebi){
	ESM_BearerContext_t *self = g_new0(ESM_BearerContext_t, 1);
	self->esm = esm;
	self->ebi = ebi;
	return self;
}

void esm_bc_free(gpointer bc_h){
	ESM_BearerContext_t *self = (ESM_BearerContext_t*)bc_h;
	g_free(self);
}

uint8_t *esm_bc_getEBIp(gpointer self){
	ESM_BearerContext_t *self = (ESM_BearerContext_t*)bc_h;
	return &(self->ebi)
}

void esm_bc_setState(gpointer self, ESM_State *s){
	
}
