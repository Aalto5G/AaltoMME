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

static void emmProcessMsg(gpointer emm_h,  GenericNASMsg_t* msg){
    EMMCtx_t *emm = (EMMCtx_t*)emm_h;

    if(msg->header.securityHeaderType.v != PlainNAS){
        log_msg(LOG_ERR, 0, "NAS Integrity or security not implemented");
        return;
    }

    switch(msg->plain.eMM.messageType){

    case AttachRequest:
        log_msg(LOG_WARNING, 0, "Recvd Attach Request");
        if(!0){ /* !isIMSIavailable(emm) */
            if(0){
                /* Get EMM context from the correct MME (use GUTI)*/
                return;
            }else{
                /* Send Identity Request*/
                return;
            }
        }
        /* Check Auth*/
        if(!0){ /* No New security context available*/
            /* Get Auth info from S6a*/
            return;
        }
        if(!0){ /* ksi in message != 7*/
            /* Send Auth request */
            /* Set T3460 */
            return;
        }
        if(!0){ /* ksi in msg != ksi ctx*/
            /* Remove Sec. Ctx*/
            if(0){ /* New context available in EMM ctx*/
                /* Send Auth request */
                /* Set T3460 */
                return;
            }else{
                /* Get Auth info from S6a*/
                return;
            }
        }
        /* -> Auth Valid */

        /* Check Security */
        if(!0){

        }
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
