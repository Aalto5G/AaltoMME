/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   EPS_Session.c
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  NAS ESM implementation
 * @brief  EPS_Session Information
 *
 * This module implements the EPS_Session Information retrived from the HSS
 */

#include <string.h>
#include "EPS_Session_priv.h"
#include "NAS_ESM_priv.h"
#include "logmgr.h"
#include "MME_S11.h"

EPS_Session ePSsession_init(ESM esm, Subscription _subs, ESM_BearerContext b){
    EPS_Session_t *self = g_new0(EPS_Session_t, 1);
    self->esm = esm;
    self->subs = _subs;
    self->defaultBearer = b;
    self->bearers = g_hash_table_new_full (g_int_hash,
                                           g_int_equal,
                                           NULL,
                                           esm_bc_free);
    return self;
}

void ePSsession_free(EPS_Session s){
    EPS_Session_t *self = (EPS_Session_t*)s;
    if(self->APN){
        g_string_free(self->apn_io_replacement, TRUE);
    }
    if(self->subscribedAPN){
        g_string_free(self->apn_io_replacement, TRUE);
    }
    if(self->apn_io_replacement){
        g_string_free(self->apn_io_replacement, TRUE);
    }
    g_hash_table_destroy(self->bearers);
    g_free(self);
}

void ePSsession_parsePDNConnectivityRequest(EPS_Session s, GenericNASMsg_t *msg){
	uint8_t  *pointer, numOp=0;
	ie_tlv_t4_t *temp;
	EPS_Session_t *self = (EPS_Session_t*)s;
	PDNConnectivityRequest_t *pdnReq = (PDNConnectivityRequest_t*)&(msg->plain.eSM);

	/*Optionals*/
    if(pdnReq->optionals[numOp].iei&0xF0 == 0xD0){
        /*ESM information transfer flag*/
        numOp++;
    }
    if(pdnReq->optionals[numOp].iei == 0x28){
        /*Access point name*/
        numOp++;
    }
    if(pdnReq->optionals[numOp].iei == 0x27){
        /*Protocol configuration options*/
        temp =  & (pdnReq->optionals[numOp].tlv_t4);
        memcpy(self->pco, temp, temp->l+2);
        numOp++;
    }
    if(pdnReq->optionals[numOp].iei&0xF0 == 0x0C0){
        /*Device properties*/
        numOp++;
    }
}

void esm_test(EPS_Session s){
	EPS_Session_t *self = (EPS_Session_t*)s;

	guint8 buffer[150], *pointer, apn[150];
	gsize len;

	memset(buffer, 0, 150);
	pointer = buffer;

	/*Activate default*/
	newNASMsg_ESM(&pointer,
	              EPSSessionManagementMessages,
	              esm_bc_getEBI(self->defaultBearer));
    encaps_ESM(&pointer, 1 ,ActivateDefaultEPSBearerContextRequest);

    /* /\* EPS QoS *\/ */
    /* nasIe_lv_t4(&pointer, &(PDATA->user_ctx->ebearer->qos.qci), 1); */

    /* Access point name*/
    nasIe_lv_t4(&pointer,
                subs_getEncodedAPN(self->subs, apn, 150, &len),
                len);

    /* /\* PDN address *\/ */
    /* nasIe_lv_t4(&pointer, (uint8_t*)&(PDATA->user_ctx->pAA), len); */
    /* /\*Optionals *\/ */
    /* /\* Protocol Configuration Options*\/ */
    /* if(PDATA->user_ctx->pco[0]==0x27){ */
	/*     if(PROC->engine->mme->uE_DNS==0){ */
	/* 	    log_msg(LOG_DEBUG, 0, "Writting PCO IE. Lenght: %d, first byte %#x", */
	/* 	            PDATA->user_ctx->pco[1]+2, *(PDATA->user_ctx->pco+2)); */
	/* 	    nasIe_tlv_t4(&pointer, 0x27, PDATA->user_ctx->pco+2, PDATA->user_ctx->pco[1]); */
	/* 	    *(pointer-1-PDATA->user_ctx->pco[1]) = PDATA->user_ctx->pco[1]; */
	/*     }else{ */
	/* 	    pco[0]=0x80; */
	/* 	    pco[1]=0x80; */
	/* 	    pco[2]=0x21; */
	/* 	    pco[3]=0x0a; */
	/* 	    pco[4]=0x01; */
	/* 	    pco[5]=0x00; */
	/* 	    pco[6]=0x00; */
	/* 	    pco[7]=0x0a; */
	/* 	    pco[8]=0x81; */
	/* 	    pco[9]=0x06; */
	/* 	    memcpy(pco+10, &(PROC->engine->mme->uE_DNS), 4); */
	/* 	    nasIe_tlv_t4(&pointer, 0x27, pco, 14); */
	/* 	    *(pointer-15) = 14; */
	/*     } */
    /* } */


    emm_attachAccept(self->esm->emm, buffer, pointer-buffer);
}

void ePSsession_activateDefault(EPS_Session s){
	EPS_Session_t *self = (EPS_Session_t*)s;

	self->s11 = S11_newUserAttach(esm_getS11iface(self->esm), self->esm->emm, self,
	                              esm_test, self);
}

gboolean ePSsession_getPCO(const EPS_Session s, gpointer pco){
	EPS_Session_t *self = (EPS_Session_t*)s;

	if(self->pco[0]==0x27){
		memcpy(pco, self->pco, self->pco[1]);
		return TRUE;
    }
	return FALSE;
}

void ePSsession_setPCO(EPS_Session s, gconstpointer pco, gsize len){
	EPS_Session_t *self = (EPS_Session_t*)s;
	memcpy(self->pco, pco, len);
}

ESM_BearerContext ePSsession_getDefaultBearer(EPS_Session s){
	EPS_Session_t *self = (EPS_Session_t*)s;
	return self->defaultBearer;
}

void ePSsession_setPDNAddress(EPS_Session s, gpointer paa, gsize len){
	EPS_Session_t *self = (EPS_Session_t*)s;
	guint8 *p;
	p = (guint8*)paa;
	self->pdn_addr_type = p[0]&0x07;
	memcpy(self->pdn_addr, p+1, len-1);
}

const char* ePSsession_getPDNAddrStr(EPS_Session s, gpointer str, gsize maxlen){
	EPS_Session_t *self = (EPS_Session_t*)s;
	struct in_addr ipv4;
	struct in6_addr ipv6;

	switch(self->pdn_addr_type) {
	case 1: /*IPv4*/
		memcpy(&(ipv4.s_addr), self->pdn_addr, 4);
		return inet_ntop(AF_INET, &ipv4, str, maxlen);
	case 2: /*IPv6*/
		memcpy(&(ipv6.s6_addr), self->pdn_addr, 16);
		return inet_ntop(AF_INET6, &ipv6, str, maxlen);
	case 3: /* IPv4v6*/
		memcpy(&(ipv4.s_addr), self->pdn_addr, 4);
		return inet_ntop(AF_INET, &ipv4, str, maxlen);
	default:
		/* strncpy(s, "Unknown AF", maxlen); */
		return "Not valid";
    }
}
