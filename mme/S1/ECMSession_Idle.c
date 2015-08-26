/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   ECMSession_Idle.c
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  ECM Session Idle State
 *
 */

#include "ECMSession_Idle.h"
#include "MME.h"
#include "logmgr.h"
#include "MME_S1_priv.h"
#include "ECMSession_FSMConfig.h"
#include "ECMSession.h"
#include "ECMSession_priv.h"
#include "NAS_EMM.h"

static void ecm_processMsg(gpointer _ecm, S1AP_Message_t *s1msg, int r_sid){
    ECMSession_t *ecm = (ECMSession_t *)_ecm;

    ENB_UE_S1AP_ID_t *eNB_ID;
    Unconstrained_Octed_String_t *nASPDU;
    TAI_t *tAI;
    EUTRAN_CGI_t *eCGI;
    RRC_Establishment_Cause_t *cause;

    if(s1msg->pdu->procedureCode == id_initialUEMessage &&
       s1msg->choice == initiating_message){

        eNB_ID = (ENB_UE_S1AP_ID_t*)s1ap_findIe(s1msg, id_eNB_UE_S1AP_ID);
        ecm->eNBUEId = eNB_ID->eNB_id;

        nASPDU = (Unconstrained_Octed_String_t*)s1ap_findIe(s1msg, id_NAS_PDU);

        tAI = (TAI_t*)s1ap_findIe(s1msg, id_TAI);
        eCGI = (EUTRAN_CGI_t*)s1ap_findIe(s1msg, id_EUTRAN_CGI);
        cause = (RRC_Establishment_Cause_t*)s1ap_findIe(s1msg,
                                                        id_RRC_Establishment_Cause);

        emm_processMsg(ecm->emm, nASPDU->str, nASPDU->len);
    }
}

static void disconnect(gpointer _ecm){
}

void linkECMSessionIdle(ECMSession_State* s){
    s->processMsg = ecm_processMsg;
    s->disconnect = disconnect;
}
