/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   MME_S1.c
 * @Author Vicent Ferrer
 * @date   May, 2013
 * @brief  MME S1 interface protocol state machine.
 *
 * This module implements the S1 interface state machine.
 */

#include "MME_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <netinet/sctp.h>
#include "signals.h"

//#include "MME.h"
//#include "NAS_FSM.h"
#include "S1AP.h"
#include "MME_S1.h"
#include "MME_S1_priv.h"
#include "NAS_FSM.h"
#include "logmgr.h"
#include "hmac_sha2.h"
#include "S1Assoc.h"
#include "S1Assoc_FSMConfig.h"


/* ======================================================================
 * Previous Definitions
 * ====================================================================== */


/** Macro to check mandatory IE presence */
#define CHECKIEPRESENCE(p) if(p==NULL){ log_msg(LOG_ERR, 0, "IE not found on message"); return 1; }

/** Dummy null function callback */
typedef void (*nullparam_cb_cast)(void *);


/* ======================================================================
 * TASK Prototypes
 * ====================================================================== */


static uint8_t TASK_MME_S1___S1Setup(Signal *signal);
static uint8_t TASK_MME_S1___initUEMsg(Signal *signal);
static uint8_t TASK_MME_S1___Forge_InitCtxSetupReq(Signal *signal);
static uint8_t TASK_MME_S1___Validate_InitCtxSetupRes(Signal *signal);
static uint8_t TASK_MME_S1___TransparentNAS(Signal *signal);
static uint8_t TASK_MME_S1___Forge_TransparentNAS(uint8_t *nasResult, uint32_t bsize, Signal *signal);
static uint8_t TASK_MME_S1___Forge_UEContextReleaseCommand(Signal *signal, cause_choice_t choice, uint32_t cause);
static uint8_t TASK_MME_S1___Validate_UEContextReleaseComplete(Signal *signal);
static uint8_t TASK_MME_S1___Validate_PathSwitchRequest(Signal *signal);
static uint8_t TASK_MME_S1___Forge_PathSwitchFailure(Signal *signal);
static uint8_t TASK_MME_S1___Forge_PathSwitchAck(Signal *signal);
static uint8_t TASK_MME_S1___Validate_HandoverRequired(Signal *signal);
static uint8_t TASK_MME_S1___Forge_HandoverReq(Signal *signal);
static uint8_t TASK_MME_S1___Validate_HandoverReqAck(Signal *signal);
static uint8_t TASK_MME_S1___Forge_HandoverCommand(Signal *signal);
static uint8_t TASK_MME_S1___Replay_StatusTransfer(Signal *signal);
static uint8_t TASK_MME_S1___Validate_HandoverNotify(Signal *signal);

/* Forward declarations*/

void S1_newUserSession(struct t_engine_data *engine, struct EndpointStruct_t* ep_S1, S1AP_Message_t *s1msg, uint16_t sid);





/** S1 Accept function callback. Used to accept a new S1-MME connection (from eNB)*/
void s1_accept_new_eNB(evutil_socket_t ss, short event, void *arg){

    struct S1_t *self = (struct S1_t*)arg;
    S1Assoc assoc;
    log_msg(LOG_DEBUG, 0, "enter s1_accept_new_eNB()");

    assoc = s1Assoc_init(self);
    s1Assoc_accept(assoc, ss);
}


gpointer s1_init(gpointer mme){
    S1_t *self = g_new0(S1_t, 1);

    self->mme = mme;

    s1ConfigureFSM();

    /*Init S1 server*/
    self->fd =init_sctp_srv(mme_getLocalAddress(self->mme), S1AP_PORT);
    log_msg(LOG_INFO, 0, "Open S1 server on file descriptor %d, port %d",
            self->fd, S1AP_PORT);

    mme_registerRead(self->mme, self->fd, s1_accept_new_eNB, self);

    self->assocs = g_hash_table_new_full( g_int_hash,
                                        g_int_equal,
                                        NULL,
                                        s1Assoc_free);

    return self;
}

void s1_free(S1 s1_h){
    S1_t *self = (S1_t *) s1_h;

    g_hash_table_destroy(self->assocs);
    mme_deregisterRead(self->mme, self->fd);
    close(self->fd);
    s1DestroyFSM();
    g_free(self);
}

void s1_registerAssoc(S1 s1_h, gpointer assoc, int fd, event_callback_fn cb){
    S1_t *self = (S1_t *) s1_h;

    /*Store the new connection*/
    g_hash_table_insert(self->assocs, s1Assoc_getfd_p(assoc), assoc);
    mme_registerRead(self->mme, fd, cb, assoc);
}

void s1_deregisterAssoc(S1 s1_h, gpointer assoc){
    S1_t *self = (S1_t *) s1_h;
    mme_deregisterRead(self->mme, s1Assoc_getfd(assoc));
    g_hash_table_remove(self->assocs, s1Assoc_getfd_p(assoc));
}


struct mme_t *s1_getMME(S1_t *self){
    return self->mme;
}


/* ======================================================================
 * S1 type related functions
 * ====================================================================== */


void free_S1_EndPoint_Info(S1_EndPoint_Info_t *epInfo){
    if(epInfo->global_eNB_ID){
        if(epInfo->global_eNB_ID->freeIE){
            epInfo->global_eNB_ID->freeIE(epInfo->global_eNB_ID);
        }
    }

    if(epInfo->eNBname){
        if(epInfo->eNBname->freeIE){
            epInfo->eNBname->freeIE(epInfo->eNBname);
        }
    }

    if(epInfo->supportedTAs){
        if(epInfo->supportedTAs->freeIE){
            epInfo->supportedTAs->freeIE(epInfo->supportedTAs);
        }
    }

    if(epInfo->cSG_IdList){
        if(epInfo->cSG_IdList->freeIE){
            epInfo->cSG_IdList->freeIE(epInfo->supportedTAs);
        }
    }
}


/* ======================================================================
 * S1 Tool API implementation
 * ====================================================================== */


/**@brief S1 Send message
 * @param [in] ep_S1    Destination EndPoint information
 * @param [in] streamId Strem to send the message
 * @param [in] s1msg    Message to be sent
 *
 * This function send the S1 message using the SCTP protocol
 * */
void s1_sendmsg(struct EndpointStruct_t* ep_S1, uint32_t streamId, S1AP_Message_t *s1msg){
    uint8_t buf[10000];
    uint32_t bsize, ret;
    S1_EndPoint_Info_t *epInfo;

    memset(buf, 0, 10000);
    log_msg(LOG_DEBUG, 0, "S1AP: Send %s", elementaryProcedureName[s1msg->pdu->procedureCode]);
    s1ap_encode(buf, &bsize, s1msg);

    /*printfbuffer(buf, bsize);*/

    /* sctp_sendmsg*/
    ret = sctp_sendmsg( ep_S1->fd, (void *)buf, (size_t)bsize, NULL, 0, SCTP_S1AP_PPID, 0, streamId, 0, 0 );

    if(ret==-1){
        epInfo = (S1_EndPoint_Info_t *)ep_S1->info;
        log_msg(LOG_ERR, errno, "S1AP : Error sending SCTP message to eNB %s", epInfo->eNBname->name);
    }
}


/* ======================================================================
 * S1 Private Tool functions
 * ====================================================================== */


/**
 * @brief generate_NH -
 * @param [inout]  sec      security context
 *
 * fill the security context with a new derived NH (next hop) parameter with the KDF function
 */
static void refresh_NH(SecurityCtx_t* sec){

    /*
    -    FC = 0x12
    -    P0 = SYNC-input
    -    L0 = length of SYNC-input (i.e. 0x00 0x20)
    The SYNC-input parameter shall be the newly derived K eNB  for the initial NH derivation, and the previous NH for all
    subsequent derivations. This results in a NH chain, where the next NH is always fresh and derived from the previous
    NH.
     */

    uint8_t s[35], zero[32];
    memset(zero, 0, 32);

    s[0]=0x12;
    if(memcmp(sec->nh, zero, 32)==0){
        /*First hop*/
        memcpy(s+1, sec->keNB, 32);
    }else{
        memcpy(s+1, sec->nh, 32);
        sec->ncc++;
    }
    s[33]=0;
    s[34]=0x20;

    hmac_sha256(sec->kASME, 32, s, 35, sec->nh, 32);
}


/* ======================================================================
 * S1 FSM State Implementations
 * ====================================================================== */


/**@brief
 *
 *
 */
