/* AaltoMME - Mobility Management Entity for LTE networks
 * Copyright (C) 2013 Vicent Ferrer Guash & Jesus Llorente Santos
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#include "S1Assoc_priv.h"
#include "NAS_Definitions.h"

static void ecm_processMsg(gpointer _ecm, S1AP_Message_t *s1msg, int r_sid){
    ECMSession_t *ecm = (ECMSession_t *)_ecm;

    ENB_UE_S1AP_ID_t *eNB_ID;
    Unconstrained_Octed_String_t *nASPDU;
    TAI_t *tAI;
    EUTRAN_CGI_t *eCGI;
    RRC_Establishment_Cause_t *cause;
    guti_t guti;
    S_TMSI_t *sTMSI = NULL;
    guint32 mtmsi;
    guint64 imsi;
    struct mme_t * mme = s1_getMME(s1Assoc_getS1(ecm->assoc));
    memset(&guti, 0, sizeof(guti_t));

    if(s1msg->pdu->procedureCode == id_initialUEMessage &&
       s1msg->choice == initiating_message){

        eNB_ID = (ENB_UE_S1AP_ID_t*)s1ap_findIe(s1msg, id_eNB_UE_S1AP_ID);
        ecm->eNBUEId = eNB_ID->eNB_id;

        nASPDU = (Unconstrained_Octed_String_t*)s1ap_findIe(s1msg, id_NAS_PDU);

        tAI = (TAI_t*)s1ap_findIe(s1msg, id_TAI);
        memcpy(ecm->tAI.sn, tAI->pLMNidentity->tbc.s, 3);
        memcpy(&(ecm->tAI.tAC), tAI->tAC->s, 2);
        eCGI = (EUTRAN_CGI_t*)s1ap_findIe(s1msg, id_EUTRAN_CGI);
        cause = (RRC_Establishment_Cause_t*)s1ap_findIe(s1msg,
                                                        id_RRC_Establishment_Cause);

        ecm->l_sid = 1;
        if(r_sid != 0){
            ecm->r_sid_valid = TRUE;
            ecm->r_sid = r_sid;
        }

        sTMSI = (S_TMSI_t*)s1ap_findIe(s1msg, id_S_TMSI);
        if(sTMSI){
            memcpy(&mtmsi, sTMSI->m_TMSI.s, 4);
            mme_lookupEMMCtxt(mme, mtmsi, &(ecm->emm));
        }else{
            emm_getGUTIfromMsg(nASPDU->str, nASPDU->len, &guti);
            if(guti.mtmsi != 0 && mme_GUMMEI_IsLocal(mme,
                                                     guti.tbcd_plmn,
                                                     guti.mmegi,
                                                     guti.mmec)){
                mme_lookupEMMCtxt(mme, guti.mtmsi, &(ecm->emm));
            }else{
                emm_getIMSIfromAttach(nASPDU->str, nASPDU->len, &imsi);
                mme_lookupEMMCtxt_byIMSI(mme, imsi, &(ecm->emm));
            }
        }

        if(ecm->emm == NULL){
            ecm->emm = emm_init(ecm, mme_getTimerMgr(mme));
            ecm_log(ecm, LOG_INFO, 0, "New EMM created");
        }else{
            emm_registerECM(ecm->emm, ecm);
        }
        ecm_ChangeState(ecm, ECM_Connected);
        ecm_log(ecm, LOG_INFO, 0, "Connected");
        emm_processMsg(ecm->emm, nASPDU->str, nASPDU->len);
    }
}

static void release(gpointer _ecm, cause_choice_t choice, uint32_t cause){
    ecm_log(_ecm, LOG_ERR, 0, "Cannot release an Idle ECM");
}

void linkECMSessionIdle(ECMSession_State* s){
    s->processMsg = ecm_processMsg;
    s->release = release;
}
