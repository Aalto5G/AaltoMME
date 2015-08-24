/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S1Assoc_Active.c
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  S1 Assoc Active State
 *
 */

#include "S1Assoc_Active.h"
#include "MME.h"
#include "logmgr.h"
#include "MME_S1_priv.h"
#include "S1Assoc_priv.h"
#include "S1Assoc_FSMConfig.h"

static void processMsg(gpointer _assoc, S1AP_Message_t *s1msg, int r_sid){
    S1Assoc_t *assoc = (S1Assoc_t *)_assoc;

    if(r_sid == assoc->nonue_rsid){
        /* ************************************************** */
        /*             Non UE Associated Signaling            */
        /* ************************************************** */
        if(s1msg->pdu->procedureCode == id_Reset){
        }else if(s1msg->pdu->procedureCode == id_initialUEMessage &&
                 s1msg->choice == initiating_message){
            log_msg(LOG_WARNING, 0, "Received Initial UE Message");
        }else if(s1msg->pdu->procedureCode == id_ErrorIndication &&
                 s1msg->choice == initiating_message){
            log_msg(LOG_WARNING, 0, "Received Error Indication");
        }else if(s1msg->pdu->procedureCode == id_eNBConfigurationTransfer &&
                 s1msg->choice == initiating_message){
            log_msg(LOG_WARNING, 0, "Received eNB Configuration Transfer");
        }else if(s1msg->pdu->procedureCode == id_ENBConfigurationUpdate &&
                 s1msg->choice == initiating_message){
            log_msg(LOG_WARNING, 0, "Received eNB Configuration Update");
        }else if(s1msg->pdu->procedureCode == id_PathSwitchRequest &&
                 s1msg->choice == initiating_message){
            log_msg(LOG_WARNING, 0, "Received Path Switch Request");
        }else if(s1msg->pdu->procedureCode == id_WriteReplaceWarning &&
                 s1msg->choice == successful_outcome){
            log_msg(LOG_WARNING, 0, "Received Write ReplaceWarning Response");
        }else if(s1msg->pdu->procedureCode == id_Kill &&
                 s1msg->choice == successful_outcome){
            log_msg(LOG_WARNING, 0, "Received Kill Response");
        }else{
            log_msg(LOG_WARNING, 0, "Received %s on non UE associated signaling, ignoring",
                    elementaryProcedureName[s1msg->pdu->procedureCode]);
            return;
        }
    }else{
        /* ************************************************** */
        /*               UE associated signaling              */
        /* ************************************************** */
        log_msg(LOG_WARNING, 0, "Received UE associated signaling message");
    }
}

static void disconnect(gpointer _assoc){
    S1Assoc_t *assoc = (S1Assoc_t *)_assoc;
    struct mme_t * mme = s1_getMME(assoc->s1);
    mme_deregisterS1Assoc(mme, assoc);
}

void linkS1AssocActive(S1Assoc_State* s){
    s->processMsg = processMsg;
    s->disconnect = disconnect;
}
