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
#include "ECMSession.h"
#include "MME_S1.h"

static void process_eNBConfigurationTransfer(S1Assoc_t *assoc,
                                             S1AP_Message_t *s1msg);
static void process_reset(S1Assoc_t *assoc, S1AP_Message_t *S1msg);
static void process_reset_uas(S1Assoc_t *assoc, S1AP_Message_t *s1msg,
                              guint8 sid);

static void processMsg(gpointer _assoc, S1AP_Message_t *s1msg, int r_sid,
                       GError** err){
    S1Assoc_t *assoc = (S1Assoc_t *)_assoc;
    ECMSession ecm;
    MME_UE_S1AP_ID_t *mme_id;
    ENB_UE_S1AP_ID_t *enb_id;
    struct mme_t * mme = s1_getMME(assoc->s1);

    if(r_sid == assoc->nonue_rsid){
        /* ************************************************** */
        /*             Non UE Associated Signaling            */
        /* ************************************************** */
        if(s1msg->pdu->procedureCode == id_Reset){
            log_msg(LOG_DEBUG, 0, "Received Reset");
            process_reset(assoc, s1msg);
        }else if(s1msg->pdu->procedureCode == id_initialUEMessage &&
                 s1msg->choice == initiating_message){
            ecm = ecmSession_init(assoc, s1msg, r_sid);
        }else if(s1msg->pdu->procedureCode == id_ErrorIndication &&
                 s1msg->choice == initiating_message){
            log_msg(LOG_WARNING, 0, "Received Error Indication");
        }else if(s1msg->pdu->procedureCode == id_eNBConfigurationTransfer &&
                 s1msg->choice == initiating_message){
            process_eNBConfigurationTransfer(assoc, s1msg);
            log_msg(LOG_DEBUG, 0, "Received eNB Configuration Transfer");
        }else if(s1msg->pdu->procedureCode == id_ENBConfigurationUpdate &&
                 s1msg->choice == initiating_message){
            log_msg(LOG_WARNING, 0, "Received eNB Configuration Update");
        }else if(s1msg->pdu->procedureCode == id_PathSwitchRequest &&
                 s1msg->choice == initiating_message){
            log_msg(LOG_WARNING, 0, "Received Path Switch Request");
            mme_id = s1ap_findIe(s1msg, id_SourceMME_UE_S1AP_ID);
            mme_lookupECM(mme, mme_id->mme_id, ecm);
            ecmSession_pathSwitchReq(ecm, assoc, s1msg, r_sid);
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
    }else if(s1msg->pdu->procedureCode == id_initialUEMessage &&
             s1msg->choice == initiating_message){
        /* ************************************************** */
        /*         Setup of new UE associated signaling       */
        /* ************************************************** */
        ecm = ecmSession_init(assoc, s1msg, r_sid);
    }else if(s1msg->pdu->procedureCode == id_PathSwitchRequest &&
                 s1msg->choice == initiating_message){
        /* ************************************************** */
        /*                 Path Switch Request                */
        /* ************************************************** */
        log_msg(LOG_DEBUG, 0, "Received Path Switch Request");
        mme_id = s1ap_findIe(s1msg, id_SourceMME_UE_S1AP_ID);
        mme_lookupECM(mme, mme_id->mme_id, &ecm);
        ecmSession_pathSwitchReq(ecm, assoc, s1msg, r_sid);
    }else if(s1msg->pdu->procedureCode == id_Reset &&
             s1msg->choice == initiating_message){
        /* ************************************************** */
        /*                        Reset                       */
        /* ************************************************** */
        log_msg(LOG_DEBUG, 0, "Received Reset");
        process_reset_uas(assoc, s1msg, r_sid);
    }else{
        /* ************************************************** */
        /*               UE associated signaling              */
        /* ************************************************** */
        log_msg(LOG_DEBUG, 0, "Received UE associated signaling message");
        mme_id = s1ap_findIe(s1msg, id_MME_UE_S1AP_ID);
        enb_id = s1ap_findIe(s1msg, id_eNB_UE_S1AP_ID);
        ecm = s1Assoc_getECMSession(assoc, enb_id->eNB_id);
        if (!ecm){
            log_msg(LOG_ERR, 0, "eNB UE S1AP (%u) not recognized",
                    enb_id->eNB_id);
            return;
        }
        if(mme_id->mme_id != ecmSession_getMMEUEID(ecm)){
            log_msg(LOG_ERR, 0, "MME UE S1AP (%u) not matching the eNB UE S1AP (%u)",
                    mme_id->mme_id, enb_id->eNB_id);
            return;
        }
        ecmSession_processMsg(ecm, s1msg, r_sid);
    }
}

static void sendReset(gpointer _assoc){
    S1AP_Message_t *s1msg;
    S1AP_PROTOCOL_IES_t* ie;
    Cause_t *c;
    /* ResetType_t *t; */
    S1Assoc_t *assoc = (S1Assoc_t *)_assoc;
    struct mme_t * mme = s1_getMME(assoc->s1);

    /* Build response*/
    s1msg = S1AP_newMsg();
    s1msg->choice = successful_outcome;
    s1msg->pdu->procedureCode = id_Reset;
    s1msg->pdu->criticality = reject;

    /* c = s1ap_newIE(s1out, id_Cause, mandatory, ignore); */

    /* t = s1ap_newIE(s1out, id_ResetType, mandatory, ignore); */

    /* Send Response*/
    s1Assoc_sendNonUE(assoc, s1msg);

    /*s1msg->showmsg(s1msg);*/

    /* This function doesn't deallocate the IE stored on the mme structure,
     * because the freeValue callback attribute of the ProtocolIE structure is NULL */
    s1msg->freemsg(s1msg);

}

void linkS1AssocActive(S1Assoc_State* s){
    s->processMsg = processMsg;
}

static void process_eNBConfigurationTransfer(S1Assoc_t *assoc,  S1AP_Message_t *s1msg){
    struct mme_t * mme = s1_getMME(assoc->s1);
    Global_ENB_ID_t *global_eNB_ID;
/*     SONConfigurationTransfer_t  */


/*     global_eNB_ID = s1ap_findIe(s1msg, id_SONConfigurationTransferECT);   */

/*     global_eNB_ID = s1ap_findIe(s1msg, id_Global_ENB_ID); */

/*  mme_lookupS1Assoc(mme,); */
}

static gboolean s1Assoc_resetECM_adaptor(gpointer key,
                                         gpointer value,
                                         gpointer user_data){

    s1Assoc_resetECM(user_data, value);
    return TRUE;
}

static void process_reset(S1Assoc_t *assoc, S1AP_Message_t *s1msg){
    struct mme_t * mme = s1_getMME(assoc->s1);
    Cause_t *c;
    ResetType_t *t;
    S1AP_Message_t *out;


    /* Check Procedure*/
    if(s1msg->pdu->procedureCode != id_Reset &&
       s1msg->choice != initiating_message){
        g_error("Not a S1 Reset message");
    }

    c = s1ap_findIe(s1msg, id_Cause);
    t = s1ap_findIe(s1msg, id_ResetType);

    if(t->choice==0){
        log_msg(LOG_INFO, 0, "S1 Reset - Reset All: (%s)", assoc->eNBname->str);

        /* Reset All ECM*/
        g_hash_table_foreach_remove(assoc->ecm_sessions, s1Assoc_resetECM_adaptor, assoc);

        /* Send Reset Ack*/
        out = S1AP_newMsg();
        out->choice = successful_outcome;
        out->pdu->procedureCode = id_Reset;
        out->pdu->criticality = reject;
        s1Assoc_sendNonUE(assoc, out);
        out->freemsg(out);

    }else if(t->choice == 1){
        log_msg(LOG_WARNING, 0, "S1 Reset, not a reset-all, unexpected");
        return;
    }
}

static void process_reset_uas(S1Assoc_t *assoc, S1AP_Message_t *s1msg,
                              guint8 r_sid){
    Cause_t *c;
    ResetType_t *t;
    UE_associatedLogicalS1_ConnectionListResAck_t *l;
    UE_associatedLogicalS1_ConnectionItem_t *item;
    S1AP_Message_t *s1out;
    guint i = 0;
    ECMSession ecm;
    struct mme_t * mme = s1_getMME(assoc->s1);

    /* Check Procedure*/
    if(s1msg->pdu->procedureCode != id_Reset &&
       s1msg->choice != initiating_message){
        g_error("Not a S1 Reset message");
    }

    c = s1ap_findIe(s1msg, id_Cause);
    t = s1ap_findIe(s1msg, id_ResetType);

    if(t->choice==0){
        log_msg(LOG_WARNING, 0, "S1 Reset, not a reset-all, unexpected");
    }else if(t->choice == 1){
        log_msg(LOG_INFO, 0, "S1 Reset - Reset (%s). "
                "Remote sid: %u S1AP-MME-UE-id %u",
                assoc->eNBname->str, r_sid, 0);

        l = t->type.partOfS1_Interface;
        /* t->type.partOfS1_Interface */
        for(i=0; i < l->size ; i++){
            item = (UE_associatedLogicalS1_ConnectionItem_t *)(l->item[i]->value);
            if((item->opt&0x40)==0x40){
                ecm = s1Assoc_getECMSession(assoc,
                                            item->eNB_UE_S1AP_ID->eNB_id);
            }else if((item->opt&0x80)==0x80){
                mme_lookupECM(mme, item->mME_UE_S1AP_ID->mme_id, &ecm);
            }else{
                log_msg(LOG_WARNING, 0, "Empty UE associated Logical "
                        "S1 Connection Item");
                continue;
            }
            s1Assoc_resetECM(assoc, ecm);
        }
        /* Send Reset Ack*/
        s1out = S1AP_newMsg();
        s1out->choice = successful_outcome;
        s1out->pdu->procedureCode = id_Reset;
        s1out->pdu->criticality = reject;
        s1ap_setValueOnNewIE(s1out,
                             id_UE_associatedLogicalS1_ConnectionListResAck,
                             mandatory, ignore, (GenericVal_t *)t);

        s1Assoc_send(assoc, ecmSession_getLocalSid(ecm), s1out);
        s1out->freemsg(s1out);
        return;
    }
}
