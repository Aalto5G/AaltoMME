/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   Subscription.c
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  NAS ESM implementation
 * @brief  Subscription Information
 *
 * This module implements the Subscription Information retrived from the HSS
 */

#include <string.h>
#include "Subscription.h"
#include "logmgr.h"

#include "glib.h"

/**
 * @brief Allocation and Retention Priority
 */
typedef struct{
    guint8   level;
    gboolean preemption_capability;
    gboolean preemption_vulnerability;
}ARP;

typedef struct{
    guint8   qci;
    ARP      arp;
}QoS;

typedef struct{
	guint32  id;
    GString  *apn;
    gboolean pgw_allocation_type;
    gboolean vplmn_dynamic_address_allowed;

    guint8   pdn_addr_type;
    guint8   pdn_addr[20];
    guint32  subscribed_apn_ambr_dl;
    guint32  subscribed_apn_ambr_up;
    QoS      qos;
}PDNCtx_t;

typedef struct{
    guint64    imsi;
    guint64    msisdn;
    guint64    imeisv;
    guint8     network_access_mode;
    guint32    ambr_ul;
    guint32    ambr_dl;
    GString    *apn_io_replacement;
    guint16    charging_characteristics;
    //GHashTable *pdnCtx;
    PDNCtx_t   *pdn;
}Subs_t;

PDNCtx_t *pdnctx_init(){
	PDNCtx_t *self = g_new0(PDNCtx_t, 1);
	return self;
}

static void pdnctx_free(PDNCtx_t *pdn){
	PDNCtx_t *self = (PDNCtx_t*)pdn;
	if(self->apn){
		g_string_free(self->apn, TRUE);		
	}
	g_free(self);
}


Subscription subs_init(uint64_t imsi){
	Subs_t *self = g_new0(Subs_t, 1);
	self->imsi = (guint64)imsi;
	/* self->pdnCtx = g_hash_table_new_full (g_int_hash, */
	/*                                       g_int_equal, */
	/*                                       NULL, */
	/*                                       pdnctx_free); */
	self->pdn = pdnctx_init();
	return self;
}

void subs_free(Subscription s){
	Subs_t *self = (Subs_t*)s;
	if(self->apn_io_replacement){
		g_string_free(self->apn_io_replacement, TRUE);		
	}
	/* g_hash_table_destroy(self->pdnCtx); */
	pdnctx_free(self->pdn);
	
	g_free(self);
}

void subs_cpyQoS_GTP(Subscription s, struct qos_t *qos){
	Subs_t *self = (Subs_t*)s;
	PDNCtx_t *pdn;
	/* GHashTableIter iter; */
	/* gpointer key, value; */

	/* /\*Single PDN Context and single bearer (default)*\/ */
	/* g_hash_table_iter_init (&iter, hash_table); */
	/* while (g_hash_table_iter_next (&iter, &key, &value)){ */
	/* 	// do something with key and value */
	/* 	pdn = (PDNCtx*)value; */
	/* } */
	pdn = self->pdn;

	qos->pvi    = pdn->qos.arp.preemption_vulnerability;
	qos->pl     = pdn->qos.arp.level;
	qos->pci    = pdn->qos.arp.preemption_vulnerability;
	qos->qci    = pdn->qos.qci;
	qos->mbr_ul = self->ambr_ul;
	qos->mbr_dl = self->ambr_dl;
	qos->gbr_ul = 0;
	qos->gbr_dl = 0;
}

const uint64_t subs_getIMSI(const Subscription s){
	Subs_t *self = (Subs_t*)s;
	return self->imsi;
}


const uint64_t subs_getMSISDN(const Subscription s){
	Subs_t *self = (Subs_t*)s;
	return self->msisdn;
}

const uint64_t subs_getIMEISV(const Subscription s){
	Subs_t *self = (Subs_t*)s;
	return self->imeisv;
}

const size_t subs_getAPNlen(const Subscription s){
	Subs_t *self = (Subs_t*)s;
	return self->pdn->apn->len;	
}

const char* subs_getAPN(const Subscription s){
	Subs_t *self = (Subs_t*)s;
	PDNCtx_t *pdn;
	/* GHashTableIter iter; */
	/* gpointer key, value; */

	/* /\*Single PDN Context and single bearer (default)*\/ */
	/* g_hash_table_iter_init (&iter, hash_table); */
	/* while (g_hash_table_iter_next (&iter, &key, &value)){ */
	/* 	// do something with key and value */
	/* 	pdn = (PDNCtx*)value; */
	/* } */
	pdn = self->pdn;
	pdn->apn->str;
}

const uint8_t subs_getPDNType(const Subscription s){
	Subs_t *self = (Subs_t*)s;
	PDNCtx_t *pdn;
	/* GHashTableIter iter; */
	/* gpointer key, value; */

	/* /\*Single PDN Context and single bearer (default)*\/ */
	/* g_hash_table_iter_init (&iter, hash_table); */
	/* while (g_hash_table_iter_next (&iter, &key, &value)){ */
	/* 	// do something with key and value */
	/* 	pdn = (PDNCtx*)value; */
	/* } */
	pdn = self->pdn;
	return pdn->pdn_addr_type;
}

void subs_setPDNaddr(Subscription s, const struct PAA_t *paa){
	Subs_t *self = (Subs_t*)s;
	PDNCtx_t *pdn;
	/* GHashTableIter iter; */
	/* gpointer key, value; */

	/* /\*Single PDN Context and single bearer (default)*\/ */
	/* g_hash_table_iter_init (&iter, hash_table); */
	/* while (g_hash_table_iter_next (&iter, &key, &value)){ */
	/* 	// do something with key and value */
	/* 	pdn = (PDNCtx*)value; */
	/* } */
	pdn = self->pdn;
	pdn->pdn_addr_type = paa->type;
	switch(paa->type){
	case PAA_IP4:
		memcpy(pdn->pdn_addr, &(paa->addr.ipv4), 4);
		break;
	case PAA_IP6:
		memcpy(pdn->pdn_addr, paa->addr.ipv6, 16);
		break;
	case PAA_IP46:
		memcpy(pdn->pdn_addr, paa->addr.both.ipv6, 16);
		memcpy(pdn->pdn_addr + 16, &(paa->addr.both.ipv4), 4);
		break;
	}
}
