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
 * @brief  Subscription Information
 *
 * This module implements the Subscription Information retrived from the HSS
 */

#include <string.h>
#include <glib.h>
#include "Subscription.h"
#include "logmgr.h"

#include "gtp.h"

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
    guint32  ctx_id;
    GString  *apn;
    gboolean pgw_allocation_type;
    gboolean vplmn_dynamic_address_allowed;

    guint8   pdn_addr_type;
    guint8   pdn_addr[20];
    guint32  subscribed_apn_ambr_dl;
    guint32  subscribed_apn_ambr_up;
    guint16  charging_characteristics;
    QoS      qos;
}PDNCtx_t;

typedef struct{
    guint64    msisdn;
    guint64    imeisv;
    guint8     network_access_mode;
    guint32    access_restriction_data;
    guint64    ambr_ul;
    guint64    ambr_dl;
    GString    *apn_io_replacement;
    //GHashTable *pdnCtx;
    PDNCtx_t   *pdn;
}Subs_t;

PDNCtx_t *pdnctx_init(){
    PDNCtx_t *self = g_new0(PDNCtx_t, 1);
    self->apn = g_string_new (NULL);
    return self;
}

static void pdnctx_free(PDNCtx_t *pdn){
    PDNCtx_t *self = (PDNCtx_t*)pdn;
    g_string_free(self->apn, TRUE);
    g_free(self);
}


Subscription subs_init(){
    Subs_t *self = g_new0(Subs_t, 1);
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
    /*  // do something with key and value */
    /*  pdn = (PDNCtx*)value; */
    /* } */
    pdn = self->pdn;

    qos->pvi    = pdn->qos.arp.preemption_vulnerability;
    qos->pl     = pdn->qos.arp.level;
    qos->pci    = pdn->qos.arp.preemption_vulnerability;
    qos->qci    = pdn->qos.qci;
    qos->mbr_ul = hton64(0);
    qos->mbr_dl = hton64(0);
    qos->gbr_ul = hton64(0);
    qos->gbr_dl = hton64(0);
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
    /*  // do something with key and value */
    /*  pdn = (PDNCtx*)value; */
    /* } */
    pdn = self->pdn;
    return pdn->apn->str;
}

guint8* subs_getEncodedAPN(const Subscription s, gpointer buffer, gsize maxLen, gsize *len){
	Subs_t *self = (Subs_t*)s;

	uint8_t i, label_len=0, *tmp, *res;
	GString *apn = self->pdn->apn;

    tmp = apn->str;
    memcpy(buffer+1, apn->str, apn->len);
    res = buffer+1;

    for(i=0; i< apn->len && i<maxLen; i++){
        if(res[i]=='.'){
            res[i-label_len-1] = label_len;
            label_len = 0;
        }else{
            label_len++;
        }
    }
    res[i-label_len-1] = label_len;
    *len = apn->len+1;
    return buffer;
}

const uint8_t subs_getPDNType(const Subscription s){
    Subs_t *self = (Subs_t*)s;
    PDNCtx_t *pdn;
    /* GHashTableIter iter; */
    /* gpointer key, value; */

    /* /\*Single PDN Context and single bearer (default)*\/ */
    /* g_hash_table_iter_init (&iter, hash_table); */
    /* while (g_hash_table_iter_next (&iter, &key, &value)){ */
    /*  // do something with key and value */
    /*  pdn = (PDNCtx*)value; */
    /* } */
    pdn = self->pdn;
    return pdn->pdn_addr_type;
}

void subs_getPDNAddr(const Subscription s, struct PAA_t *paa, gsize *len){
	Subs_t *self = (Subs_t*)s;
    PDNCtx_t *pdn;
    memset(paa, 0, sizeof(struct PAA_t));
    pdn = self->pdn;

	paa->type = pdn->pdn_addr_type;
	switch(pdn->pdn_addr_type){
	case PAA_IP4:
		*len = 5;
		memcpy(&(paa->addr.ipv4), pdn->pdn_addr, 4);
		break;
	case PAA_IP6:
		*len = 17;
		memcpy(paa->addr.ipv6, pdn->pdn_addr, 16);
		break;
	case PAA_IP46:
		*len = 21;
		memcpy(paa->addr.both.ipv6, pdn->pdn_addr, 16);
		memcpy(&(paa->addr.both.ipv4), pdn->pdn_addr + 16, 4);
        break;
	default:
		g_error("Wrong coding of PDN address, type %u", pdn->pdn_addr_type);
	}
}

void subs_setPDNaddr(Subscription s, const struct PAA_t *paa){
    Subs_t *self = (Subs_t*)s;
    PDNCtx_t *pdn;
    /* GHashTableIter iter; */
    /* gpointer key, value; */

    /* /\*Single PDN Context and single bearer (default)*\/ */
    /* g_hash_table_iter_init (&iter, hash_table); */
    /* while (g_hash_table_iter_next (&iter, &key, &value)){ */
    /*  // do something with key and value */
    /*  pdn = (PDNCtx*)value; */
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

PDNCtx subs_newPDNCtx(Subscription s){
	Subs_t *self = (Subs_t*)s;
	return self->pdn;
}

void subs_setUEAMBR(Subscription s, guint64 ue_ambr_ul, guint64 ue_ambr_dl){
	Subs_t *self = (Subs_t*)s;
	self->ambr_ul = ue_ambr_ul;
    self->ambr_dl = ue_ambr_dl;
}

void subs_getUEAMBR(const Subscription s, guint64 *ue_ambr_ul, guint64 *ue_ambr_dl){
	Subs_t *self = (Subs_t*)s;
	*ue_ambr_ul = self->ambr_ul;
    *ue_ambr_dl = self->ambr_dl;
}

void pdnCtx_setDefaultBearerQoS(PDNCtx _pdn, struct qos_t *qos){
	PDNCtx_t *pdn = (PDNCtx_t *)_pdn;
	pdn->qos.qci = qos->qci;
	pdn->qos.arp.level = qos->pl;
	pdn->qos.arp.preemption_capability = qos->pci;
	pdn->qos.arp.preemption_vulnerability = qos->pvi;
	pdn->subscribed_apn_ambr_dl = qos->mbr_ul;
	pdn->subscribed_apn_ambr_up = qos->mbr_dl;
}

void pdnCtx_setPDNtype(PDNCtx _pdn, guint8 t){
	PDNCtx_t *pdn = (PDNCtx_t *)_pdn;
	pdn->pdn_addr_type = t;
}

void pdnCtx_setAPN(PDNCtx _pdn, const char* apn){
	PDNCtx_t *pdn = (PDNCtx_t *)_pdn;
	g_string_assign(pdn->apn, apn);
}