static int STATE_S1_handle(Signal *signal)
{
    uint8_t res;
    struct  SessionStruct_t *usersession = NULL;
    struct sctp_sndrcvinfo *sndrcvinfo;
    struct EndpointStruct_t  *s1ep;
    struct t_message *msg;
    S1AP_Message_t *s1msg;
    Signal *output;
    MME_UE_S1AP_ID_t *mmeUEId;

    log_msg(LOG_DEBUG, 0, "Enter");

    if(signal->name != S1_handler_ready) {
        return 0;
    }

    s1ep = (((struct EndpointStruct_t  *)signal->processTo->data));
    sndrcvinfo = &((S1_EndPoint_Info_t*)s1ep->info)->sndrcvinfo;
    msg = (struct t_message *)signal->data;

    s1msg = s1ap_decode((void *)msg->packet.raw, msg->length);

    switch(sndrcvinfo->sinfo_stream){
    case 0:
        log_msg(LOG_DEBUG, 0, "Stream %d", sndrcvinfo->sinfo_stream);
        /*printfbuffer(msg->packet.raw, msg->length);*/

        if(s1msg->pdu->procedureCode == id_initialUEMessage && s1msg->choice == initiating_message){
            log_msg(LOG_DEBUG, 0, "S1AP: New user");
            S1_newUserSession(PROC->engine, s1ep, s1msg, 1);
            return 0;
            /*
        }else if(s1msg->pdu->procedureCode == id_eNBStatusTransfer && s1msg->choice == initiating_message){
            log_msg(LOG_DEBUG, 0, "Received an id_eNBStatusTransfer Msg");
            TASK_MME_S1___Replay_StatusTransfer(signal);
            return 0;*/
        }else{
            log_msg(LOG_INFO, 0, "Procedure %s not recognized on stream 0", elementaryProcedureName[s1msg->pdu->procedureCode]);
            return 0;
        }
        break;

    default:
        log_msg(LOG_DEBUG, 0, "User Stream ID %u", sndrcvinfo->sinfo_stream);
        /*printfbuffer(msg->packet.raw, msg->length);*/

        if(s1msg->pdu->procedureCode == id_initialUEMessage && s1msg->choice == initiating_message){
            log_msg(LOG_DEBUG, 0, "S1AP: New user");
            S1_newUserSession(PROC->engine, s1ep, s1msg, sndrcvinfo->sinfo_stream);
            return 0;
        }

        mmeUEId = s1ap_findIe(s1msg, id_MME_UE_S1AP_ID);

        if(s1msg->pdu->procedureCode == id_PathSwitchRequest){
            mmeUEId = s1ap_findIe(s1msg, id_SourceMME_UE_S1AP_ID);
        }

        if(mmeUEId==NULL){
            log_msg(LOG_WARNING, 0, "MME_UE_S1AP_ID not found ignoring message");
            return 0;
        }

        /*Recover UE session on MME*/
        usersession = SELF_ON_SIG->s1apUsersbyLocalID[mmeUEId->mme_id];
        if(usersession==NULL || mmeUEId->mme_id == 0){
             log_msg(LOG_WARNING, 0, "Received Invalid MME-UE-S1AP-ID %u, Ignoring message", mmeUEId->mme_id);
             return 0;
        }

        if(usersession->sid != sndrcvinfo->sinfo_stream
           && s1msg->pdu->procedureCode != id_PathSwitchRequest){
            log_msg(LOG_WARNING, 0, "MME_UE_S1AP_ID not corresponds to the expected stream. Ignoring message");
            return 0;
        }

        output = new_signal(usersession->sessionHandler);
        if(s1msg->choice != initiating_message ||
           (s1msg->choice == initiating_message && s1msg->pdu->procedureCode == id_uplinkNASTransport)){

            /* Search session on hash table (key = stream, value = session pointer)*/
            if(usersession->pendingRsp == 1){
                removePendentResponse(usersession);
            }else if(s1msg->pdu->procedureCode == id_uplinkNASTransport){
                /* UPLINK NAS TRANSPORT, new procedure*/
            }else{
                log_msg(LOG_WARNING, 0, "Received an unexpected response. Ignoring message");
                return 0;
            }

            /* Relate signal with */
            if(s1msg->pdu->procedureCode == id_uplinkNASTransport){
                /*MMESTARTTIME*/
                output->name = NAS_data_available;
                output->priority = MAXIMUM_PRIORITY;
            }else if(s1msg->pdu->procedureCode == id_InitialContextSetup){
                /*output->name = ;*/
                output->priority = MAXIMUM_PRIORITY;
                log_msg(LOG_DEBUG, 0, "Received an InitialContextSetup Msg");
            }else if(s1msg->pdu->procedureCode == id_UEContextRelease){
                /*output->name = ;*/
                output->priority = MAXIMUM_PRIORITY;
                log_msg(LOG_DEBUG, 0, "Received an UEContextRelease Msg");
                /*MMEENDTIME*/
            }else if(s1msg->pdu->procedureCode == id_HandoverResourceAllocation){
                /*output->name = ;*/
                output->priority = MAXIMUM_PRIORITY;
                log_msg(LOG_DEBUG, 0, "Received an Handover Request Ack Msg");
            }else{
                log_msg(LOG_WARNING, 0, "Message not expected. Ignoring");
                return 0;
            }

        }else if(s1msg->pdu->procedureCode == id_HandoverPreparation && s1msg->choice == initiating_message){
            usersession->s1=s1ep;          /*This change should be after the validation of the message, not before as it is now*/
            /*output->name = ;*/
            output->priority = MAXIMUM_PRIORITY;
        }else if(s1msg->pdu->procedureCode == id_eNBStatusTransfer && s1msg->choice == initiating_message){
            log_msg(LOG_DEBUG, 0, "Received an id_eNBStatusTransfer Msg");
            /*output->name = ;*/
            output->priority = MAXIMUM_PRIORITY;
            /*TASK_MME_S1___Replay_StatusTransfer(signal, s1msg);*/
            /*return 0;*/
        }else if(s1msg->pdu->procedureCode == id_UEContextReleaseRequest && s1msg->choice == initiating_message){
            log_msg(LOG_DEBUG, 0, "Received an id_UEContextReleaseRequest Msg");
            /*output->name = ;*/
            output->priority = MAXIMUM_PRIORITY;
        }else if(s1msg->pdu->procedureCode == id_HandoverNotification && s1msg->choice == initiating_message){
            log_msg(LOG_DEBUG, 0, "Received an id_HandoverNotification Msg");
            /*output->name = ;*/
            output->priority = MAXIMUM_PRIORITY;
        }else if(s1msg->pdu->procedureCode == id_PathSwitchRequest && s1msg->choice == initiating_message){
            usersession->s1=s1ep;          /*This change should be after the validation of the message, not before as it is now*/
            usersession->sid = sndrcvinfo->sinfo_stream;
            output->name = S1_PathSwitchRequest;
            output->priority = MAXIMUM_PRIORITY;
        }else{
            log_msg(LOG_WARNING, 0, "Message not expected, ignoring. Procedure %s, choice %u",  elementaryProcedureName[s1msg->pdu->procedureCode], s1msg->choice);
            return 0;
        }
        break;
    }

    output->data = (void *)s1msg;
    output->freedataFunc = (nullparam_cb_cast)s1msg->freemsg;
    signal_send(output);
    return 0;

}

/**@brief   S1 Setup State
 *
 * State to control the setup of a new S1-MME connection
 */
static int STATE_S1_Setup(Signal *signal)
{
    uint8_t res;
    log_msg(LOG_DEBUG, 0, "Enter");
    switch (signal->name)
    {
    case S1_Setup_Endpoint:
        TASK_MME_S1___S1Setup(signal);
        /*This process is used as an endpoint control session, not deallocated*/
        /*process_stop(PDATA->sessionHandler);*/
        PDATA->s1->handler->next_state=STATE_S1_handle;
        PDATA->s1->handler->data = malloc(sizeof(struct sctp_sndrcvinfo));

        /*free session structure*/
        free(PDATA);
        return 0;
    default:                return(1);
    }
}

static int STATE_S1_Active(Signal *signal){
    uint8_t nasResult[1000], save=0;
    uint32_t bsize=0;
    uint8_t flag=0;
    S1AP_Message_t *s1msg;
    Signal *output;

    log_msg(LOG_DEBUG, 0, "Enter");

    if(signal->name == NAS_data_available){
        if(PDATA->user_ctx->stateNAS_EMM == EMM_SpecificProcedureInitiated) flag=1;
        TASK_MME_S1___TransparentNAS(signal);
        if(PDATA->user_ctx->stateNAS_EMM == EMM_Registered && flag){
            S11_Attach_ModifyBearerReq(PDATA->user_ctx->s11,
                              (void(*)(gpointer)) sendFirstStoredSignal,
                              (gpointer)PDATA->sessionHandler);
        }

    }else if(signal->name == S1_PathSwitchRequest){
        if(TASK_MME_S1___Validate_PathSwitchRequest(signal)==0){
            S11_Attach_ModifyBearerReq(PDATA->user_ctx->s11,
                                       (void(*)(gpointer)) sendFirstStoredSignal,
                                       (gpointer)PDATA->sessionHandler);
            Controller_newHandover(SELF_ON_SIG->sdnCtrl, PDATA->user_ctx);
            signal->name = S1_PathSwitchACK;
            save = 1;
        }else{
            TASK_MME_S1___Forge_PathSwitchFailure(signal);
        }

    }else if (signal->name == S1_PathSwitchACK){
        TASK_MME_S1___Forge_PathSwitchAck(signal);

    }else if (signal->name == S1_HandoverCommand){
        TASK_MME_S1___Forge_HandoverCommand(signal);

    }else if(signal->name == S1_UE_Context_Release){
        /*Timeout to release UE context on the source eNB*/
        TASK_MME_S1___Forge_UEContextReleaseCommand(signal, CauseRadioNetwork , CauseRadioNetwork_successful_handover);
        addToPendingResponse(PDATA);
    }else if(signal->name > NAS_data_available){
        NAS_sessionAvailable(nasResult, &bsize, signal);
        TASK_MME_S1___Forge_TransparentNAS(nasResult, bsize, signal);

    }else{  /*Without signal name*/
        if(signal->data == NULL){
            log_msg(LOG_ERR, 0, "Message not found");
            return 0;
        }

        s1msg = (S1AP_Message_t *)signal->data;

        if(s1msg->pdu->procedureCode == id_HandoverResourceAllocation &&
                s1msg->choice == successful_outcome){
            if (TASK_MME_S1___Validate_HandoverReqAck(signal)==0){
                /* Create Indirect Data Forwarding Tunnel*/
                /*S11_CreateIndirectDataForwardingTunnel(PROC->engine, PDATA);
                signal->name = S1_HandoverCommand;
                save = 1;*/
                /* Send Handover Command Message, remove this line to send a create Indirect Data Forwarding tunnel*/
                TASK_MME_S1___Forge_HandoverCommand(signal);
            }
        }else if(s1msg->pdu->procedureCode == id_HandoverPreparation){
            if (TASK_MME_S1___Validate_HandoverRequired(signal)==0){
                TASK_MME_S1___Forge_HandoverReq(signal);
                /*signal->name = S1_PathSwitchACK;
                save = 1;*/
            }else{

            }
        }else if(s1msg->pdu->procedureCode == id_eNBStatusTransfer){
            TASK_MME_S1___Replay_StatusTransfer(signal);
        }else if(s1msg->pdu->procedureCode == id_UEContextReleaseRequest){
          //if (TASK_MME_S1___Validate_UEContextReleaseRequest(signal)==0){
            output = new_signal(PDATA->sessionHandler);
            output->name = S1_UE_Context_Release;
            output->priority = MAXIMUM_PRIORITY/2;
            set_timeout(output, 0, 500000);
            S11_ReleaseAccessBearers(PDATA->user_ctx->s11,
                                     (void(*)(gpointer)) sendFirstStoredSignal,
                                     (gpointer)PDATA->sessionHandler);
            //}

        }else if(s1msg->pdu->procedureCode == id_HandoverNotification){
            if(TASK_MME_S1___Validate_HandoverNotify(signal)==0){
                output = new_signal(PDATA->sessionHandler);
                output->name = S1_UE_Context_Release;
                output->priority = MAXIMUM_PRIORITY/2;
                set_timeout(output, 0, 500000);

                S11_Attach_ModifyBearerReq(PDATA->user_ctx->s11,
                                           (void(*)(gpointer)) sendFirstStoredSignal,
                                           (gpointer)PDATA->sessionHandler);
                Controller_newHandover(SELF_ON_SIG->sdnCtrl, PDATA->user_ctx);
            }

        }

    }
    return save;
}

