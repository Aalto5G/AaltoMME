/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   ECMSession_Connected.c
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief
 *
 */

#include "ECMSession_Connected.h"
#include "MME.h"
#include "logmgr.h"
#include "S1Assoc_priv.h"
#include "ECMSession_priv.h"
#include "ECMSession_FSMConfig.h"
#include "NAS_EMM.h"

static void ecm_UEContextRelease(gpointer ecm_h, S1AP_Message_t *s1msg);

static void processMsg(gpointer _ecm, S1AP_Message_t *s1msg, int r_sid){
    ECMSession_t *ecm = (ECMSession_t *)_ecm;
    MME_UE_S1AP_ID_t *mme_id;
    ENB_UE_S1AP_ID_t *eNB_ID;
    Unconstrained_Octed_String_t *nASPDU;
    E_RABSetupListCtxtSURes_t *list;
    Cause_t *c;
    struct mme_t * mme;

    if(r_sid != ecm->r_sid && ecm->r_sid_valid){
        log_msg(LOG_ERR, 0,
                "Received S1AP msg from wrong stream id, expected %u != %u",
                ecm->r_sid,
                r_sid);
        return;
    }else if(!ecm->r_sid_valid){
        ecm->r_sid = r_sid;
        ecm->r_sid_valid = TRUE;
    }

    /*Class 1 Procedures*/
    if(s1msg->pdu->procedureCode == id_HandoverPreparation &&
       s1msg->choice == initiating_message){
        log_msg(LOG_WARNING, 0, "Received id_HandoverPreparation");
    }else if(s1msg->pdu->procedureCode ==  id_HandoverCancel &&
             s1msg->choice == initiating_message){
        log_msg(LOG_WARNING, 0, "Received id_HandoverCancel");
    }else if(s1msg->pdu->procedureCode ==  id_UEContextRelease &&
             s1msg->choice == successful_outcome){
        log_msg(LOG_DEBUG, 0, "Received id_UEContextReleaseComplete");
        if(ecm->causeRelease){
            c = ecm->causeRelease;
            if(c->choice == CauseRadioNetwork &&
               c->cause.radioNetwork.cause.noext == CauseRadioNetwork_radio_connection_with_ue_lost){
                /* Deactivate GBR Dedicated Bearers*/
            }else if(c->choice == CauseRadioNetwork &&
                     c->cause.radioNetwork.cause.noext == CauseRadioNetwork_cs_fallback_triggered){
                /* Check TS 23.272*/
            }
            ecm->causeRelease->freeIE(ecm->causeRelease);
            ecm->causeRelease=NULL;
        }
        s1Assoc_deregisterECMSession(ecm->assoc, ecm);
        mme = s1_getMME(s1Assoc_getS1(ecm->assoc));
        mme_deregisterECM(mme, ecm);
    }else if(s1msg->pdu->procedureCode ==  id_InitialContextSetup &&
             s1msg->choice == successful_outcome){
        mme_id = (MME_UE_S1AP_ID_t*)s1ap_findIe(s1msg, id_MME_UE_S1AP_ID);
        eNB_ID = (ENB_UE_S1AP_ID_t*)s1ap_findIe(s1msg, id_eNB_UE_S1AP_ID);
        if (eNB_ID->eNB_id != ecm->eNBUEId || mme_id->mme_id != ecm->mmeUEId){
            log_msg(LOG_WARNING, 0, "Received InitialContextSetupResponse"
                    " with incorrect IDs");
            return;
        }
        log_msg(LOG_DEBUG, 0, "Received InitialContextSetupResponse");
        list = s1ap_findIe(s1msg, id_E_RABSetupListCtxtSURes);
        emm_modifyE_RABList(ecm->emm, list, NULL, NULL);
    }else if(s1msg->pdu->procedureCode ==  id_InitialContextSetup &&
             s1msg->choice == unsuccessful_outcome){
        log_msg(LOG_WARNING, 0, "Received InitialContextSetupFailure");

    /* Class 2 Procedures*/
    }else if(s1msg->pdu->procedureCode == id_uplinkNASTransport &&
             s1msg->choice == initiating_message){
        mme_id = (MME_UE_S1AP_ID_t*)s1ap_findIe(s1msg, id_MME_UE_S1AP_ID);
        eNB_ID = (ENB_UE_S1AP_ID_t*)s1ap_findIe(s1msg, id_eNB_UE_S1AP_ID);
        if (eNB_ID->eNB_id != ecm->eNBUEId || mme_id->mme_id != ecm->mmeUEId){
            log_msg(LOG_WARNING, 0, "Received id_uplinkNASTransport with incorrect IDs");
            return;
        }
        nASPDU = (Unconstrained_Octed_String_t*)s1ap_findIe(s1msg, id_NAS_PDU);
        emm_processMsg(ecm->emm, nASPDU->str, nASPDU->len);
    }else if(s1msg->pdu->procedureCode == id_HandoverNotification &&
             s1msg->choice == initiating_message){
        log_msg(LOG_WARNING, 0, "Received id_HandoverNotification");
    }else if(s1msg->pdu->procedureCode == id_NASNonDeliveryIndication &&
             s1msg->choice == initiating_message){
        log_msg(LOG_WARNING, 0, "Received id_NASNonDeliveryIndication");
    }else if(s1msg->pdu->procedureCode == id_UEContextReleaseRequest &&
             s1msg->choice == initiating_message){
        log_msg(LOG_INFO, 0, "Received id_UEContextReleaseRequest");
        ecm_UEContextRelease(ecm, s1msg);
    }else if(s1msg->pdu->procedureCode == id_UECapabilityInfoIndication &&
             s1msg->choice == initiating_message){
        log_msg(LOG_DEBUG, 0, "Received id_UECapabilityInfoIndication");
    }else if(s1msg->pdu->procedureCode == id_eNBStatusTransfer &&
             s1msg->choice == initiating_message){
        log_msg(LOG_WARNING, 0, "Received id_eNBStatusTransfer");
    }else if(s1msg->pdu->procedureCode == id_TraceFailureIndication &&
             s1msg->choice == initiating_message){
        log_msg(LOG_WARNING, 0, "Received id_TraceFailureIndication");
    }else if(s1msg->pdu->procedureCode == id_LocationReportingFailureIndication &&
             s1msg->choice == initiating_message){
        log_msg(LOG_WARNING, 0, "Received id_LocationReportingFailureIndication");
    }else if(s1msg->pdu->procedureCode == id_LocationReport &&
             s1msg->choice == initiating_message){
        log_msg(LOG_WARNING, 0, "Received id_LocationReport");
    }else if(s1msg->pdu->procedureCode == id_CellTrafficTrace &&
             s1msg->choice == initiating_message){
        log_msg(LOG_WARNING, 0, "Received id_CellTrafficTrace");
    }else{
        log_msg(LOG_ERR, 0, "Message not recognized in this context");
    }
}

