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
#include "EPS_Session_priv.h"
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

static gboolean esm_errorEMMToSessions(gpointer unused,
                                   gpointer session,
                                   gpointer esm_h){
    ePSsession_errorESM(session);
    /*Return True to remove the EPS session*/
    return TRUE;
}

void esm_errorEMM(gpointer esm_h){
    ESM_t *self = (ESM_t*)esm_h;

    /* self->emm = NULL; */
    log_msg(LOG_ERR, 0, "EMM Error indication, deleting ESM");
    g_hash_table_foreach_remove(self->sessions, esm_errorEMMToSessions, self);
    /* Loyer layer will free this?*/
    /* esm_free(self); */
}

gpointer esm_getS11iface(ESM esm_h){
    ESM_t *self = (ESM_t*)esm_h;
    return self->s11_iface;
}


void esm_processMsg(gpointer esm_h, ESM_Message_t* msg){
    ESM_t *self = (ESM_t*)esm_h;
    gpointer bearer;
    EPS_Session s;
    gboolean infoTxRequired;
    guint8 *pointer, buf[100];
    GList *l;

    g_assert((ProtocolDiscriminator_t)msg->protocolDiscriminator.v
             == EPSSessionManagementMessages);

    switch(msg->messageType){
    /*Network Initiated*/
    case ActivateDefaultEPSBearerContextAccept:
    case ActivateDefaultEPSBearerContextReject:
    case ActivateDedicatedEPSBearerContextAccept:
    case ActivateDedicatedEPSBearerContextReject:
    case ModifyEPSBearerContextAccept:
    case ModifyEPSBearerContextReject:
    case DeactivateEPSBearerContextAccept:
        if (!g_hash_table_lookup_extended (self->bearers,
                                           &(msg->bearerIdendity),
                                           &bearer,
                                           NULL)){
            log_msg(LOG_WARNING, 0, "Received wrong EBI");
            return;
        }
        esm_bc_processMsg(bearer, msg);
        break;
    /* UE Requests*/
    case PDNConnectivityRequest:
        log_msg(LOG_DEBUG, 0, "Received PDNConnectivityRequest");
        bearer = esm_bc_init(self->emm, self->next_ebi);
        g_hash_table_insert(self->bearers, esm_bc_getEBIp(bearer), bearer);
        self->next_ebi++;

        s = ePSsession_init(self,
                            emmCtx_getSubscription(self->emm),
                            bearer);
        g_hash_table_add(self->sessions, s);
        ePSsession_parsePDNConnectivityRequest(s, msg, &infoTxRequired);
        ePSsession_activateDefault(s, infoTxRequired);
    case PDNDisconnectRequest:
    case BearerResourceAllocationRequest:
    case BearerResourceModificationRequest:
        break;
    /* Miscelaneous*/
    case ESMInformationResponse:
        log_msg(LOG_DEBUG, 0, "Received ESMInformationResponse");
        l = g_hash_table_get_values (self->sessions);
        ePSsession_activateDefault(l->data, FALSE);
        /* g_hash_table_foreach (GHashTable *hash_table, */
        /*                       GHFunc func, */
        /*                       gpointer user_data); */
        break;
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

void esm_modifyE_RABList(ESM esm_h,  E_RABsToBeModified_t* l,
                         void (*cb)(gpointer), gpointer args){
    GList *ls;
    ESM_t *self = (ESM_t*)esm_h;
    EPS_Session_t *session;
    ls = g_hash_table_get_values (self->sessions);
    if(ls){
        session = ls->data;
        ePSsession_modifyE_RABList(session, l, cb, args);
    }else if(cb){
        cb(args);
    }
}

void esm_UEContextReleaseReq(ESM esm_h, void (*cb)(gpointer), gpointer args){
    ESM_t *self = (ESM_t*)esm_h;
    GList *ls;
    EPS_Session_t *session;
    ls = g_hash_table_get_values (self->sessions);
    if(ls){
        session = ls->data;
        ePSsession_UEContextReleaseReq(session, cb, args);
    }else if(cb){
        cb(args);
    }
}

void esm_deleteEPSSession(EPS_Session s){
    EPS_Session_t *session = (EPS_Session_t*)s;
    ESM_t *esm = (ESM_t*)session->esm;
    ESM_BearerContext bearer = ePSsession_getDefaultBearer(session);
    g_hash_table_remove(esm->sessions, session);
    g_hash_table_remove(esm->bearers, esm_bc_getEBIp(bearer));

    esm->next_ebi = 5;
    if(esm->cb){
        esm->cb(esm->args);
    }
}

void esm_detach(ESM esm_h, void(*cb)(gpointer), gpointer args){
    ESM_t *self = (ESM_t*)esm_h;
    GList *ls;
    EPS_Session_t *session;
    ls = g_hash_table_get_values(self->sessions);
    self->cb = cb;
    self->args = args;
    if(ls){
        session = ls->data;
        ePSsession_detach(session, esm_deleteEPSSession, session);
    }else if(cb){
        cb(args);
    }
}

void esm_getSessions(ESM esm_h, GList **sessions){
    ESM_t *self = (ESM_t*)esm_h;
    *sessions = g_hash_table_get_values(self->sessions);
}

void esm_getBearers(ESM esm_h, GList **bearers){
    ESM_t *self = (ESM_t*)esm_h;
    *bearers = g_hash_table_get_values(self->bearers);
}