static int STATE_S1_SetupOfUEContext(Signal *signal){
    S1AP_Message_t *s1msg;
    uint8_t flag=0;

    log_msg(LOG_DEBUG, 0, "Enter");

    if(signal->name == S1_ContinueAttach){
        if(TASK_MME_S1___Forge_InitCtxSetupReq(signal)){
            /*ERROR*/
        }
        return 0;

    }else if(signal->name >= NAS_data_available){

        if(PDATA->user_ctx->stateNAS_EMM == EMM_SpecificProcedureInitiated) flag=1;
        TASK_MME_S1___TransparentNAS(signal);
        if(PDATA->user_ctx->stateNAS_EMM == EMM_Registered && flag){
            /*Received Attach Complete but not the Initial Context Setup Response,
             * Added session to pending response list*/
            addToPendingResponse(PDATA);
        }
        PDATA->user_ctx->sec_ctx.ulNAScnt++;
        return 0;
    }

    if(signal->data == NULL){
        log_msg(LOG_ERR, 0, "Message not found");
        return 0;
    }

    s1msg = (S1AP_Message_t *)signal->data;

    if(s1msg->pdu->procedureCode != id_InitialContextSetup){
        /*ERROR*/
        return 0;
    }

    if( s1msg->choice == successful_outcome){

        TASK_MME_S1___Validate_InitCtxSetupRes(signal);

        if (PDATA->user_ctx->stateNAS_EMM != EMM_Registered){
            /* Attach message not received yet, add session to pending response*/
            addToPendingResponse(PDATA);
        }else{
            S11_Attach_ModifyBearerReq(PDATA->user_ctx->s11,
                                       (void(*)(gpointer)) sendFirstStoredSignal,
                                       (gpointer)PDATA->sessionHandler);
        }
        signal->processTo->next_state = STATE_S1_Active;
    }else if(s1msg->choice == unsuccessful_outcome){

    }
    return 0;
}

static int STATE_S1_ProcessUeMsg(Signal *signal){
    log_msg(LOG_DEBUG, 0, "Enter");
    uint8_t nasResult[1000];
    uint32_t bsize=0;
    S1AP_Message_t *s1msg = (S1AP_Message_t *)signal->data;
    Unconstrained_Octed_String_t *nAS_in;


    if(signal->name == S1_newUEmessage){
        if(TASK_MME_S1___initUEMsg(signal)){
            /*ERROR*/
            return 0;
        }
        /*TODO @Vicent Map UE_MME_ID somewhere*/
    }else if(signal->name == NAS_data_available){
        TASK_MME_S1___TransparentNAS(signal);

    }else if(signal->name == NAS_AuthVectorAvailable){
        NAS_sessionAvailable(nasResult, &bsize, signal);
        TASK_MME_S1___Forge_TransparentNAS(nasResult, bsize, signal);
        return 0;
    }else if(signal->name == NAS_AttachReqCont){
        s1msg = (S1AP_Message_t *)signal->data;

        /*NAS-PDU */
        nAS_in = (Unconstrained_Octed_String_t*)s1ap_findIe(s1msg, id_NAS_PDU);
        if(nAS_in==NULL){
            log_msg(LOG_ERR, 0, "Uplink NAS Transport has no NAS IE");
            return 0;
        }

        memset(nasResult, 0, 1000);

        /* Process NAS PDU*/
        NAS_process(nasResult, &bsize, nAS_in->str, nAS_in->len, signal);
        TASK_MME_S1___Forge_TransparentNAS(nasResult, bsize, signal);
        signal->processTo->next_state = STATE_S1_SetupOfUEContext;
        signal->name = S1_ContinueAttach;
        return 1;
    }

    if(signal->name == S1_newUEmessage){
        return 0;       /*Store signal only the first time*/
    }else if(signal->name == NAS_AttachReqCont){

        /*Change of state the last time*/
        signal->processTo->next_state = STATE_S1_SetupOfUEContext;
        signal->name = S1_ContinueAttach;
    }
    return(0);

}

static int STATE_S1_UEContextRelease(Signal *signal){
    S1AP_Message_t *s1msg;
    uint8_t nasResult[1000];
    uint32_t bsize=0, error;

    log_msg(LOG_DEBUG, 0, "Enter");
    if (signal->name == S1_detach){
        /*MMEENDTIME*/
        if (TASK_MME_S1___Forge_UEContextReleaseCommand( signal, CauseNas, CauseNas_detach)==0){
            /* Store session, pending responses*/
            addToPendingResponse(PDATA);
        }
        return 0;
    }else if(signal->name == NAS_data_available){
        TASK_MME_S1___TransparentNAS(signal);
        return 0;
    }else if(signal->name > NAS_data_available){
        NAS_sessionAvailable(nasResult, &bsize, signal);
        TASK_MME_S1___Forge_TransparentNAS(nasResult, bsize, signal);
        return 0;
    }


    /* Process Response*/
    if(signal->data == NULL){
        log_msg(LOG_ERR, 0, "Message not found");
        return 0;
    }

    s1msg = (S1AP_Message_t *)signal->data;

    if(s1msg->pdu->procedureCode != id_UEContextRelease){
        /*ERROR*/
        return 0;
    }

    if( s1msg->choice == successful_outcome){
        error = TASK_MME_S1___Validate_UEContextReleaseComplete(signal);
        if (!error){
            /* Delete context*/
            free(PDATA->user_ctx);                  /* Delete user context*/
            free(PDATA);                            /* Delete session */
            process_stop(signal->processFrom);      /* Delete process */
        }
    }else if(s1msg->choice == unsuccessful_outcome){

    }
    return 0;
}


/* ======================================================================
 * TASK Implementations
 * ====================================================================== */


