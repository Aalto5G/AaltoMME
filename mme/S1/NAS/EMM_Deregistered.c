/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   EMM_Deregistered.c
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  EMM State
 *
 * The MME has no EMM context or the EMM Context is marked as detached
 */

#include "EMM_Deregistered.h"
#include "logmgr.h"
#include "EMM_FSMConfig.h"
#include "NAS.h"
#include "NAS_EMM_priv.h"

void processAttach(gpointer emm_h,  GenericNASMsg_t* msg);
void sendIdentityReq(gpointer emm_h);

static void emmProcessMsg(gpointer emm_h,  GenericNASMsg_t* msg){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;

    if(msg->header.securityHeaderType.v != PlainNAS){
        log_msg(LOG_ERR, 0, "NAS Integrity or security not implemented");
        return;
    }

    switch(msg->plain.eMM.messageType){

    case AttachRequest:
        processAttach(emm, msg);
        break;
    default:
        log_msg(LOG_WARNING, 0,
                "NAS Message type (%u) not recognized in this context",
                msg->plain.eMM.messageType);
    }
}


void linkEMMDeregistered(EMM_State* s){
    s->processMsg = emmProcessMsg;
}

void processAttach(gpointer emm_h,  GenericNASMsg_t* msg){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    AttachRequest_t *attachMsg;
    guint ksi_msg, i;
    uint64_t mobid=0ULL;

    attachMsg = (AttachRequest_t*)&(msg->plain.eMM);
    ksi_msg = attachMsg->nASKeySetId.v & 0x07;

    if(((ePSMobileId_header_t*)attachMsg->ePSMobileId.v)->type == 1 ){  /* IMSI*/
        for(i=0; i<attachMsg->ePSMobileId.l-1; i++){
            mobid = mobid*10 + ((attachMsg->ePSMobileId.v[i])>>4);
            mobid = mobid*10 + ((attachMsg->ePSMobileId.v[i+1])&0x0F);
        }
        if(((ePSMobileId_header_t*)attachMsg->ePSMobileId.v)->parity == 1){
            mobid = mobid*10 + ((attachMsg->ePSMobileId.v[i])>>4);
        }
        log_msg(LOG_DEBUG, 0,"Attach Received from imsi : %llu", mobid);
    }

    if(mobid == 0ULL){ /* !isIMSIavailable(emm) */
        if(0){
            /* Get EMM context from the correct MME (use GUTI)*/
            return;
        }else{
            sendIdentityReq(emm);
            emmChangeState(emm, EMM_CommonProcedureInitiated);
            return;
        }
    }
    /* Check Auth, Proof down*/
    if( emm->ksi == 7 && !emm->authQuadrsLen>0){
        log_msg(LOG_DEBUG, 0,"Getting info from HSS");
        /* Get Auth info from S6a*/
        return;
    }else if(emm->ksi == 7 && emm->authQuadrsLen>0
             || emm->ksi != ksi_msg){
        /* Remove Sec. Ctx*/
        /* New context available in EMM ctx*/
        /* Send Auth request */
        /* Set T3460 */
        emm_sendAuthRequest(emm);
        return;
    }
    /* User Authenticated,
     * Proof:
     *
     * A = Sec ctx available
     * B = New Sec ctx available
     * C = UE Sec Ctx available
     * D = UE Sec equal to MME Sec Ctx
     * Eqs:
     * 1) not (not A and not B) and not (not A and B or not D)
     *    = A and D
     * 2) A and D -> C
     * */

    /* Check Security */
    if(!0){

    }
}

void sendIdentityReq(gpointer emm_h){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;
    guint8 *pointer;
    guint8 buffer[150];

    log_msg(LOG_DEBUG, 0, "Building Identity Request");

    pointer = buffer;
    newNASMsg_EMM(&pointer, EPSMobilityManagementMessages, PlainNAS);

    encaps_EMM(&pointer, IdentityRequest);

    /* Selected NAS security algorithms */
    nasIe_v_t1_l(&pointer, 1); /*Get Imsi*/
    pointer++; /*Spare half octet*/

    ecm_send(emm->ecm, pointer, pointer-buffer);
}
