/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S1Assoc_NotConfigured.c
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  S1 Assoc NotConfigured State
 *
 */

#include <glib.h>
#include "S1Assoc_NotConfigured.h"
#include "logmgr.h"
#include "S1Assoc_priv.h"
#include "S1Assoc_FSMConfig.h"
#include "MME_S1_priv.h"
#include "S1AP.h"

static void sendS1SetupResponse(S1Assoc_t *assoc);

static void processMsg(gpointer _assoc, S1AP_Message_t *s1msg, int r_sid){
    S1Assoc_t *assoc = (S1Assoc_t *)_assoc;
    ENBname_t       *eNBname;
    Global_ENB_ID_t *global_eNB_ID;
    struct mme_t * mme = s1_getMME(assoc->s1);

    /* Check Procedure*/
    if(s1msg->pdu->procedureCode != id_S1Setup &&
       s1msg->choice != initiating_message){
        log_msg(LOG_WARNING, 0, "Not a S1-Setup Request but %s, ignoring",
                elementaryProcedureName[s1msg->pdu->procedureCode]);
        return;
    }

    eNBname = s1ap_findIe(s1msg, id_eNBname);              /*OPTIONAL*/
    if(eNBname){
        g_string_assign(assoc->eNBname, eNBname->name);
    }

    global_eNB_ID = s1ap_getIe(s1msg, id_Global_ENB_ID);
    s1Assoc_setGlobalID(assoc, global_eNB_ID);

    assoc->suportedTAs     = s1ap_getIe(s1msg, id_SupportedTAs);
    //assoc->cGS_IdList      = s1ap_getIe(s1msg, id_CSG_IdList);

    CHECKIEPRESENCE(global_eNB_ID);
    CHECKIEPRESENCE(assoc->suportedTAs);

    assoc->nonue_rsid = r_sid;
    assoc->nonue_lsid = 0;

    log_msg(LOG_INFO, 0, "S1-Setup : new eNB \"%s\", connection added", assoc->eNBname->str);
    sendS1SetupResponse(assoc);
    mme_registerS1Assoc(mme, assoc);
    s1ChangeState(assoc, Active);
}

static void disconnect(gpointer _assoc){
    /*Do nothing*/
    return;
}


void linkS1AssocNotConfigured(S1Assoc_State* s){
    s->processMsg = processMsg;
    s->disconnect = disconnect;
}

static void sendS1SetupResponse(S1Assoc_t *assoc){
    S1AP_Message_t *s1msg;
    S1AP_PROTOCOL_IES_t* ie;

    struct mme_t * mme = s1_getMME(assoc->s1);
    /* Forge response*/
    s1msg = S1AP_newMsg();
    s1msg->choice = successful_outcome;
    s1msg->pdu->procedureCode = id_S1Setup;
    s1msg->pdu->criticality = reject;
    s1msg->pdu->value = new_ProtocolIE_Container();

    /*mme->name->showIE(mme->name);*/

    /* MME Name (optional)*/
    if(mme->name != NULL){
        ie=newProtocolIE();
        ie->id = id_MMEname;
        ie->presence = optional;
        ie->criticality = ignore;
        ie->value = mme->name;
        ie->showValue = mme->name->showIE;
        s1msg->pdu->value->addIe(s1msg->pdu->value, ie);
    }

    /* Served GUMMEIs*/
    ie=newProtocolIE();
    if(ie == NULL){
        log_msg(LOG_ERR, 0, "S1AP: Coudn't allocate new Protocol IE structure");
    }

    ie->id = id_ServedGUMMEIs;
    ie->presence = optional;
    ie->criticality = reject;
    ie->value = mme->servedGUMMEIs;
    ie->showValue = mme->servedGUMMEIs->showIE;
    s1msg->pdu->value->addIe(s1msg->pdu->value, ie);

    /* Relative MME Capacity*/
    ie=newProtocolIE();
    ie->id = id_RelativeMMECapacity;
    ie->presence = optional;
    ie->criticality = ignore;
    ie->value = mme->relativeCapacity;
    ie->showValue = mme->relativeCapacity->showIE;
    s1msg->pdu->value->addIe(s1msg->pdu->value, ie);

    /* Send Response*/
    s1Assoc_sendNonUE(assoc, s1msg);

    /*s1msg->showmsg(s1msg);*/

    /* This function doesn't deallocate the IE stored on the mme structure,
     * because the freeValue callback attribute of the ProtocolIE structure is NULL */
    s1msg->freemsg(s1msg);

}
