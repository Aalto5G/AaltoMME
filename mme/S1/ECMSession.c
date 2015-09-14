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

#include "S1Assoc_priv.h"
#include "ECMSession.h"
#include "ECMSession_priv.h"
#include "ECMSession_FSMConfig.h"
#include "S1AP.h"
#include "logmgr.h"
#include "NAS_EMM.h"
#include "MME_S1_priv.h"
#include "EPS_Session.h"
#include "ESM_BearerContext.h"

#include "hmac_sha2.h"
#include <string.h>


/* API to S1AP */
ECMSession ecmSession_init(S1Assoc s1, guint32 l_id){
    ECMSession_t *self = g_new0(ECMSession_t, 1);
    self->assoc = s1;
    self->mmeUEId = l_id;
    self->r_sid_valid = FALSE;

    /* Initial state for ECM FSM*/
    ecm_ChangeState(self, ECM_Idle);

    /* Configure high layer*/
    self->emm = emm_init(self);

    s1Assoc_registerECMSession(s1, self);

    return self;
}

void ecmSession_free(ECMSession h){
    ECMSession_t *self = (ECMSession_t *)h;
    emm_deregister(self->emm);
    g_free(self);
}

void ecmSession_setEMM(ECMSession h, gpointer emm){
	ECMSession_t *self = (ECMSession_t *)h;
	self->emm = emm;
}

void ecmSession_processMsg(ECMSession h, S1AP_Message_t *s1msg, int r_sid){
    ECMSession_t *self = (ECMSession_t *)h;
    self->state->processMsg(self, s1msg, r_sid);
}

void ecmSession_setState(ECMSession ecm, ECMSession_State *s){
    ECMSession_t *self = (ECMSession_t *)ecm;
    self->state = s;
}

const guint32 ecmSession_getMMEUEID(const ECMSession h){
    ECMSession_t *self = (ECMSession_t *)h;
    return self->mmeUEId;
}

guint32 *ecmSession_getMMEUEID_p(const ECMSession h){
    ECMSession_t *self = (ECMSession_t *)h;
    return &(self->mmeUEId);
}

/* API to NAS */
void ecm_send(ECMSession h, gpointer msg, size_t len){
    S1AP_Message_t *s1out;

    MME_UE_S1AP_ID_t *mmeUEId;
    ENB_UE_S1AP_ID_t *eNBUEId;
    Unconstrained_Octed_String_t *nAS_PDU;

    ECMSession_t *self = (ECMSession_t *)h;

    s1out = S1AP_newMsg();
    s1out->choice = initiating_message;
    s1out->pdu->procedureCode = id_downlinkNASTransport;
    s1out->pdu->criticality = ignore;

    /* MME-UE-S1AP-ID*/
    mmeUEId = s1ap_newIE(s1out, id_MME_UE_S1AP_ID, mandatory, reject);
    mmeUEId->mme_id = self->mmeUEId;

    /* eNB-UE-S1AP-ID*/
    eNBUEId = s1ap_newIE(s1out, id_eNB_UE_S1AP_ID, mandatory, reject);
    eNBUEId->eNB_id = self->eNBUEId;

    /* NAS-PDU*/
    nAS_PDU = s1ap_newIE(s1out, id_NAS_PDU, mandatory, reject);
    nAS_PDU->len = len;
    nAS_PDU->str = msg;

    /*s1out->showmsg(s1out);*/
    s1Assoc_send(self->assoc, self->l_sid, s1out);
    s1out->freemsg(s1out);
}

void ecm_sendCtxtSUReq(ECMSession h, gpointer msg, size_t len, GList *bearers){
	S1AP_Message_t *s1out;

    MME_UE_S1AP_ID_t *mmeUEId;
    ENB_UE_S1AP_ID_t *eNBUEId;
    Unconstrained_Octed_String_t *nAS_PDU;
    SecurityKey_t *key;
    UESecurityCapabilities_t * sec;
    UEAggregateMaximumBitrate_t *ambr;
    E_RABToBeSetupListCtxtSUReq_t *list;
    E_RABToBeSetupItemCtxtSUReq_t *eRABitem;
    GList *first;
    ESM_BearerContext bearer;
    EPS_Session session;
    ECMSession_t *self = (ECMSession_t *)h;
    guint32 teid;
    struct fteid_t fteid;
    gsize fteid_size=0;

    s1out = S1AP_newMsg();
    s1out->choice = initiating_message;
    s1out->pdu->procedureCode = id_InitialContextSetup;
    s1out->pdu->criticality = reject;

    /* MME-UE-S1AP-ID*/
    mmeUEId = s1ap_newIE(s1out, id_MME_UE_S1AP_ID, mandatory, reject);
    mmeUEId->mme_id = self->mmeUEId;

    /* eNB-UE-S1AP-ID*/
    eNBUEId = s1ap_newIE(s1out, id_eNB_UE_S1AP_ID, mandatory, reject);
    eNBUEId->eNB_id = self->eNBUEId;

    /* UE - AMBR*/
    ambr = s1ap_newIE(s1out, id_uEaggregateMaximumBitrate, mandatory, reject);
    emm_getUEAMBR(self->emm, ambr);

    
    /* E-RABToBeSetupListCtxtSUReq*/
    /* NAS-PDU*/
    list = s1ap_newIE(s1out, id_E_RABToBeSetupListCtxtSUReq, mandatory, reject);
    /* g_list_foreach (bearers, ecm_buildE_RABToBeSetupItemCtxtSUReq, (gpointer){self, list}); */

    eRABitem = list->newItem(list);

    eRABitem->eRABlevelQoSParameters = new_E_RABLevelQoSParameters();
    eRABitem->transportLayerAddress = new_TransportLayerAddress();
    eRABitem->eRABlevelQoSParameters->allocationRetentionPriority = new_AllocationAndRetentionPriority();

    /* NAS PDU is optional */
    if(msg && len>0){
	    eRABitem->opt |=0x80;
	    eRABitem->nAS_PDU = new_Unconstrained_Octed_String();
	    eRABitem->nAS_PDU->str = msg;
	    eRABitem->nAS_PDU->len = len;
    }

    first = g_list_first(bearers);
    session = (EPS_Session)first->data;
    bearer = ePSsession_getDefaultBearer(session);
    
    
    /* /\*eRABitem->eRABlevelQoSParameters->gbrQosInformation = new_GBR_QosInformation();*\/ */

    eRABitem->eRAB_ID.id = esm_bc_getEBI(bearer);
    
    eRABitem->eRABlevelQoSParameters->qCI = 9;
    eRABitem->eRABlevelQoSParameters->allocationRetentionPriority->priorityLevel= 15;
    eRABitem->eRABlevelQoSParameters->allocationRetentionPriority->pre_emptionCapability = 0;
    eRABitem->eRABlevelQoSParameters->allocationRetentionPriority->pre_emptionVulnerability = 0;

    /*eRABitem->eRABlevelQoSParameters->gbrQosInformation;*/

    esm_bc_getS1uSGWfteid(bearer, &fteid, &fteid_size);
    memcpy(eRABitem->transportLayerAddress->addr, (uint8_t*)&(fteid.addr), fteid_size-5);
    eRABitem->transportLayerAddress->len = (fteid_size - 5)*8;
    memcpy(eRABitem->gTP_TEID.teid, &(fteid.teid), sizeof(uint32_t));


    /*UE Security Capabilities*/
    sec = s1ap_newIE(s1out, id_UESecurityCapabilities, mandatory, reject);
    emm_getUESecurityCapabilities(self->emm, sec);

    /*Security Key*/
    key = s1ap_newIE(s1out, id_SecurityKey, mandatory, reject);
    emm_getKeNB(self->emm, key->key);


    /*s1out->showmsg(s1out);*/
    s1Assoc_send(self->assoc, self->l_sid, s1out);
    s1out->freemsg(s1out);
}