static uint8_t TASK_MME_S1___S1Setup(Signal *signal)
{
    struct t_message *msg;
    S1AP_Message_t *s1msg;
    S1_EndPoint_Info_t *epInfo;
    S1AP_PROTOCOL_IES_t* ie;

    log_msg(LOG_DEBUG, 0, "Enter TASK_MME_S1___S1Setup");

    msg = (struct t_message *)signal->data;

    s1msg = s1ap_decode((void *)msg->packet.raw, msg->length);

    /* Check Procedure*/
    if(s1msg->pdu->procedureCode != id_S1Setup && s1msg->choice != initiating_message){
        log_msg(LOG_WARNING, 0, "Not a S1-Setup Request but %s, ignoring", elementaryProcedureName[s1msg->pdu->procedureCode]);
        return 0;
    }

    /* Allocate Endpoint information structure and store the received IE*/
    PDATA->s1->info = malloc(sizeof(S1_EndPoint_Info_t));

    epInfo = PDATA->s1->info;

    epInfo->global_eNB_ID   = s1ap_getIe(s1msg, id_Global_ENB_ID);
    epInfo->eNBname         = s1ap_getIe(s1msg, id_eNBname);              /*OPTIONAL*/
    epInfo->supportedTAs     = s1ap_getIe(s1msg, id_SupportedTAs);
    epInfo->cSG_IdList      = s1ap_getIe(s1msg, id_CSG_IdList);

    CHECKIEPRESENCE(epInfo->global_eNB_ID)
    CHECKIEPRESENCE(epInfo->supportedTAs)

    s1msg->freemsg(s1msg);

    log_msg(LOG_INFO, 0, "S1-Setup : new eNB \"%s\", connection added", epInfo->eNBname->name);

    /* Forge response*/
    s1msg = S1AP_newMsg();
    s1msg->choice = successful_outcome;
    s1msg->pdu->procedureCode = id_S1Setup;
    s1msg->pdu->criticality = reject;
    s1msg->pdu->value = new_ProtocolIE_Container();

    /*SELF_ON_SIG->name->showIE(SELF_ON_SIG->name);*/

    /* MME Name (optional)*/
    if(SELF_ON_SIG->name != NULL){
        ie=newProtocolIE();
        ie->id = id_MMEname;
        ie->presence = optional;
        ie->criticality = ignore;
        ie->value = SELF_ON_SIG->name;
        ie->showValue = SELF_ON_SIG->name->showIE;
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
    ie->value = SELF_ON_SIG->servedGUMMEIs;
    ie->showValue = SELF_ON_SIG->servedGUMMEIs->showIE;
    s1msg->pdu->value->addIe(s1msg->pdu->value, ie);

    /* Relative MME Capacity*/
    ie=newProtocolIE();
    ie->id = id_RelativeMMECapacity;
    ie->presence = optional;
    ie->criticality = ignore;
    ie->value = SELF_ON_SIG->relativeCapacity;
    ie->showValue = SELF_ON_SIG->relativeCapacity->showIE;
    s1msg->pdu->value->addIe(s1msg->pdu->value, ie);

    /* Send Response*/
    s1_sendmsg(PDATA->s1, S1AP_NONUESIGNALING_STREAM0, s1msg);

    /*s1msg->showmsg(s1msg);*/

    /* This function doesn't deallocate the IE stored on the mme structure,
     * because the freeValue callback attribute of the ProtocolIE structure is NULL */
    s1msg->freemsg(s1msg);

    /*port state change to open on End Point structure*/
    PDATA->s1->portState = opened;
    return 0;
}

static uint8_t TASK_MME_S1___initUEMsg(Signal *signal)
{
    struct t_message *msg;
    S1AP_Message_t *s1msg, *s1out;
    ENB_UE_S1AP_ID_t *eNB_ID;
    Unconstrained_Octed_String_t *nASPDU;
    TAI_t *tAI;
    EUTRAN_CGI_t *eCGI;
    RRC_Establishment_Cause_t *cause;
    struct user_ctx_t *user;
    uint8_t nasResult[1000];
    uint32_t bsize;
    S1AP_PROTOCOL_IES_t* ie;
    MME_UE_S1AP_ID_t *mmeUEId;
    ENB_UE_S1AP_ID_t *eNBUEId;
    Unconstrained_Octed_String_t *nAS_PDU;


    log_msg(LOG_DEBUG, 0, "Enter TASK_MME_S1___initUEMsg");
    user = PDATA->user_ctx;

    /*msg = (struct t_message *)signal->data;

    s1msg = s1ap_decode((void *)msg->packet.raw, msg->length);*/
    s1msg = (S1AP_Message_t *)signal->data;

    /* Check Procedure*/
    if(s1msg->pdu->procedureCode != id_initialUEMessage && s1msg->choice != initiating_message){
        log_msg(LOG_WARNING, 0, "Not a initialUEMessage but %s, ignoring", elementaryProcedureName[s1msg->pdu->procedureCode]);
        return 1;
    }

    /*eNB UE S1AP ID */
    eNB_ID = (ENB_UE_S1AP_ID_t*)s1ap_findIe(s1msg, id_eNB_UE_S1AP_ID);
    if(eNB_ID != NULL){
        user->eNB_UE_S1AP_ID = eNB_ID->eNB_id;
    }
    /*NAS-PDU */
    nASPDU = (Unconstrained_Octed_String_t*)s1ap_findIe(s1msg, id_NAS_PDU);
    if(nASPDU==NULL){

    }

    /*TAI */
    tAI = (TAI_t*)s1ap_findIe(s1msg, id_TAI);
    if(tAI != NULL){
        user->tAI.MCC = tAI->pLMNidentity->MCC;
        user->tAI.MNC = tAI->pLMNidentity->MNC;
        memcpy(user->tAI.sn, tAI->pLMNidentity->tbc.s, 3);
        memcpy(&(user->tAI.tAC), tAI->tAC->s,2);
        user->tAI.tAC = ntohs(user->tAI.tAC);
    }else{

    }

    /*E-UTRAN CGI */
    eCGI = (EUTRAN_CGI_t*)s1ap_findIe(s1msg, id_EUTRAN_CGI);
    if(eCGI != NULL){
        user->ECGI.MCC = eCGI->pLMNidentity->MCC;
        user->ECGI.MNC = eCGI->pLMNidentity->MNC;
        user->ECGI.cellID = eCGI->cell_ID.id;
    }else{

    }

    /*RRC Establishment cause */
    cause = (RRC_Establishment_Cause_t*)s1ap_findIe(s1msg, id_RRC_Establishment_Cause);
    if(cause != NULL){

    }else{

    }

    /* Process NAS PDU*/
    if(PDATA->user_ctx->mME_UE_S1AP_ID==0){
        /* PDATA->user_ctx->mME_UE_S1AP_ID = getNewLocalUEid(PDATA); */
    }

    NAS_process(nasResult, &bsize, nASPDU->str, nASPDU->len, signal);
    TASK_MME_S1___Forge_TransparentNAS(nasResult, bsize, signal);
    return 0;
}

static uint8_t TASK_MME_S1___Forge_InitCtxSetupReq(Signal *signal){
    /* Forge Initial Context Setup Request*/
    S1AP_Message_t *s1msg;
    S1AP_PROTOCOL_IES_t* ie;
    MME_UE_S1AP_ID_t *mmeUEId;
    ENB_UE_S1AP_ID_t *eNBUEId;
    UEAggregateMaximumBitrate_t *bitrate;
    E_RABToBeSetupListCtxtSUReq_t *eRABlist;
    E_RABToBeSetupItemCtxtSUReq_t *eRABitem;
    UESecurityCapabilities_t *sec;
    SecurityKey_t *key;
    uint8_t nAS_buffer[500];

    log_msg(LOG_DEBUG, 0, "Enter");


    s1msg = S1AP_newMsg();
    s1msg->choice = initiating_message;
    s1msg->pdu->procedureCode = id_InitialContextSetup;
    s1msg->pdu->criticality = reject;

    /* MME-UE-S1AP-ID*/
    ie=newProtocolIE();
    if(ie == NULL){
        log_msg(LOG_ERR, 0, "S1AP: Coudn't allocate new Protocol IE structure");
        return 1;
    }
    mmeUEId = new_MME_UE_S1AP_ID();
    ie->value = mmeUEId;
    ie->showValue = mmeUEId->showIE;
    ie->freeValue = mmeUEId->freeIE;
    s1msg->pdu->value->addIe(s1msg->pdu->value, ie);
    ie->id = id_MME_UE_S1AP_ID;
    ie->presence = mandatory;
    ie->criticality = reject;
    if(PDATA->user_ctx->mME_UE_S1AP_ID==0){
        log_msg(LOG_ERR, 0, "S1AP: MME UE S1AP ID not available");
        return 1;
    }
    mmeUEId->mme_id = PDATA->user_ctx->mME_UE_S1AP_ID;

    /* eNB-UE-S1AP-ID*/
    ie=newProtocolIE();
    if(ie == NULL){
        log_msg(LOG_ERR, 0, "S1AP: Coudn't allocate new Protocol IE structure");
        return 1;
    }
    eNBUEId = new_ENB_UE_S1AP_ID();
    ie->value = eNBUEId;
    ie->showValue = eNBUEId->showIE;
    ie->freeValue = eNBUEId->freeIE;
    s1msg->pdu->value->addIe(s1msg->pdu->value, ie);
    ie->id = id_eNB_UE_S1AP_ID;
    ie->presence = optional;
    ie->criticality = reject;
    eNBUEId->eNB_id = PDATA->user_ctx->eNB_UE_S1AP_ID;

    /* uEaggregateMaximumBitrate*/ /* HSS*/
    ie=newProtocolIE();
    bitrate = new_UEAggregateMaximumBitrate();
    ie->value = bitrate;
    ie->showValue = bitrate->showIE;
    ie->freeValue = bitrate->freeIE;
    s1msg->pdu->value->addIe(s1msg->pdu->value, ie);
    ie->id = id_uEaggregateMaximumBitrate;
    ie->presence = optional;
    ie->criticality = reject;
    bitrate->uEaggregateMaximumBitRateDL.rate = PDATA->user_ctx->ue_ambr_dl;
    bitrate->uEaggregateMaximumBitRateUL.rate = PDATA->user_ctx->ue_ambr_ul;

    /* E-RABToBeSetupListCtxtSUReq*/
    ie=newProtocolIE();
    eRABlist = new_E_RABToBeSetupListCtxtSUReq();
    ie->value = eRABlist;
    ie->showValue = eRABlist->showIE;
    ie->freeValue = eRABlist->freeIE;
    s1msg->pdu->value->addIe(s1msg->pdu->value, ie);
    ie->id = id_E_RABToBeSetupListCtxtSUReq;
    ie->presence = optional;
    ie->criticality = reject;

    ie = newProtocolIE();
    eRABitem = new_E_RABToBeSetupItemCtxtSUReq();
    ie->value = eRABitem;
    ie->showValue = eRABitem->showIE;
    ie->freeValue = eRABitem->freeIE;
    eRABlist->additem(eRABlist, ie);
    eRABitem->eRABlevelQoSParameters = new_E_RABLevelQoSParameters();
    eRABitem->transportLayerAddress = new_TransportLayerAddress();
    eRABitem->nAS_PDU = new_Unconstrained_Octed_String();
    eRABitem->nAS_PDU->str = nAS_buffer;
    eRABitem->eRABlevelQoSParameters->allocationRetentionPriority = new_AllocationAndRetentionPriority();
    /*eRABitem->eRABlevelQoSParameters->gbrQosInformation = new_GBR_QosInformation();*/
    ie->id = id_E_RABToBeSetupItemCtxtSUReq;
    ie->presence = optional;
    ie->criticality = reject;
    eRABitem->eRAB_ID.id = PDATA->user_ctx->ebearer[0].id;
    eRABitem->eRABlevelQoSParameters->qCI = PDATA->user_ctx->ebearer[0].qos.qci;
    eRABitem->eRABlevelQoSParameters->allocationRetentionPriority->priorityLevel=PDATA->user_ctx->ebearer[0].qos.pl;
    eRABitem->eRABlevelQoSParameters->allocationRetentionPriority->pre_emptionCapability = PDATA->user_ctx->ebearer[0].qos.pci;
    eRABitem->eRABlevelQoSParameters->allocationRetentionPriority->pre_emptionVulnerability = PDATA->user_ctx->ebearer[0].qos.pvi;

    /*eRABitem->eRABlevelQoSParameters->gbrQosInformation;*/
    memcpy(eRABitem->transportLayerAddress->addr, &(PDATA->user_ctx->ebearer[0].s1u_sgw.addr.addrv4), sizeof(uint32_t));
    eRABitem->transportLayerAddress->len = 32;
    memcpy(eRABitem->gTP_TEID.teid, &(PDATA->user_ctx->ebearer[0].s1u_sgw.teid), sizeof(uint32_t));

    /* UE Security Capabilities */
    ie=newProtocolIE();
    sec = new_UESecurityCapabilities();;
    ie->value = sec;
    ie->showValue = sec->showIE;
    ie->freeValue = sec->freeIE;
    s1msg->pdu->value->addIe(s1msg->pdu->value, ie);
    ie->id = id_UESecurityCapabilities;
    ie->presence = optional;
    ie->criticality = reject;
    sec->encryptionAlgorithms.v = 0;
    sec->integrityProtectionAlgorithms.v = 0;

    /* SecurityKey */
    ie=newProtocolIE();
    key = new_SecurityKey();;
    ie->value = key;
    ie->showValue = key->showIE;
    ie->freeValue = key->freeIE;
    s1msg->pdu->value->addIe(s1msg->pdu->value, ie);
    ie->id = id_SecurityKey;
    ie->presence = optional;
    ie->criticality = reject;
    /*memset(&key->key, 0, 32);*/
    memcpy(&key->key, PDATA->user_ctx->sec_ctx.keNB, 32);

    NAS_sessionAvailable(eRABitem->nAS_PDU->str, &(eRABitem->nAS_PDU->len), signal);
    if(eRABitem->nAS_PDU->str != NULL){
        eRABitem->opt |=0x80;
        PDATA->user_ctx->sec_ctx.dlNAScnt++;
    }

    /* Send Response*/
    s1_sendmsg(PDATA->s1, PDATA->sid, s1msg);

    s1msg->freemsg(s1msg);

    /* Store session, pending responses*/
    addToPendingResponse(PDATA);
    return 0;

}

static uint8_t TASK_MME_S1___Validate_InitCtxSetupRes(Signal *signal){
    MME_UE_S1AP_ID_t *mme_id;
    ENB_UE_S1AP_ID_t *enb_id;
    E_RABSetupListCtxtSURes_t *list;
    E_RABSetupItemCtxtSURes_t *item;
    S1AP_Message_t *s1msg = (S1AP_Message_t *)signal->data;
    struct user_ctx_t *user = PDATA->user_ctx;

    log_msg(LOG_DEBUG, 0, "Enter");


    mme_id = s1ap_findIe(s1msg, id_MME_UE_S1AP_ID);
    enb_id = s1ap_findIe(s1msg, id_eNB_UE_S1AP_ID);

    if(mme_id->mme_id != PDATA->user_ctx->mME_UE_S1AP_ID || enb_id->eNB_id != PDATA->user_ctx->eNB_UE_S1AP_ID){
        log_msg(LOG_ERR, 0, "MME_UE_S1AP_ID or eNB_UE_S1AP_ID are not the expected ones");
        return 1;
    }

    list = s1ap_findIe(s1msg, id_E_RABSetupListCtxtSURes);
    item = list->item[0]->value;
    if (user->ebearer[0].id != item->eRAB_ID.id){
        log_msg(LOG_ERR, 0, "Bearer ID not expected");
        return 1;
    }
    memcpy(&(user->ebearer[0].s1u_eNB.teid), &(item->gTP_TEID.teid), 4);
    if (item->transportLayerAddress->len == 32){
        user->ebearer[0].s1u_eNB.ipv4=1;
        memcpy(&(user->ebearer[0].s1u_eNB.addr.addrv4), &(item->transportLayerAddress->addr), 4);
    }else{
        log_msg(LOG_ERR, 0, "Only IPv4 implemented, len %u, %x", item->transportLayerAddress->len);
    }

    user->ebearer[0].s1u_eNB.iface = hton8(S1U_eNB);
    return 0;
}

static uint8_t TASK_MME_S1___Forge_TransparentNAS(uint8_t *nasResult, uint32_t bsize, Signal *signal){
    S1AP_Message_t *s1out;

    MME_UE_S1AP_ID_t *mmeUEId;
    ENB_UE_S1AP_ID_t *eNBUEId;
    Unconstrained_Octed_String_t *nAS_PDU;

    if(bsize != 0 && nasResult != NULL){
        s1out = S1AP_newMsg();
        s1out->choice = initiating_message;
        s1out->pdu->procedureCode = id_downlinkNASTransport;
        s1out->pdu->criticality = ignore;

        /* MME-UE-S1AP-ID*/
        if(PDATA->user_ctx->mME_UE_S1AP_ID==0){
            PDATA->user_ctx->mME_UE_S1AP_ID = 1;
        }
        mmeUEId = s1ap_newIE(s1out, id_MME_UE_S1AP_ID, mandatory, reject);
        mmeUEId->mme_id = PDATA->user_ctx->mME_UE_S1AP_ID;

        /* eNB-UE-S1AP-ID*/
        eNBUEId = s1ap_newIE(s1out, id_eNB_UE_S1AP_ID, mandatory, reject);
        eNBUEId->eNB_id = PDATA->user_ctx->eNB_UE_S1AP_ID;

        /* NAS-PDU*/
        nAS_PDU = s1ap_newIE(s1out, id_NAS_PDU, mandatory, reject);
        nAS_PDU->len = bsize;
        nAS_PDU->str = nasResult;

        /*s1out->showmsg(s1out);*/
        s1_sendmsg(PDATA->s1, PDATA->sid, s1out);
        s1out->freemsg(s1out);
        return 0;
    }

    return 0;
}

static uint8_t TASK_MME_S1___TransparentNAS(Signal *signal){
    struct t_message *msg;
    S1AP_Message_t *s1msg, *s1out;
    ENB_UE_S1AP_ID_t *eNB_ID;

    struct user_ctx_t *user;
    uint8_t nasResult[1000];
    uint32_t bsize=0;
    MME_UE_S1AP_ID_t *mmeUEId;
    ENB_UE_S1AP_ID_t *eNBUEId;
    Unconstrained_Octed_String_t *nAS_PDU, *nAS_in;

    user = PDATA->user_ctx;
    s1msg = (S1AP_Message_t *)signal->data;

    /*NAS-PDU */
    nAS_in = (Unconstrained_Octed_String_t*)s1ap_findIe(s1msg, id_NAS_PDU);
    if(nAS_in==NULL){
        log_msg(LOG_ERR, 0, "Uplink NAS Transport has no NAS IE");
        return 0;
    }

    memset(nasResult, 0, 1000);

    /* Process NAS PDU*/
    NAS_process(nasResult, &bsize, nAS_in->str, nAS_in->len, signal);
    TASK_MME_S1___Forge_TransparentNAS(nasResult, bsize, signal);

    return 0;
}

static uint8_t TASK_MME_S1___Forge_UEContextReleaseCommand(Signal *signal, cause_choice_t choice, uint32_t cause){
    S1AP_Message_t *s1msg;
    UE_S1AP_IDs_t *ue_ids;
    Cause_t *c;
    struct EndpointStruct_t* ep_S1;
    uint32_t eNB_id;
    uint32_t mme_id = PDATA->user_ctx->mME_UE_S1AP_ID;

    log_msg(LOG_DEBUG, 0, "Enter");

    if(cause == CauseNas_detach && choice == CauseNas){
        ep_S1 =  PDATA->s1;
        eNB_id = PDATA->user_ctx->eNB_UE_S1AP_ID;

    }else if(cause == CauseRadioNetwork_successful_handover && choice == CauseRadioNetwork){
        ep_S1 = &(PDATA->user_ctx->hoCtx.source_s1);
        eNB_id = PDATA->user_ctx->hoCtx.source_eNB_id;
    }

    /*MMEENDTIME*/
    s1msg = S1AP_newMsg();
    s1msg->choice = initiating_message;
    s1msg->pdu->procedureCode = id_UEContextRelease;
    s1msg->pdu->criticality = reject;
    /*MMEMEASUREPROCTIME*(
    /* id-UE-S1AP-IDs */
    ue_ids = s1ap_newIE(s1msg, id_UE_S1AP_IDs, mandatory, reject);
    ue_ids->choice = 0;
    ue_ids->uE_S1AP_ID.uE_S1AP_ID_pair = new_UE_S1AP_ID_pair();
    ue_ids->uE_S1AP_ID.uE_S1AP_ID_pair->eNB_UE_S1AP_ID->eNB_id = eNB_id;
    ue_ids->uE_S1AP_ID.uE_S1AP_ID_pair->mME_UE_S1AP_ID->mme_id = mme_id;
    /*MMEMEASUREPROCTIME*/
    /* id-Cause */
    c = s1ap_newIE(s1msg, id_Cause, mandatory, ignore);
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
    /*MMEMEASUREPROCTIME*/
    /* Send Response*/
    s1_sendmsg(ep_S1, PDATA->sid, s1msg);
    /*MMEMEASUREPROCTIME*/
    s1msg->freemsg(s1msg);
    return 0;
}

static uint8_t TASK_MME_S1___Validate_UEContextReleaseComplete(Signal *signal){
    S1AP_Message_t *s1msg;
    MME_UE_S1AP_ID_t *mme_id;
    ENB_UE_S1AP_ID_t *enb_id;
    s1msg = (S1AP_Message_t *)signal->data;

    mme_id = s1ap_findIe(s1msg, id_MME_UE_S1AP_ID);
    enb_id = s1ap_findIe(s1msg, id_eNB_UE_S1AP_ID);

    if(mme_id->mme_id == PDATA->user_ctx->mME_UE_S1AP_ID && enb_id->eNB_id == PDATA->user_ctx->eNB_UE_S1AP_ID){
        return 0;
    }
    return 1;
}

static uint8_t TASK_MME_S1___Validate_PathSwitchRequest(Signal *signal){
    MME_UE_S1AP_ID_t *mme_id;
    ENB_UE_S1AP_ID_t *enb_id;
    E_RABSetupListCtxtSURes_t *list;
    E_RABSetupItemCtxtSURes_t *item;
    S1AP_Message_t *s1msg = (S1AP_Message_t *)signal->data;
    struct user_ctx_t *user = PDATA->user_ctx;

    log_msg(LOG_DEBUG, 0, "Enter");


    mme_id = s1ap_findIe(s1msg, id_SourceMME_UE_S1AP_ID);
    enb_id = s1ap_findIe(s1msg, id_eNB_UE_S1AP_ID);

    if(mme_id->mme_id != PDATA->user_ctx->mME_UE_S1AP_ID){
        log_msg(LOG_ERR, 0, "Source MME-UE-S1AP-ID is not the expected one. (%u != %u)", mme_id->mme_id, PDATA->user_ctx->mME_UE_S1AP_ID);
        return 1;
    }

    PDATA->user_ctx->mME_UE_S1AP_ID = mme_id->mme_id;
    PDATA->user_ctx->eNB_UE_S1AP_ID = enb_id->eNB_id;

    list = s1ap_findIe(s1msg, id_E_RABToBeSwitchedDLList);
    item = list->item[0]->value;
    if (user->ebearer[0].id != item->eRAB_ID.id){
        log_msg(LOG_ERR, 0, "Bearer ID not expected");
        return 1;
    }

    /* Store source parameters*/
    user->hoCtx.source_eNB_id = enb_id->eNB_id;
    memcpy(&(user->hoCtx.source_s1), PDATA->s1, sizeof(struct EndpointStruct_t));
    memcpy(user->hoCtx.old_ebearers, user->ebearer, sizeof(Bearer_Ctx_t));
    /* Store current parameters*/
    memcpy(&(user->ebearer[0].s1u_eNB.teid), &(item->gTP_TEID.teid), 4);
    if (item->transportLayerAddress->len == 32){
        user->ebearer[0].s1u_eNB.ipv4=1;
        memcpy(&(user->ebearer[0].s1u_eNB.addr.addrv4), &(item->transportLayerAddress->addr), 4);
    }else{
        log_msg(LOG_ERR, 0, "Only IPv4 implemented, len %u, %x", item->transportLayerAddress->len);
    }

    return 0;
}

static uint8_t TASK_MME_S1___Forge_PathSwitchFailure(Signal *signal){
    log_msg(LOG_DEBUG, 0, "Enter");

    return 0;
}

static uint8_t TASK_MME_S1___Forge_PathSwitchAck(Signal *signal){

    S1AP_Message_t *s1msg;
    S1AP_PROTOCOL_IES_t* ie;
    MME_UE_S1AP_ID_t *mmeUEId;
    ENB_UE_S1AP_ID_t *eNBUEId;
    UEAggregateMaximumBitrate_t *bitrate;
    E_RABSetupListBearerSURes_t *eRABlist;
    E_RABSetupItemBearerSURes_t *eRABitem;
    SecurityContext_t *sec;

    log_msg(LOG_DEBUG, 0, "Enter");

    struct user_ctx_t *user = PDATA->user_ctx;

    log_msg(LOG_DEBUG, 0, "Enter");

    s1msg = S1AP_newMsg();
    s1msg->choice = successful_outcome;
    s1msg->pdu->procedureCode = id_PathSwitchRequest;
    s1msg->pdu->criticality = reject;

    /* MME-UE-S1AP-ID*/
    mmeUEId = s1ap_newIE(s1msg, id_MME_UE_S1AP_ID, mandatory, ignore);
    mmeUEId->mme_id = PDATA->user_ctx->mME_UE_S1AP_ID;

    /* eNB-UE-S1AP-ID*/
    eNBUEId = s1ap_newIE(s1msg, id_eNB_UE_S1AP_ID, mandatory, ignore);
    eNBUEId->eNB_id = PDATA->user_ctx->eNB_UE_S1AP_ID;

    /* uEaggregateMaximumBitrate*/ /* HSS*/
    bitrate = s1ap_newIE(s1msg, id_uEaggregateMaximumBitrate, optional, ignore);
    bitrate->uEaggregateMaximumBitRateDL.rate = user->ue_ambr_dl;
    bitrate->uEaggregateMaximumBitRateUL.rate = user->ue_ambr_ul;

    /* E-RAB To Be Switched in Uplink List */
    eRABlist = s1ap_newIE(s1msg, id_E_RABToBeSwitchedULList, optional, ignore);

    ie = newProtocolIE();
    eRABitem = new_E_RABSetupItemBearerSURes();
    ie->value = eRABitem;
    ie->showValue = eRABitem->showIE;
    ie->freeValue = eRABitem->freeIE;
    eRABlist->additem(eRABlist, ie);
    eRABitem->transportLayerAddress = new_TransportLayerAddress();
    ie->id = id_E_RABToBeSwitchedULItem;
    ie->presence = optional;
    ie->criticality = ignore;
    eRABitem->eRAB_ID.id = PDATA->user_ctx->ebearer[0].id;

    /*eRABitem->eRABlevelQoSParameters->gbrQosInformation;*/
    memcpy(eRABitem->transportLayerAddress->addr, &(PDATA->user_ctx->ebearer[0].s1u_sgw.addr.addrv4), sizeof(uint32_t));
    eRABitem->transportLayerAddress->len = 32;
    memcpy(eRABitem->gTP_TEID.teid, &(PDATA->user_ctx->ebearer[0].s1u_sgw.teid), sizeof(uint32_t));

    /*id-SecurityContext */
    refresh_NH(&(user->sec_ctx));
    sec = s1ap_newIE(s1msg, id_SecurityContext, mandatory, reject);
    sec->nextHopChainingCount = user->sec_ctx.ncc;
    sec->nextHopParameter = new_SecurityKey();
    memcpy(sec->nextHopParameter->key, user->sec_ctx.nh, 32);


    /* Send Response*/
    s1_sendmsg(PDATA->s1, PDATA->sid, s1msg);

    s1msg->freemsg(s1msg);

    return 0;
}

struct EndpointStruct_t *get_ep_with_GlobalID(S1_t *self, TargeteNB_ID_t *t){
    GHashTableIter iter;
    gpointer v;
    struct EndpointStruct_t* ep;
    gboolean found = FALSE;
    Global_ENB_ID_t *id1, *id2;

    id1 = t->global_ENB_ID;

    g_hash_table_iter_init (&iter, self->assocs);

    while (g_hash_table_iter_next (&iter, NULL, &v)){
        ep = (struct EndpointStruct_t*)v;
        id2 = ((S1_EndPoint_Info_t*)ep->info)->global_eNB_ID;

        if( memcmp(id1->pLMNidentity->tbc.s, id2->pLMNidentity->tbc.s, 3) ==0 &&
            memcmp(id1->eNBid, id2->eNBid, sizeof(ENB_ID_t)) == 0 ){
            found = TRUE;
            break;
        }
    }

    if (found)
        return ep;
    else
        return NULL;
}

static uint8_t TASK_MME_S1___Validate_HandoverRequired(Signal *signal){
    S1AP_Message_t *s1msg;
    MME_UE_S1AP_ID_t *mme_id;
    ENB_UE_S1AP_ID_t *enb_id;
    HandoverType_t  *type;
    Cause_t         *cause;
    TargetID_t     *target;
    TargeteNB_ID_t  *targeteNB;
    Direct_Forwarding_Path_Availability_t *forward;

    s1msg = (S1AP_Message_t *)signal->data;
    int i, ep_index=-1;

    log_msg(LOG_DEBUG, 0, "Enter");

    mme_id = s1ap_findIe(s1msg, id_MME_UE_S1AP_ID);
    enb_id = s1ap_findIe(s1msg, id_eNB_UE_S1AP_ID);

    if(mme_id->mme_id != PDATA->user_ctx->mME_UE_S1AP_ID || enb_id->eNB_id != PDATA->user_ctx->eNB_UE_S1AP_ID){
        return 1;
    }

    type = s1ap_findIe(s1msg, id_HandoverType);
    CHECKIEPRESENCE(type)
    if (type->ext != 0 || type->ht.noext != HandoverType_intralte){
        log_msg(LOG_ERR, 0, "Handover type not implemented");
        return 1;
    }

    cause = s1ap_findIe(s1msg, id_Cause);
    CHECKIEPRESENCE(cause)

    forward = s1ap_findIe(s1msg, id_Direct_Forwarding_Path_Availability);
    if(forward == NULL){
        log_msg(LOG_ERR, 0, "No direct Path Available. Indirect forwarding not implemented");
        return 1;
    }

    target = s1ap_findIe(s1msg, id_TargetID);
    CHECKIEPRESENCE(target)
    if(target->choice != 0){
        log_msg(LOG_ERR, 0, "Target ID message is not an eNB as expected");
        return 1;
    }

    targeteNB = target->targetID.targeteNB_ID;

    /** @TODO WRONG!!!*/
    PDATA->user_ctx->hoCtx.target_s1 = get_ep_with_GlobalID(SELF_ON_SIG, targeteNB);

    if(PDATA->user_ctx->hoCtx.target_s1 == NULL){
        log_msg(LOG_ERR, 0, "Target endpoint not available on this MME. MME relocation not implemented.");
        return 1;
    }

    log_msg(LOG_DEBUG, 0, "Target ENBid  %x endpoint found",
            ((S1_EndPoint_Info_t*)PDATA->user_ctx->hoCtx.target_s1->info)->global_eNB_ID->eNBid->id.macroENB_ID);

    /* Store source parameters*/
    PDATA->user_ctx->hoCtx.source_eNB_id = enb_id->eNB_id;
    memcpy(&(PDATA->user_ctx->hoCtx.source_s1), PDATA->s1, sizeof(struct EndpointStruct_t));
    memcpy(PDATA->user_ctx->hoCtx.old_ebearers, PDATA->user_ctx->ebearer, sizeof(Bearer_Ctx_t));
    log_msg(LOG_DEBUG, 0, "Stored source parameters: source UE-eNB-S1AP %u", PDATA->user_ctx->hoCtx.source_eNB_id);


    return 0;
}

static uint8_t TASK_MME_S1___Forge_HandoverReq(Signal *signal){
    S1AP_Message_t *handoverrequiredMSG, *s1msg;
    MME_UE_S1AP_ID_t *mmeUEId;
    ENB_UE_S1AP_ID_t *eNBUEId;
    E_RABToBeSetupListHOReq_t   *eRABlist;
    E_RABToBeSetupItemHOReq_t   *eRABitem;
    S1AP_PROTOCOL_IES_t         *ie;
    Unconstrained_Octed_String_t *container;
    UESecurityCapabilities_t    *secCap;
    SecurityContext_t           *secCtx;
    UEAggregateMaximumBitrate_t *bitrate;
    HandoverType_t  *type;
    Cause_t         *cause;

    handoverrequiredMSG = (S1AP_Message_t *)signal->data;
    struct user_ctx_t *user = PDATA->user_ctx;

    log_msg(LOG_DEBUG, 0, "Enter");

    container = s1ap_findIe(handoverrequiredMSG, id_Source_ToTarget_TransparentContainer);
    CHECKIEPRESENCE(container)

    s1msg = S1AP_newMsg();
    s1msg->choice = initiating_message;
    s1msg->pdu->procedureCode = id_HandoverResourceAllocation;
    s1msg->pdu->criticality = reject;

    /* MME-UE-S1AP-ID*/
    mmeUEId = s1ap_newIE(s1msg, id_MME_UE_S1AP_ID, mandatory, ignore);
    mmeUEId->mme_id = PDATA->user_ctx->mME_UE_S1AP_ID;

    /* Handover Type*/
    type = s1ap_findIe(handoverrequiredMSG, id_HandoverType);
    s1ap_setValueOnNewIE(s1msg, id_HandoverType, mandatory, reject, (GenericVal_t*)type);

    /* Cause*/
    cause = s1ap_findIe(handoverrequiredMSG, id_Cause);
    s1ap_setValueOnNewIE(s1msg, id_Cause, mandatory, ignore, (GenericVal_t*)cause);

    /* UE Aggregate Maximum Bit Rate */
    bitrate = s1ap_newIE(s1msg, id_uEaggregateMaximumBitrate, mandatory, reject);
    bitrate->uEaggregateMaximumBitRateDL.rate = user->ue_ambr_dl;
    bitrate->uEaggregateMaximumBitRateUL.rate = user->ue_ambr_ul;

    /* E-RABs To Be Setup List */
    eRABlist = s1ap_newIE(s1msg, id_E_RABToBeSetupListHOReq, mandatory, reject);

    ie = newProtocolIE();
    eRABitem = new_E_RABToBeSetupItemHOReq();
    ie->value = eRABitem;
    ie->showValue = eRABitem->showIE;
    ie->freeValue = eRABitem->freeIE;
    eRABlist->additem(eRABlist, ie);
    eRABitem->eRABLevelQoSParameters = new_E_RABLevelQoSParameters();
    eRABitem->transportLayerAddress = new_TransportLayerAddress();
    eRABitem->eRABLevelQoSParameters->allocationRetentionPriority = new_AllocationAndRetentionPriority();
    ie->id = id_E_RABToBeSetupItemHOReq;
    ie->presence = optional;
    ie->criticality = reject;

    eRABitem->eRAB_ID.id = PDATA->user_ctx->ebearer[0].id;

    memcpy(eRABitem->transportLayerAddress->addr, &(PDATA->user_ctx->ebearer[0].s1u_sgw.addr.addrv4), sizeof(uint32_t));
    eRABitem->transportLayerAddress->len = 32;

    memcpy(eRABitem->gTP_TEID.teid, &(PDATA->user_ctx->ebearer[0].s1u_sgw.teid), sizeof(uint32_t));

    eRABitem->eRABLevelQoSParameters->qCI = PDATA->user_ctx->ebearer[0].qos.qci;
    eRABitem->eRABLevelQoSParameters->allocationRetentionPriority->priorityLevel=PDATA->user_ctx->ebearer[0].qos.pl;
    eRABitem->eRABLevelQoSParameters->allocationRetentionPriority->pre_emptionCapability = shall_not_trigger_pre_emption;
    eRABitem->eRABLevelQoSParameters->allocationRetentionPriority->pre_emptionVulnerability = not_pre_emptable;
    /*GBR_QosInformation_t                *gbrQosInformation; OPTIONAL*/

    /* Source to Target Transparent Container*/
    s1ap_setValueOnNewIE(s1msg, id_Source_ToTarget_TransparentContainer, mandatory, ignore, (GenericVal_t*)container);

    /* UE Security Capabilities */
    secCap = s1ap_newIE(s1msg, id_UESecurityCapabilities, mandatory, reject);
    secCap->encryptionAlgorithms.v = 0;
    secCap->integrityProtectionAlgorithms.v = 0;

    /* Security Context  */
    refresh_NH(&(user->sec_ctx));
    secCtx = s1ap_newIE(s1msg, id_SecurityContext, mandatory, reject);
    secCtx->nextHopChainingCount = user->sec_ctx.ncc;
    secCtx->nextHopParameter = new_SecurityKey();
    memcpy(secCtx->nextHopParameter->key, user->sec_ctx.nh, 32);


    /* Send Response*/
    s1_sendmsg(PDATA->user_ctx->hoCtx.target_s1, PDATA->sid, s1msg);

    s1msg->freemsg(s1msg);

    addToPendingResponse(PDATA);

    return 0;
}

static uint8_t TASK_MME_S1___Validate_HandoverReqAck(Signal *signal){
    S1AP_Message_t *s1msg;
    MME_UE_S1AP_ID_t *mme_id;
    ENB_UE_S1AP_ID_t *enb_id;
    E_RABAdmittedList_t *admList;
    E_RABAdmittedItem_t *admItem;
    Unconstrained_Octed_String_t *container;

    struct user_ctx_t   *user = PDATA->user_ctx;
    s1msg = (S1AP_Message_t *)signal->data;

    mme_id = s1ap_findIe(s1msg, id_MME_UE_S1AP_ID);
    if(mme_id->mme_id != PDATA->user_ctx->mME_UE_S1AP_ID){
        return 1;
    }

    enb_id = s1ap_findIe(s1msg, id_eNB_UE_S1AP_ID);

    user->hoCtx.target_eNB_id = enb_id->eNB_id;

    /* E-RABs Subject to Forwarding List */
    admList = s1ap_findIe(s1msg, id_E_RABAdmittedList);
    admItem = (E_RABAdmittedItem_t*)admList->item[0]->value;
    memcpy(&(user->hoCtx.eRAB_ID), &(admItem->eRAB_ID), sizeof(E_RAB_ID_t));
    memcpy(&(user->hoCtx.GTP_TEID), &(admItem->gTP_TEID), sizeof(GTP_TEID_t));
    memcpy(&(user->hoCtx.transportLayerAddress), admItem->transportLayerAddress,
           sizeof(TransportLayerAddress_t));
    if (admItem->opt & 0x80 ){
        memcpy(&(user->hoCtx.dL_Forward_transportLayerAddress),
               admItem->dL_transportLayerAddress,
               sizeof(TransportLayerAddress_t));
    }
    if (admItem->opt & 0x40) {
        memcpy(&(user->hoCtx.dL_Forward_GTP_TEID), &(admItem->dL_GTP_TEID),
               sizeof(GTP_TEID_t));
    }
    if (admItem->opt & 0x20){
        memcpy(&(user->hoCtx.uL_Forward_transportLayerAddress),
               admItem->uL_transportLayerAddress,
               sizeof(TransportLayerAddress_t));
    }
    if (admItem->opt & 0x10){
        memcpy(&(user->hoCtx.uL_Forward_GTP_TEID), &(admItem->uL_GTP_TEID),
               sizeof(GTP_TEID_t));
    }

    /* id_Target_ToSource_TransparentContainer */
    container = s1ap_findIe(s1msg, id_Target_ToSource_TransparentContainer);
    CHECKIEPRESENCE(container)
    user->hoCtx.target2sourceTransparentContainer.len = container->len;
    user->hoCtx.target2sourceTransparentContainer.str = malloc(container->len);
    memcpy(user->hoCtx.target2sourceTransparentContainer.str, container->str,
           container->len);

    return 0;
}

static uint8_t TASK_MME_S1___Forge_HandoverCommand(Signal *signal){
    S1AP_Message_t *s1msg;
    MME_UE_S1AP_ID_t *mmeUEId;
    ENB_UE_S1AP_ID_t *eNBUEId;
    Unconstrained_Octed_String_t *container;
    E_RABDataForwardingItem_t *item;
    E_RABSubjecttoDataForwardingList_t *list;
    S1AP_PROTOCOL_IES_t *ie;

    HandoverType_t  *type;

    struct user_ctx_t *user = PDATA->user_ctx;

    log_msg(LOG_DEBUG, 0, "Enter");

    s1msg = S1AP_newMsg();
    s1msg->choice = successful_outcome;
    s1msg->pdu->procedureCode = id_HandoverPreparation;
    s1msg->pdu->criticality = reject;

    /* MME-UE-S1AP-ID*/
    mmeUEId = s1ap_newIE(s1msg, id_MME_UE_S1AP_ID, mandatory, ignore);
    mmeUEId->mme_id = PDATA->user_ctx->mME_UE_S1AP_ID;

    /* eNB-UE-S1AP-ID*/
    eNBUEId = s1ap_newIE(s1msg, id_eNB_UE_S1AP_ID, mandatory, ignore);
    eNBUEId->eNB_id = PDATA->user_ctx->eNB_UE_S1AP_ID;

    /* Handover Type*/
    type = s1ap_newIE(s1msg, id_HandoverType, mandatory, reject);
    type->ht.noext = HandoverType_intralte;

    /* E-RABs Subject to Forwarding List */
    list = s1ap_newIE(s1msg, id_E_RABSubjecttoDataForwardingList, mandatory, ignore);
    ie = newProtocolIE();
    item = new_E_RABDataForwardingItem();

    ie->value = item;
    ie->showValue = item->showIE;
    ie->freeValue = item->freeIE;
    ie->value = item;
    ie->id = id_E_RABDataForwardingItem;
    ie->presence = mandatory;
    ie->criticality = reject;

    list->additem(list, ie);

    item->opt = 0xC0;
    item->eRAB_ID.id = user->hoCtx.eRAB_ID.id;
    /* Direct Forwarding. Cofigure downlink on Source eNB*/
    memcpy(item->dL_GTP_TEID.teid, &(user->hoCtx.dL_Forward_GTP_TEID.teid), 4*sizeof(uint8_t));
    item->dL_transportLayerAddress = new_TransportLayerAddress();
    item->dL_transportLayerAddress->len = user->hoCtx.dL_Forward_transportLayerAddress.len;
    memcpy(item->dL_transportLayerAddress->addr, &(user->hoCtx.dL_Forward_transportLayerAddress.addr), user->hoCtx.dL_Forward_transportLayerAddress.len/8);

    /* Target to Source Transparent Container  */
    /*container = s1ap_newIE(s1msg, id_Target_ToSource_TransparentContainer, mandatory, ignore);
    container->len = user->hoCtx.target2sourceTransparentContainer.len;
    container->str = user->hoCtx.target2sourceTransparentContainer.str;*/
    s1ap_setValueOnNewIE(s1msg, id_Target_ToSource_TransparentContainer, mandatory, ignore, (GenericVal_t*)&(user->hoCtx.target2sourceTransparentContainer));


    /* Send Response*/
    s1_sendmsg(PDATA->s1, PDATA->sid, s1msg);

    s1msg->freemsg(s1msg);

    return 0;
}

static uint8_t TASK_MME_S1___Replay_StatusTransfer(Signal *signal){
    MME_UE_S1AP_ID_t *mme_id;
    ENB_UE_S1AP_ID_t *eNBUEId;

    struct user_ctx_t *user = PDATA->user_ctx;
    S1AP_Message_t *s1msg = (S1AP_Message_t *)signal->data;


    mme_id = s1ap_findIe(s1msg, id_MME_UE_S1AP_ID);
    eNBUEId = s1ap_findIe(s1msg, id_eNB_UE_S1AP_ID);

    if(mme_id==NULL){
        log_msg(LOG_WARNING, 0, "MME_UE_S1AP_ID not found ignoring message");
        return 1;
    }

    if(eNBUEId==NULL){
        log_msg(LOG_WARNING, 0, "ENB_UE_S1AP_ID not found ignoring message");
        return 1;
    }

    if(mme_id->mme_id != user->mME_UE_S1AP_ID && eNBUEId->eNB_id != user->eNB_UE_S1AP_ID){
        log_msg(LOG_ERR, 0, "Invalid eNB UE ID %u or MME UE ID %u", eNBUEId->eNB_id, mme_id->mme_id);
        return 1;
    }

    eNBUEId->eNB_id = user->hoCtx.target_eNB_id;

    s1msg->pdu->procedureCode = id_MMEStatusTransfer;

    /* Send Response*/
    s1_sendmsg(user->hoCtx.target_s1, PDATA->sid, s1msg);

    return 0;
}

static uint8_t TASK_MME_S1___Validate_HandoverNotify(Signal *signal){
    S1AP_Message_t      *s1msg;
    MME_UE_S1AP_ID_t    *mme_id;
    ENB_UE_S1AP_ID_t    *enb_id;
    EUTRAN_CGI_t        *eCGI;
    TAI_t               *tAI;
    struct EndpointStruct_t *source_s1ep;

    struct user_ctx_t   *user = PDATA->user_ctx;

    s1msg = (S1AP_Message_t *)signal->data;

    log_msg(LOG_DEBUG, 0, "Enter");

    mme_id = s1ap_findIe(s1msg, id_MME_UE_S1AP_ID);
    CHECKIEPRESENCE(mme_id)

    enb_id = s1ap_findIe(s1msg, id_eNB_UE_S1AP_ID);
    CHECKIEPRESENCE(enb_id)

    eCGI = s1ap_findIe(s1msg, id_EUTRAN_CGI);
    CHECKIEPRESENCE(eCGI)

    tAI = s1ap_findIe(s1msg, id_TAI);
    CHECKIEPRESENCE(tAI)

    if(mme_id->mme_id != user->mME_UE_S1AP_ID || enb_id->eNB_id != user->hoCtx.target_eNB_id){
        return 1;
    }


    /*TAI */
    tAI = (TAI_t*)s1ap_findIe(s1msg, id_TAI);
    if(tAI != NULL){
        user->tAI.MCC = tAI->pLMNidentity->MCC;
        user->tAI.MNC = tAI->pLMNidentity->MNC;
        memcpy(user->tAI.sn, tAI->pLMNidentity->tbc.s, 3);
        memcpy(&(user->tAI.tAC), tAI->tAC->s,2);
        user->tAI.tAC = ntohs(user->tAI.tAC);
    }else{
        return 1;
    }

    /*E-UTRAN CGI */
    eCGI = (EUTRAN_CGI_t*)s1ap_findIe(s1msg, id_EUTRAN_CGI);
    if(eCGI != NULL){
        user->ECGI.MCC = eCGI->pLMNidentity->MCC;
        user->ECGI.MNC = eCGI->pLMNidentity->MNC;
        user->ECGI.cellID = eCGI->cell_ID.id;
    }else{
        return 1;
    }

    /* S1 HO completed.*/

    /*refresh endpoint and bearer parameters*/
    enb_id->eNB_id = user->hoCtx.target_eNB_id;
    PDATA->s1 = user->hoCtx.target_s1;
    PDATA->user_ctx->eNB_UE_S1AP_ID = user->hoCtx.target_eNB_id;
    user->ebearer[0].id = user->hoCtx.eRAB_ID.id;
    memcpy(&(user->ebearer[0].s1u_eNB.teid), user->hoCtx.GTP_TEID.teid, 4);
    if(user->hoCtx.transportLayerAddress.len == 32){
        user->ebearer[0].s1u_eNB.ipv4=1;
        user->ebearer[0].s1u_eNB.ipv6=0;
    }else if(user->hoCtx.transportLayerAddress.len == 128){
        user->ebearer[0].s1u_eNB.ipv4=0;
        user->ebearer[0].s1u_eNB.ipv6=1;
    }else{
        user->ebearer[0].s1u_eNB.ipv4=1;
        user->ebearer[0].s1u_eNB.ipv6=1;
    }
    memcpy(&(user->ebearer[0].s1u_eNB.addr), user->hoCtx.transportLayerAddress.addr, user->hoCtx.transportLayerAddress.len/8);

    return 0;
}


/* ======================================================================
 * S1 API Implementation
 * ====================================================================== */


struct t_process *S1_Setup(struct t_engine_data *self, struct t_process *owner, struct EndpointStruct_t *ep_S1){

    struct  SessionStruct_t *session;

    /*Create new session for for control*/
    session = (struct SessionStruct_t *) malloc(sizeof(struct SessionStruct_t));
    memset(session, 0, sizeof(struct SessionStruct_t));

    session->s1 = ep_S1;

    session->sessionHandler = process_create(self, STATE_S1_Setup, (void*)session, owner);
    ep_S1->handler = session->sessionHandler; /*This process will be used as a handler of the endpoint after the setup*/

    return session->sessionHandler;
}

void S1_newUserSession(struct t_engine_data *engine, struct EndpointStruct_t* ep_S1, S1AP_Message_t *s1msg, uint16_t sid){
    Signal *output;
    struct t_process proc;
    struct  SessionStruct_t *usersession;
    struct user_ctx_t *user;

    log_msg(LOG_DEBUG, 0, "enter S1_newUserSession()");

    /*Create new session for new user*/
    usersession = (struct SessionStruct_t *) malloc(sizeof(struct SessionStruct_t));
    memset(usersession, 0, sizeof(struct SessionStruct_t));
    usersession->user_ctx = (struct user_ctx_t *) malloc(sizeof(struct user_ctx_t));

    user = usersession->user_ctx;
    memset(user, 0, sizeof(struct user_ctx_t));
    usersession->s1 = ep_S1;

    /* Choose SCTP stream ID*/
    usersession->sid = sid;

    /*Create a new process to manage the S1 state machine. The older session handler is stored as parent
     * to return once the S11 state machine ends*/
    usersession->sessionHandler = process_create(engine, STATE_S1_ProcessUeMsg, (void *)usersession, usersession->sessionHandler);

    output = new_signal(usersession->sessionHandler);
    /*output->data = (void *)session;*/
    output->name = S1_newUEmessage;
    output->priority = MAXIMUM_PRIORITY;
    output->data = (void *)s1msg;
    output->freedataFunc = (nullparam_cb_cast)s1msg->freemsg;
    signal_send(output);
}

void S1_UEContextRelease(struct SessionStruct_t *session){
    Signal *output;

    log_msg(LOG_DEBUG, 0, "enter");

    session->sessionHandler->next_state = STATE_S1_UEContextRelease;
    output = new_signal(session->sessionHandler);
    output->name = S1_detach;
    output->priority = MAXIMUM_PRIORITY;
    signal_send(output);

}