static void release(gpointer _ecm, cause_choice_t choice, uint32_t cause){
    ECMSession_t *ecm = (ECMSession_t *)_ecm;

    S1AP_Message_t *s1out;

    MME_UE_S1AP_ID_t *mmeUEId;
    ENB_UE_S1AP_ID_t *eNBUEId;
    UE_S1AP_IDs_t *ue_ids;
    Cause_t *c;

    s1out = S1AP_newMsg();
    s1out->choice = initiating_message;
    s1out->pdu->procedureCode = id_UEContextRelease;
    s1out->pdu->criticality = reject;

    /* id-UE-S1AP-IDs */
    ue_ids = s1ap_newIE(s1out, id_UE_S1AP_IDs, mandatory, reject);
    ue_ids->choice = 0;
    ue_ids->uE_S1AP_ID.uE_S1AP_ID_pair = new_UE_S1AP_ID_pair();
    ue_ids->uE_S1AP_ID.uE_S1AP_ID_pair->eNB_UE_S1AP_ID->eNB_id = ecm->eNBUEId;
    ue_ids->uE_S1AP_ID.uE_S1AP_ID_pair->mME_UE_S1AP_ID->mme_id = ecm->mmeUEId;

    /* id-Cause */
    c = s1ap_newIE(s1out, id_Cause, mandatory, ignore);
    c->choice = choice;
    switch(choice){
    case CauseRadioNetwork:
        c->cause.radioNetwork.cause.noext = cause;
        break;
    case CauseTransport:
        break;
    case CauseNas:
        c->cause.nas.cause.noext = cause;
        break;
    case CauseProtocol:
        break;
    case CauseMisc:
        break;
    }
    /*s1out->showmsg(s1out);*/
    s1Assoc_send(ecm->assoc, ecm->l_sid, s1out);
    s1out->freemsg(s1out);
}

static void sendUEContextReleaseCommand(gpointer _ecm){
    ECMSession_t *ecm = (ECMSession_t *)_ecm;

    S1AP_Message_t *s1out;

    MME_UE_S1AP_ID_t *mmeUEId;
    ENB_UE_S1AP_ID_t *eNBUEId;
    UE_S1AP_IDs_t *ue_ids;

    s1out = S1AP_newMsg();
    s1out->choice = initiating_message;
    s1out->pdu->procedureCode = id_UEContextRelease;
    s1out->pdu->criticality = reject;

    /* id-UE-S1AP-IDs */
    ue_ids = s1ap_newIE(s1out, id_UE_S1AP_IDs, mandatory, reject);
    ue_ids->choice = 0;
    ue_ids->uE_S1AP_ID.uE_S1AP_ID_pair = new_UE_S1AP_ID_pair();
    ue_ids->uE_S1AP_ID.uE_S1AP_ID_pair->eNB_UE_S1AP_ID->eNB_id = ecm->eNBUEId;
    ue_ids->uE_S1AP_ID.uE_S1AP_ID_pair->mME_UE_S1AP_ID->mme_id = ecm->mmeUEId;

    /* id-Cause */
    s1ap_setValueOnNewIE(s1out, id_Cause, mandatory, ignore, (GenericVal_t *)(ecm->causeRelease));

    /*s1out->showmsg(s1out);*/
    s1Assoc_send(ecm->assoc, ecm->l_sid, s1out);
    s1out->freemsg(s1out);
}

void linkECMSessionConnected(ECMSession_State* s){
    s->processMsg = processMsg;
    s->release = release;
}

void ecm_UEContextRelease(gpointer ecm_h, S1AP_Message_t *s1msg){
    ECMSession_t *ecm = (ECMSession_t *)ecm_h;
    Cause_t *c = (Cause_t*)s1ap_getIe(s1msg, id_Cause);
    /*
      TS.23.401 clause 5.3.5 --
      "User Inactivity",
      "Radio Connection With UE Lost",
      "CSG Subscription Expiry",
      "CS Fallback triggered",
      "Redirection towards 1xRTT",
      "Inter-RAT Redirection",
      "UE Not Available for PS Service"
    */
    ecm->causeRelease = c;
    emm_UEContextReleaseReq(ecm->emm, sendUEContextReleaseCommand, ecm);
}
