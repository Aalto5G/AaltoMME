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
#include "ECMSession.h"

static void processMsg(gpointer _ecm, S1AP_Message_t *s1msg, int r_sid){
    ECMSession_t *ecm = (ECMSession_t *)_ecm;
    MME_UE_S1AP_ID_t *mme_id;
    ENB_UE_S1AP_ID_t *eNB_ID;
    Unconstrained_Octed_String_t *nASPDU;

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
             s1msg->choice == initiating_message){
	    log_msg(LOG_WARNING, 0, "Received id_UEContextRelease");

    }else if(s1msg->pdu->procedureCode ==  id_InitialContextSetup &&
             s1msg->choice == successful_outcome){
	    log_msg(LOG_WARNING, 0, "Received InitialContextSetupResponse");
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
        log_msg(LOG_WARNING, 0, "Received id_UEContextReleaseRequest");
    }else if(s1msg->pdu->procedureCode == id_UECapabilityInfoIndication &&
             s1msg->choice == initiating_message){
	    log_msg(LOG_WARNING, 0, "Received id_UECapabilityInfoIndication");
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

static void disconnect(gpointer _ecm){
}

void linkECMSessionConnected(ECMSession_State* s){
    s->processMsg = processMsg;
    s->disconnect = disconnect;
}