const guint8 *ecmSession_getServingNetwork_TBCD(const ECMSession h){
    ECMSession_t *self = (ECMSession_t *)h;
    return self->tAI.sn;
}

gpointer ecmSession_getS6a(const ECMSession h){
    ECMSession_t *self = (ECMSession_t *)h;
    struct mme_t *mme;
    S1 s1 = s1Assoc_getS1(self->assoc);
    mme = s1_getMME(s1);
    return  mme_getS6a(mme);
}

gpointer ecmSession_getS11(const ECMSession h){
    ECMSession_t *self = (ECMSession_t *)h;
    struct mme_t *mme;
    S1 s1 = s1Assoc_getS1(self->assoc);
    mme = s1_getMME(s1);
    return  mme_getS11(mme);
}

void ecmSession_getTAIlist(const ECMSession h, NAS_tai_list_t *tAIl, gsize *len){
	ECMSession_t *self = (ECMSession_t *)h;

	memset(tAIl, 0, sizeof(NAS_tai_list_t));
	tAIl->numOfElem = 0; /* 1 - 1*/
	tAIl->type = 0;
	tAIl->list.singlePLMNnonconsec.plmn
		= hton24(self->tAI.sn[0] <<16
		         | self->tAI.sn[1] << 8
		         | self->tAI.sn[2]);

	tAIl->list.singlePLMNnonconsec.tAC[0] = htons(self->tAI.tAC);
	*len=6;
}

void ecmSession_getGUMMEI(const ECMSession h, guint32* sn, guint16 *mmegi, guint8 *mmec){
	ECMSession_t *self = (ECMSession_t *)h;
	struct mme_t *mme = s1_getMME(s1Assoc_getS1(self->assoc));

	ServedGUMMEIsItem_t * item;
	const ServedGUMMEIs_t *gummeis = mme_getServedGUMMEIs(mme);
	item = gummeis->item[0];

	*sn = hton24(self->tAI.sn[0] <<16 | self->tAI.sn[1] << 8 | self->tAI.sn[2]);

	memcpy(mmegi, item->servedGroupIDs->item[0]->s, 2);
	*mmec = item->servedMMECs->item[0]->s[0];
}

void ecmSession_newGUTI(ECMSession h, guti_t *guti){
	ECMSession_t *self = (ECMSession_t *)h;
	guint32 sn, r;
	guint16 mmegi;
	guint8 mmec;
	guint64 n;
	struct mme_t *mme = s1_getMME(s1Assoc_getS1(self->assoc));

	ecmSession_getGUMMEI(self, &sn, &mmegi, &mmec);
	guti->tbcd_plmn = sn;
	guti->mmegi = mmegi;
	guti->mmec = mmec;

	/* M-TMSI IMSI hash salted with random number*/
	srand(time(NULL));
	r = rand();
	n =  emmCtx_getIMSI(self->emm) ^ ((guint64)r & ((guint64)r)<<32);
	guti->mtmsi = g_int64_hash(&n);

	mme_registerEMMSession(mme, self->emm);
}

void ecm_sendUEContextReleaseCommand(const ECMSession h, cause_choice_t choice, uint32_t cause){
	ECMSession_t *self = (ECMSession_t *)h;
	self->state->release(self, choice, cause);
}


const guint32 *ecmSession_getM_TMSI_p(const ECMSession h){
	ECMSession_t *self = (ECMSession_t *)h;
	const guti_t *guti = emmCtx_getGUTI(self->emm);
	return &(guti->mtmsi);
}
