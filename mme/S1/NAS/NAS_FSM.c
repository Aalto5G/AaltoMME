/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   NAS_FSM.c
 * @Author Vicent Ferrer
 * @date   May, 2013
 * @brief  NAS FSM implementation
 *
 * This module implements the NAS interface state machine on the MME EndPoint.
 */

#include "logmgr.h"
#include "NAS_FSM.h"
#include "MME_Controller.h"
#include "NAS.h"
#include "MME_S6a.h"
#include <stdio.h>


/*#define hton24(x) htonl(x)>>8*/


/* ======================================================================
 * NAS Type definitions
 * ====================================================================== */


typedef struct PSMobileId_header_c{
    uint8_t type:3;
    uint8_t parity:1;
}ePSMobileId_header_t;


/* ======================================================================
 * NAS State Machine State prototypes
 * ====================================================================== */


/**@brief State function definition
 * @param [out] returnbuffer return information send to the lower layer protocol
 * @param [out] bsize size of returnbuffer
 * @param [in]  msg parsed message to be processed
 * @param [in]  user user structure to store the parsed information
 *
 *  This functions detects the used protocol and triggers the state processing function.
 */
typedef void (*eSMstate_fn)(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal);
typedef void (*eMMstate_fn)(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal);

static void stateESM_BearerContextInactive(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal);
static void stateESM_BearerContextActivePending(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal);
static void stateESM_BearerContextActive(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal);
static void stateESM_BearerContextModifyPending(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal);
static void stateESM_BearerContextInactivePending(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal);

static void stateEMM_Deregistered(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal);
static void stateEMM_Registered(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal);
static void stateEMM_SpecificProcedureInitiated(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal);
static void stateEMM_CommonProcedureInitiated(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal);
static void stateEMM_DeregisteredInitiated(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal);


/* ======================================================================
 * State Machine State Callbacks Vectors
 * ====================================================================== */


eSMstate_fn eSMstate[]= {
        stateESM_BearerContextInactive,
        stateESM_BearerContextActivePending,
        stateESM_BearerContextActive,
        stateESM_BearerContextInactivePending,
        stateESM_BearerContextModifyPending,
};

eMMstate_fn eMMstate[]= {
        stateEMM_Deregistered,
        stateEMM_Registered,
        stateEMM_SpecificProcedureInitiated,
        stateEMM_CommonProcedureInitiated,
        stateEMM_DeregisteredInitiated,
};


/* ======================================================================
 * TASK Prototypes
 * ====================================================================== */


static uint32_t TASK_AttachReqParse(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal);
static uint32_t TASK_AttachReqCont(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal);
static uint32_t TASK_AttachComplete(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal);

static uint32_t TASK_IdentityReq(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal);
static uint32_t TASK_IdentityRespParse(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal);

static uint32_t TASK_PDNConnectivityReqParse(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal);

static uint32_t TASK_InitSec(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal);
static uint32_t TASK_SecModeCommand(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal);

static uint32_t TASK_EMM_ForgeAttachAccept(uint8_t *returnbuffer, uint32_t *bsize, Signal *signal);
static uint32_t TASK_ESM_ForgeActivateDefaultBearerContextReq(uint8_t *returnbuffer, uint32_t *bsize, Signal *signal);

static uint32_t TASK_DetachReqParse(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal);
static uint32_t TASK_EMM_ForgeDetachAccept(uint8_t *returnbuffer, uint32_t *bsize, Signal *signal);

static uint32_t TASK_Validate_TAUReq(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal);
static uint32_t TASK_EMM_ForgeTAUAccept(uint8_t *returnbuffer, uint32_t *bsize, Signal *signal);


/* ======================================================================
 * Internal Tool Functions
 * ====================================================================== */


static void encodeAPN(uint8_t *res, uint8_t *name){
    uint8_t i, aPN_len, lable_len=0, *tmp;
    aPN_len = strlen(name);
    tmp = name;
    for(i=0; i<aPN_len; i++){
        if(name[i]=='.'){
            *res = lable_len;
            res++;
            memcpy(res, tmp, lable_len);
            res +=lable_len;
            tmp +=(lable_len+1);
            lable_len = 0;
        }else{
            lable_len++;
        }
    }
    *res = lable_len;
    res++;
    memcpy(res, tmp, lable_len);
}


/* ======================================================================
 * ESM STATES Implementation, More info in 3gpp 24.301 clause 6.1.3.3
 * ====================================================================== */


/**
 * No EPS bearer context exists*/
void stateESM_BearerContextInactive(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal){
    /* ACTIVATE DEFAULT EPS BEARER CONTEXT REQUEST
     * ACTIVATE DEDICATED EPS BEARER CONTEXT REQUEST*/
    struct user_ctx_t *user = PDATA->user_ctx;
    PDNConnectivityRequest_t *pdn_req;

    switch(signal->name){
    case NAS_ESM_Continue:
	    if(user->ebearer[0].id>=5){
		    TASK_ESM_ForgeActivateDefaultBearerContextReq(returnbuffer, bsize, signal);
		    PDATA->user_ctx->stateNAS_ESM = ESM_Bearer_Context_Active_Pending;
	    }else{
		    log_msg(LOG_ERR, 0, "Not a valid EPS bearer was created");
	    }

        break;
    default:
        if( msg->plain.eSM.messageType == PDNConnectivityRequest ){
            PDATA->s11 = &(SELF_ON_SIG->s11);   /*Select SGW endpoint structure before entering to S11 state machine*/
            TASK_PDNConnectivityReqParse(returnbuffer, bsize, msg, signal);

            /*Enter S11 State Machine to set user context*/
            user->s11 = S11_newUserAttach(SELF_ON_SIG->s11, user,
                              (void(*)(gpointer)) sendFirstStoredSignal,
                              (gpointer)PDATA->sessionHandler);
        }
        break;
    }

    /*ELSE*/
}

/**
 * The network has initiated an EPS bearer context activation towards the UE*/
void stateESM_BearerContextActivePending(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal){
    struct user_ctx_t *user;
    uint8_t msgType;

    log_msg(LOG_DEBUG, 0, "Enter");

    user = PDATA->user_ctx;
    msgType = msg->plain.eSM.messageType;

    if( msgType == ActivateDefaultEPSBearerContextAccept ||
            msgType == ActivateDedicatedEPSBearerContextAccept){
        user->stateNAS_ESM = ESM_Bearer_Context_Active;

    }else if(msgType == ActivateDefaultEPSBearerContextReject ||
            msgType == ActivateDedicatedEPSBearerContextReject){
        user->stateNAS_ESM = ESM_Bearer_Context_Inactive;
    }
    return;
}

/**
 * The EPS bearer context is active in the network*/
void stateESM_BearerContextActive(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal){
    struct user_ctx_t *user = PDATA->user_ctx;

    log_msg(LOG_DEBUG, 0, "Enter");
    return;
    /* PDN DISCONECT REJCT  including cause value 43*/
    user->stateNAS_ESM = ESM_Bearer_Context_Inactive;

    /* MODIFY EPS BEARER CONTEXT ACCEPT/REJECT including cause value /=43*/
    user->stateNAS_ESM = ESM_Bearer_Context_Modify_Pending;

    /* DEACTIVATE EPS BEARER CONTEXT REQUEST*/
    user->stateNAS_ESM = ESM_Bearer_Context_Inactive_Pending;

    /* PDN DISCONNECT REJECT including cause value = 49
     * BEARER RESOURCE MODIFICATION REJECT including cause value /= 43*/
}

/**
 * The network has initiated an EPS bearer context deactivation towards the UE*/
void stateESM_BearerContextModifyPending(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal){
    struct user_ctx_t *user = PDATA->user_ctx;
    log_msg(LOG_DEBUG, 0, "Enter");
    return;
    /* MODIFY EPS BEARER CONTEXT ACCEPT/REJECT including cause value /=43 */
    user->stateNAS_ESM = ESM_Bearer_Context_Active;

    /* MODIFY EPS BEARER CONTEXT REJECT including cause value =43 */
    user->stateNAS_ESM = ESM_Bearer_Context_Inactive;
}

/**
 * The network has initiated an EPS bearer context deactivation towards the UE*/
void stateESM_BearerContextInactivePending(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal){
    struct user_ctx_t *user = PDATA->user_ctx;
    log_msg(LOG_DEBUG, 0, "Enter");
    return;
    /* DEACTIVATE EPS BEARER CONTEXT ACCEPT*/
    user->stateNAS_ESM = ESM_Bearer_Context_Inactive;
}


/* ======================================================================
 * EMM STATES Implementation, More info in 3gpp 24.301 clause 5.1.3.4
 * ====================================================================== */


/**
 *  The MME has no EMM context or the EMM Context is marked as detached. */
void stateEMM_Deregistered(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal){
    struct user_ctx_t *user = PDATA->user_ctx;
    Signal *continueAttach, *continueAuth;

    INIT_TIME_MEASUREMENT_ENVIRONMENT

    log_msg(LOG_DEBUG, 0, "Enter");

    if(msg == NULL){
        if(signal->name == S1_ContinueAttach){
            TASK_EMM_ForgeAttachAccept(returnbuffer, bsize, signal);
            MME_MEASURE_PROC_TIME
            log_msg(LOG_DEBUG, 0, "NAS: Sent Attach Accept time = %u us", SELF_ON_SIG->procTime);
            PDATA->user_ctx->stateNAS_EMM = EMM_Specific_Procedure_Initiated;
            return;

        }else if(signal->name == NAS_AuthVectorAvailable){
            TASK_InitSec(returnbuffer, bsize, NULL, signal);
            addToPendingResponse(PDATA);
            MME_MEASURE_PROC_TIME
            log_msg(LOG_DEBUG, 0, "NAS: Sent Auth Request time = %u us", SELF_ON_SIG->procTime);
            user->stateNAS_EMM = EMM_Common_Procedure_Initiated;
            return;

        }
    }else if(msg->header.securityHeaderType.v != PlainNAS){
        log_msg(LOG_ERR, 0, "NAS Integrity or security not implemented");
        returnbuffer=NULL;
        *bsize = 0;
        return;
    }

    /* Get procedure & Create response message */
    if (msg->plain.eMM.messageType == AttachRequest || msg->plain.eMM.messageType == IdentityResponse){
        if(signal->name == NAS_AttachReqCont){
            TASK_AttachReqCont(returnbuffer, bsize, msg, signal);

        }else if(msg->plain.eMM.messageType == AttachRequest){
            MME_RESET_TIME
            log_msg(LOG_DEBUG, 0, "NAS: Recv Attach Request, Initiated time measurement.");
            TASK_AttachReqParse(returnbuffer, bsize, msg, signal);
            if(PDATA->user_ctx->imsi == 0ULL){
	            TASK_IdentityReq(returnbuffer, bsize, NULL, signal);
	            addToPendingResponse(PDATA);
	            MME_MEASURE_PROC_TIME
	            log_msg(LOG_DEBUG, 0, "NAS: Sent Identity Request time = %u us", SELF_ON_SIG->procTime);
	            /* Create a new signal to finish the process of the message*/
	            continueAttach = new_signal(signal->processTo);
	            continueAttach->name = NAS_data_available;
	            continueAttach->priority = signal->priority;

	            /* Pass the data to the new signal*/
	            continueAttach->data =  signal->data;
	            signal->data = NULL;
	            continueAttach->freedataFunc = signal->freedataFunc;
	            signal->freedataFunc=NULL;
	            save_signal(continueAttach);    /* Signal to continue with the Attach procedure*/
	            return;
            }
        }else if(msg->plain.eMM.messageType == IdentityResponse){
	        TASK_IdentityRespParse(returnbuffer, bsize, msg, signal);
	        sendFirstStoredSignal(signal->processTo);
	        return;
        }
        if(user->ksi.id==7){
            MME_MEASURE_PROC_TIME
            log_msg(LOG_DEBUG, 0, "NAS: Time Before S6a = %u us", SELF_ON_SIG->procTime);

            PDATA->sessionHandler = process_create(PROC->engine,
                                                   PROC->next_state,
                                                   (void *)PDATA,
                                                   PDATA->sessionHandler);
    
            /* Create a new signal to start the Auth Procedure*/
            continueAuth = new_signal(PDATA->sessionHandler);
            continueAuth->name = NAS_AuthVectorAvailable;
            continueAuth->priority = signal->priority;

            save_signal(continueAuth);      /* Signal to continue with the authentication to S11*/


            //s6a_GetAuthVector(PROC->engine, PDATA);


            /* Create a new signal to finish the process of the message*/
            continueAttach = new_signal(signal->processTo);
            continueAttach->name = NAS_AttachReqCont;
            continueAttach->priority = signal->priority;

            /* Pass the data to the new signal*/
            continueAttach->data =  signal->data;
            signal->data = NULL;
            continueAttach->freedataFunc = signal->freedataFunc;
            signal->freedataFunc=NULL;
            //signal->name = NAS_AuthVectorAvailable;
            save_signal(continueAttach);    /* Signal to continue with the Attach procedure to S11*/
            signal->name = S1_ContinueAttach;

            s6a_GetAuthVector(SELF_ON_SIG->s6a, user,
                              (void(*)(gpointer)) sendFirstStoredSignal,
                              (gpointer)PDATA->sessionHandler);
            return;
        }
    }else{
        log_msg(LOG_WARNING, 0, "Received a NAS message type not expected on this context, %u", msg->plain.eMM.messageType);
    }

    /* @TODO When is this transition done?
     * user->stateNAS_EMM = EMM_Registered;*/
}

/**
 *  The EMM context has been established and a default bearer context has been activated in the MME. */
void stateEMM_Registered(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal){
    struct user_ctx_t *user = PDATA->user_ctx;
    Signal *continueDetach;
    uint8_t error;

    log_msg(LOG_DEBUG, 0, "Enter");

    if(msg->header.securityHeaderType.v != PlainNAS){
        log_msg(LOG_ERR, 0, "NAS Integrity or security not implemented");
        returnbuffer=NULL;
        *bsize = 0;
        return;
    }

    if (msg->plain.eMM.messageType == DetachRequest){

        MME_RESET_TIME
        log_msg(LOG_DEBUG, 0, "NAS: Recv Detach Request, Initiated time measurement.");

        error = TASK_DetachReqParse(NULL, NULL, msg, signal);
        /*MMEENDTIME*/
        if(error){
            log_msg(LOG_ERR, 0, "NAS: Detach Request Parse Error");
        }else{
            /* Create a new signal to start the Auth Procedure*/
            continueDetach = new_signal(signal->processTo);
            continueDetach->name = NAS_ContinueDettach;
            continueDetach->priority = signal->priority;
            save_signal(continueDetach);      /* Signal to continue with the authentication to S11*/
            PDATA->user_ctx->stateNAS_EMM = EMM_Specific_Procedure_Initiated;

            S11_dettach(user->s11,
                        (void(*)(gpointer)) sendFirstStoredSignal,
                        (gpointer)PDATA->sessionHandler);
            Controller_newDetach(PROC->engine, PDATA);
        }
    }else if (msg->plain.eMM.messageType == TrackingAreaUpdateRequest){
        if(TASK_Validate_TAUReq(NULL, NULL, msg, signal)==0){
            TASK_EMM_ForgeTAUAccept(returnbuffer, bsize, signal);
        }

    }else{
        log_msg(LOG_WARNING, 0, "Received a NAS message type not expected on this context, %u", msg->plain.eMM.messageType);
    }

    return;

    /* UE Initiated Detach request
     * TAU Rejected
     * Implicit detach*/
    user->stateNAS_EMM = EMM_Deregistered;

    /* Network Initiated Dettach request*/
    user->stateNAS_EMM = EMM_Deregistered_Initiated;

    /* Common procedure request*/
    user->stateNAS_EMM = EMM_Common_Procedure_Initiated;
}

/*
 * A specific EMM procedure has been started and the MME is waiting for a response from the UE */
void stateEMM_SpecificProcedureInitiated(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal){
    struct user_ctx_t *user = PDATA->user_ctx;

    INIT_TIME_MEASUREMENT_ENVIRONMENT

    log_msg(LOG_DEBUG, 0, "Enter");

    if(msg == NULL){
        if(signal->name == NAS_ContinueDettach){
            TASK_EMM_ForgeDetachAccept(returnbuffer, bsize, signal);
            PDATA->user_ctx->stateNAS_EMM = EMM_Deregistered;
            S1_UEContextRelease(PDATA);
            return;
        }
        user->stateNAS_EMM = EMM_Deregistered;
    }else if(msg->plain.eMM.messageType == AttachComplete){
        /* Specific procedure successful
         * Attach procedure successful and bearer context activated*/
        TASK_AttachComplete(returnbuffer, bsize, msg, signal);
        user->stateNAS_EMM = EMM_Registered;
        MME_MEASURE_PROC_TIME
        Controller_newAttach(PROC->engine, PDATA);
        log_msg(LOG_DEBUG, 0, "NAS: Received Attach Complete, transition to EMM_Registered - time = %u us", SELF_ON_SIG->procTime);

        return;
    }/*else if(msg->plain->eMM->messageType == ){

    }*/else{
        user->stateNAS_EMM = EMM_Deregistered;
    }
    return;
    /* Specific procedure failed
     * Lower layer failure*/
    user->stateNAS_EMM = EMM_Deregistered;
}

/*
 * A common EMM procedure has been started and the MME is waiting for a response from the UE */
void stateEMM_CommonProcedureInitiated(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal){
    struct user_ctx_t *user = PDATA->user_ctx;
    Signal *continueAuth;
    AuthenticationFailure_t *authFail;
    AuthenticationResponse_t * authRsp;

    INIT_TIME_MEASUREMENT_ENVIRONMENT

    log_msg(LOG_DEBUG, 0, "Enter");
    /* Common procedure successful
     * Attach procedure successful and bearer context activated*/
    if(msg->plain.eMM.messageType == AuthenticationResponse){

        MME_MEASURE_PROC_TIME
        log_msg(LOG_DEBUG, 0, "NAS: Received a Authentication Response - time = %u us", SELF_ON_SIG->procTime);
        authRsp = (AuthenticationResponse_t*)&(msg->plain.eMM);

        /* Check XRES == RES*/
        if(authRsp->authParam.l == 8){
	        if(memcmp(authRsp->authParam.v, user->sec_ctx.xRES, 8)!=0){
		        log_msg(LOG_WARNING, 0, "NAS: Authentication Failed for user: %llu",user->imsi);
		        return;
	        }
        }else{
	        log_msg(LOG_WARNING, 0, "NAS: Authentication Parameter has a wrong lenght");
	        return;
        }
        
        /*Continue the attach procedure after authentication
         * @TODO Comment the following line and uncomment the next ones to activate the Security Mode Procedure*/
        run_parent(signal);
        /*sendFirstStoredSignal(signal->processTo);*/

        /*Send security Mode command*/
        /* TASK_SecModeCommand(returnbuffer, bsize, msg, signal);
        addToPendingResponse(PDATA)
        */
        user->stateNAS_EMM = EMM_Deregistered;

    }else if(msg->plain.eMM.messageType ==AuthenticationFailure){
	    authFail = (AuthenticationFailure_t*)&(msg->plain.eMM);

	    log_msg(LOG_WARNING, 0, "Received a NAS AuthenticationFailure, cause : %u", authFail->eMMCause);
	    if(authFail->eMMCause == EMM_SynchFailure){
		    log_msg(LOG_DEBUG, 0, "Starting NAS SQNms Resynch with HSS");
		    //s6a_SynchAuthVector(PROC->engine, PDATA, authFail->optionals[0].tlv_t4.v);

		    /* Create a new signal to start the Auth Procedure*/
		    continueAuth = new_signal(signal->processTo);
		    continueAuth->name = NAS_AuthVectorAvailable;
		    continueAuth->priority = signal->priority;
		    
		    save_signal(continueAuth);      /* Signal to continue with the authentication to S11*/
		    s6a_SynchAuthVector(SELF_ON_SIG->s6a, user, authFail->optionals[0].tlv_t4.v,
		                        (void(*)(gpointer)) sendFirstStoredSignal,
		                        (gpointer)PDATA->sessionHandler);

	    }        

    }else if(msg->plain.eMM.messageType == AuthenticationReject){
        log_msg(LOG_WARNING, 0, "Received a NAS AuthenticationReject");

    }else if(msg->plain.eMM.messageType == SecurityModeComplete){
        log_msg(LOG_DEBUG, 0, "Received a NAS SecurityModeComplete");
        /* @TODO Parse SecurityModeComplete*/


        /*Continue the attach procedure after authentication*/
        sendFirstStoredSignal(signal->processTo);

    }else if(msg->plain.eMM.messageType ==  SecurityModeReject){
        log_msg(LOG_DEBUG, 0, "Received a NAS SecurityModeReject");

    }/*else if(msg->plain->eMM->messageType == ){

    }*/
    user->stateNAS_EMM = EMM_Deregistered;
    return;

    /* Common procedure failed
     * Lower layer failure*/
    user->stateNAS_EMM = EMM_Deregistered;
}

/*
 * A detach procedure has been started and MME is waiting for a response from the UE */
void stateEMM_DeregisteredInitiated(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal){
    struct user_ctx_t *user = PDATA->user_ctx;
    log_msg(LOG_DEBUG, 0, "Enter");
    return;
    /* Dettach accepted
     * Lower layer failure*/
    user->stateNAS_EMM = EMM_Deregistered;

}


/* ======================================================================
 * TASK Implementations
 * ====================================================================== */


uint32_t TASK_AttachReqParse(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal){
    uint8_t esmBuf[500], res[1000], *pointer, t3412, mobId[20];
    uint32_t esmSize, i;
    uint64_t mobid=0ULL;

    GenericNASMsg_t response;
    AttachRequest_t *attachMsg = (AttachRequest_t*)&(msg->plain.eMM);

    log_msg(LOG_DEBUG, 0, "Enter");

    /*ePSAttachType*/
    /*attachMsg->ePSAttachType.v;*/

    /*nASKeySetId*/
    PDATA->user_ctx->ksi.id=7;

    /*ePSMobileId*/
    /*printf("ePSMobileId : %u, mobid = %llu\n", attachMsg->ePSMobileId.l, mobid);*/

    if(((ePSMobileId_header_t*)attachMsg->ePSMobileId.v)->type == 1 ){  /* IMSI*/
        for(i=0; i<attachMsg->ePSMobileId.l-1; i++){
            mobid = mobid*10 + ((attachMsg->ePSMobileId.v[i])>>4);
            mobid = mobid*10 + ((attachMsg->ePSMobileId.v[i+1])&0x0F);
            /*printf("imsi : %llu, %x %x\n", mobid, (attachMsg->ePSMobileId.v[i])>>4, (attachMsg->ePSMobileId.v[i+1])&0x0F);*/
        }
        if(((ePSMobileId_header_t*)attachMsg->ePSMobileId.v)->parity == 1){
            mobid = mobid*10 + ((attachMsg->ePSMobileId.v[i])>>4);
        }

    
        PDATA->user_ctx->imsi = mobid;
        /*printf("${1:format string}"${2: ,a0,a1});intf("imsi : %llu, %x\n", mobid, ((attachMsg->ePSMobileId.v[i+1])>>4));*/
        log_msg(LOG_DEBUG, 0,"imsi : %llu", mobid);
    }

    /*uENetworkCapability*/

    /*eSM_MessageContainer: Parsed on TASK_AttachReqCont*/
    return 0;
}

uint32_t TASK_AttachReqCont(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal){
    uint8_t esmBuf[500], res[1000], *pointer, t3412, mobId[20];
    uint32_t esmSize, i;
    uint64_t mobid=0ULL;

    GenericNASMsg_t response;
    AttachRequest_t *attachMsg = (AttachRequest_t*)&(msg->plain.eMM);

    log_msg(LOG_DEBUG, 0, "Enter");

    /*eSM_MessageContainer*/
    NAS_process(esmBuf, &esmSize, attachMsg->eSM_MessageContainer.v, attachMsg->eSM_MessageContainer.l, signal);

    return 0;
}

uint32_t TASK_AttachComplete(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal){
    uint8_t esmBuf[500], res[1000], *pointer, t3412, mobId[20];
    uint32_t esmSize, i;
    uint64_t mobid=0ULL;

    GenericNASMsg_t response;
    AttachComplete_t *attachMsg = (AttachComplete_t*)&(msg->plain.eMM);

    log_msg(LOG_DEBUG, 0, "Enter");

    /*eSM_MessageContainer*/
    NAS_process(esmBuf, &esmSize, attachMsg->eSM_MessageContainer.v, attachMsg->eSM_MessageContainer.l, signal);

    return 0;
}


uint32_t TASK_IdentityReq(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal){

	uint8_t *pointer;

    log_msg(LOG_DEBUG, 0, "Forging Identity Request");

    pointer = returnbuffer;
    newNASMsg_EMM(&pointer, EPSMobilityManagementMessages, PlainNAS);

    encaps_EMM(&pointer, IdentityRequest);

    /* Selected NAS security algorithms */
    nasIe_v_t1_l(&pointer,1); /*Get Imsi*/
    pointer++; /*Spare half octet*/

    *bsize = pointer-returnbuffer;

    return 0;
}

uint32_t TASK_IdentityRespParse(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal) {
	uint8_t esmBuf[500], res[1000], *pointer, t3412, mobId[20];
    uint32_t i;
    uint64_t mobid=0ULL;

    log_msg(LOG_DEBUG, 0, "Enter");

    IdentityResponse_t *idRsp = (IdentityResponse_t*)&(msg->plain.eMM);

    if(((ePSMobileId_header_t*)idRsp->mobileId.v)->type == 1 ){  /* IMSI*/
        for(i=0; i<idRsp->mobileId.l-1; i++){
            mobid = mobid*10 + ((idRsp->mobileId.v[i])>>4);
            mobid = mobid*10 + ((idRsp->mobileId.v[i+1])&0x0F);
            /*printf("imsi : %llu, %x %x\n", mobid, (attachMsg->ePSMobileId.v[i])>>4, (attachMsg->ePSMobileId.v[i+1])&0x0F);*/
        }
        if(((ePSMobileId_header_t*)idRsp->mobileId.v)->parity == 1){
            mobid = mobid*10 + ((idRsp->mobileId.v[i])>>4);
        }

    }
    PDATA->user_ctx->imsi = mobid;
    /*printf("imsi : %llu, %x\n", mobid, ((attachMsg->ePSMobileId.v[i+1])>>4));*/
    log_msg(LOG_DEBUG, 0,"imsi : %llu", mobid);

    return 0;
}


uint32_t TASK_PDNConnectivityReqParse(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal){
    uint8_t  *pointer, numOp=0;
    ie_tlv_t4_t *temp;

    log_msg(LOG_DEBUG, 0, "Enter");

    PDNConnectivityRequest_t *pdnReq = (PDNConnectivityRequest_t*)&(msg->plain.eSM);

    /*Optionals*/
    if(pdnReq->optionals[numOp].iei&0xF0 == 0xD0){
        /*ESM information transfer flag*/
        numOp++;
    }
    if(pdnReq->optionals[numOp].iei == 0x28){
        /*Access point name*/
        numOp++;
    }
    if(pdnReq->optionals[numOp].iei == 0x27){
        /*Protocol configuration options*/
        temp =  & (pdnReq->optionals[numOp].tlv_t4);
        memcpy(PDATA->user_ctx->pco, temp, temp->l+2);
        numOp++;
    }
    if(pdnReq->optionals[numOp].iei&0xF0 == 0x0C0){
        /*Device properties*/
        numOp++;
    }
    return 0;
}


uint32_t TASK_InitSec(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal){
    uint8_t *pointer, ksi, randv[8];
    struct user_ctx_t *user = PDATA->user_ctx;

    log_msg(LOG_DEBUG, 0, "Initiating UE authentication");

    /* Forge Auth Request*/
    pointer = returnbuffer;
    newNASMsg_EMM(&pointer, EPSMobilityManagementMessages, PlainNAS);

    encaps_EMM(&pointer, AuthenticationRequest);

    /* NAS Key Set ID */
    memcpy(&ksi,&(user->ksi),1);
    nasIe_v_t1_l(&pointer, ksi&0x0F);
    pointer++; /*Spare half octet*/

    /*printfbuffer(user->sec_ctx.rAND, 16);*/
    /*printfbuffer(user->sec_ctx.aUTN, 16);*/

    /* RAND */
    nasIe_v_t3(&pointer, user->sec_ctx.rAND, 16); /* 256 bits */

    /* AUTN */
    nasIe_lv_t4(&pointer, user->sec_ctx.aUTN, 16); /* 256 bits */

    *bsize = pointer-returnbuffer;

    /* No state change*/
    /*PDATA->user_ctx->stateNAS_EMM = EMM_Common_Procedure_Initiated;
    PDATA->user_ctx->stateNAS_ESM = ESM_Bearer_Context_Active_Pending;
    */

    return 0;
}

uint32_t TASK_SecModeCommand(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal){
    uint8_t *pointer, ksi, randv[8], algorithms=0, capabilities[2];
    struct user_ctx_t *user = PDATA->user_ctx;

    log_msg(LOG_DEBUG, 0, "Forging Secure Mode Command");

    pointer = returnbuffer;
    newNASMsg_EMM(&pointer, EPSMobilityManagementMessages, PlainNAS);

    encaps_EMM(&pointer, SecurityModeCommand);

    /* Selected NAS security algorithms */
    nasIe_v_t3(&pointer, &algorithms, 1);

    /*NAS key set identifier*/
    memcpy(&ksi,&(user->ksi),1);
    nasIe_v_t1_l(&pointer, ksi&0x0F);
    pointer++; /*Spare half octet*/

    /* Replayed UE security capabilities */
    capabilities[0]=0x80;
    capabilities[1]=0x80;
    nasIe_lv_t4(&pointer, capabilities, 2); /* 256 bits */

    *bsize = pointer-returnbuffer;


    return 0;
}

uint32_t TASK_EMM_ForgeAttachAccept(uint8_t *returnbuffer, uint32_t *bsize, Signal *signal){
    uint8_t *pointer, t3412, esm[300], aPN[100], guti[11];
    uint32_t len, esmLen, tbcd_plmn;
    NAS_tai_list_t tAIl;
    struct user_ctx_t *user = PDATA->user_ctx;


    /* Forge Attach Accept*/
    pointer = returnbuffer;
    newNASMsg_EMM(&pointer, EPSMobilityManagementMessages, PlainNAS);

    encaps_EMM(&pointer, AttachAccept);

    /* EPS attach result */
    nasIe_v_t1_l(&pointer, 1); /* EPS only */
    pointer++; /*Spare half octet*/

    /* T3412 value */
    t3412 = 0x23;
    nasIe_v_t3(&pointer, &t3412, 1); /* EPS only */

    /* TAI list */
    memset(&tAIl,0,sizeof(NAS_tai_list_t));
    tAIl.numOfElem = 0; /* 1 - 1*/
    tAIl.type = 0;
    tAIl.list.singlePLMNnonconsec.plmn = encapPLMN(PDATA->user_ctx->tAI.MCC, PDATA->user_ctx->tAI.MNC);
    tAIl.list.singlePLMNnonconsec.tAC[0] = htons(PDATA->user_ctx->tAI.tAC);
    nasIe_lv_t4(&pointer, (uint8_t*)&tAIl, 6);

    signal->name = NAS_ESM_Continue;
    eSMstate[user->stateNAS_ESM](esm, &esmLen, NULL, signal);

    /* ESM message container */
    nasIe_lv_t6(&pointer, esm, esmLen);

    /* GUTI */
    memcpy(&tbcd_plmn, SELF_ON_SIG->servedGUMMEIs->item[0]->servedPLMNs->item[0]->tbc.s, 3);
    PDATA->user_ctx->guti.tbcd_plmn = tbcd_plmn;
    memcpy(&(PDATA->user_ctx->guti.mmegi), SELF_ON_SIG->servedGUMMEIs->item[0]->servedGroupIDs->item[0]->s, 2);
    memcpy(&(PDATA->user_ctx->guti.mmec), SELF_ON_SIG->servedGUMMEIs->item[0]->servedMMECs->item[0]->s, 1);
    PDATA->user_ctx->guti.mtmsi = 0x80000001;

    guti[0]=0xF6;   /*1111 0 110 - spare, odd/even , GUTI id*/
    memcpy(guti+1, &(PDATA->user_ctx->guti), 10);

    nasIe_tlv_t4(&pointer, 0x50, guti, 11);
    *(pointer-12) = 11;

    *bsize = pointer-returnbuffer;

    return 0;
}

uint32_t TASK_ESM_ForgeActivateDefaultBearerContextReq(uint8_t *returnbuffer, uint32_t *bsize, Signal *signal){
    uint8_t *pointer, aPN[100],pco[14];
    uint32_t len;

    /* Forge Activate Default Bearer Context Req*/
    pointer = returnbuffer;
    newNASMsg_ESM(&pointer, EPSSessionManagementMessages, 5);
    encaps_ESM(&pointer, 1 ,ActivateDefaultEPSBearerContextRequest);

    /* EPS QoS */
    nasIe_lv_t4(&pointer, &(PDATA->user_ctx->ebearer->qos.qci), 1);

    /* Access point name*/
    encodeAPN(aPN, PDATA->user_ctx->aPname);
    nasIe_lv_t4(&pointer, aPN, strlen(PDATA->user_ctx->aPname)+1); /* This +1 is the initial label lenght*/

    /* PDN address */
    switch(PDATA->user_ctx->pAA.type){
    case  1: /* IPv4*/
        len = 5;
        break;
    case 2: /* IPv6*/
        len = 9;
        break;
    case 3: /*IPv4v6*/
        len = 13;
        break;
    }
    nasIe_lv_t4(&pointer, (uint8_t*)&(PDATA->user_ctx->pAA), len);

    /*Optionals */
    /* Protocol Configuration Options*/
    if(PDATA->user_ctx->pco[0]==0x27){
        if(PROC->engine->mme->uE_DNS==0){
            log_msg(LOG_DEBUG, 0, "Writting PCO IE. Lenght: %d, first byte %#x", PDATA->user_ctx->pco[1]+2, *(PDATA->user_ctx->pco+2));
            nasIe_tlv_t4(&pointer, 0x27, PDATA->user_ctx->pco+2, PDATA->user_ctx->pco[1]);
        *(pointer-1-PDATA->user_ctx->pco[1]) = PDATA->user_ctx->pco[1];
        }else{
        pco[0]=0x80;
        pco[1]=0x80;
        pco[2]=0x21;
        pco[3]=0x0a;
        pco[4]=0x01;
        pco[5]=0x00;
        pco[6]=0x00;
        pco[7]=0x0a;
        pco[8]=0x81;
        pco[9]=0x06;
        memcpy(pco+10, &(PROC->engine->mme->uE_DNS), 4);
        nasIe_tlv_t4(&pointer, 0x27, pco, 14);
        *(pointer-15) = 14;
    }

    }

    *bsize = pointer-returnbuffer;
    return 0;

}

uint32_t TASK_DetachReqParse(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal){
    uint8_t esmBuf[500], res[1000], *pointer, t3412, mobId[20];
    uint32_t esmSize, i;
    uint64_t mobid=0ULL;
    guti_t  *guti;


    DetachRequestUEOrig_t *detachMsg = (DetachRequestUEOrig_t*)&(msg->plain.eMM);

    log_msg(LOG_DEBUG, 0, "Enter");

    /*ePSAttachType*/
    /*detachMsg->detachType.v;*/

    /*nASKeySetId*/
    /*if(detachMsg->nASKeySetId.v != PDATA->user_ctx->ksi.id){
        log_msg(LOG_ERR, 0, "Incorrect KSI: %u Ignoring detach", detachMsg->nASKeySetId.v);
        return 1;
     }*/

    /*EPSMobileId*/
    if(((ePSMobileId_header_t*)detachMsg->ePSMobileId.v)->type == 1 ){  /* IMSI*/
        for(i=0; i<detachMsg->ePSMobileId.l-1; i++){
            mobid = mobid*10 + ((detachMsg->ePSMobileId.v[i])>>4);
            mobid = mobid*10 + ((detachMsg->ePSMobileId.v[i+1])&0x0F);
            /*printf("imsi : %llu, %x %x\n", mobid, (attachMsg->ePSMobileId.v[i])>>4, (attachMsg->ePSMobileId.v[i+1])&0x0F);*/
        }
        if(((ePSMobileId_header_t*)detachMsg->ePSMobileId.v)->parity == 1){
            mobid = mobid*10 + ((detachMsg->ePSMobileId.v[i])>>4);
        }
        if(PDATA->user_ctx->imsi != mobid){
            log_msg(LOG_ERR, 0, "received IMSI doesn't match.");
            return 1;
        }
    }else if(((ePSMobileId_header_t*)detachMsg->ePSMobileId.v)->type == 1 ){    /*GUTI*/
        guti = (guti_t  *)(detachMsg->ePSMobileId.v+1);
        if(memcmp(guti, &(PDATA->user_ctx->guti), 10)!=0){
            log_msg(LOG_ERR, 0, "GUTI incorrect. GUTI reallocation not implemented yet.");
            return 1;
        }
    }


    return 0;
}

uint32_t TASK_EMM_ForgeDetachAccept(uint8_t *returnbuffer, uint32_t *bsize, Signal *signal){
    uint8_t *pointer;

    /* Forge Detach Accept*/
    pointer = returnbuffer;
    newNASMsg_EMM(&pointer, EPSMobilityManagementMessages, PlainNAS);

    encaps_EMM(&pointer, DetachAccept);

    *bsize = pointer-returnbuffer;
    return 0;
}

uint32_t TASK_Validate_TAUReq(uint8_t *returnbuffer, uint32_t *bsize, GenericNASMsg_t *msg, Signal *signal){
    uint8_t esmBuf[500], res[1000], *pointer, t3412, mobId[20];
    uint32_t esmSize, i;
    uint64_t mobid=0ULL;
    guti_t  *guti;


    TrackingAreaUpdateRequest_t *tau_msg = (TrackingAreaUpdateRequest_t*)&(msg->plain.eMM);

    log_msg(LOG_DEBUG, 0, "Enter");

    /*EPS update type */
    if((tau_msg->ePSUpdateType.v &0x07) != 0){
        log_msg(LOG_WARNING, 0, "Not a TA update, ignoring. (ePSUpdateType = %u)", tau_msg->ePSUpdateType.v);
        return 1;
    }

    /*nASKeySetId*/
    if(tau_msg->nASKeySetId.v != PDATA->user_ctx->ksi.id && tau_msg->nASKeySetId.v != 7){
	    log_msg(LOG_WARNING, 0, "Key Set invalid (tau: %u != ctx: %u). New authentication procedure must be triggered. Not implemented yet",
	            tau_msg->nASKeySetId.v, PDATA->user_ctx->ksi.id);
        return 1;
    }

    /*EPSMobileId*/
    if(((ePSMobileId_header_t*)tau_msg->oldGUTI.v)->type == 6 ){  /* GUTI*/
        guti = (guti_t  *)(tau_msg->oldGUTI.v+1);
        if(memcmp(guti, &(PDATA->user_ctx->guti), 10)!=0){
            log_msg(LOG_DEBUG, 0, "GUTI incorrect. GUTI reallocation not implemented yet.");
            return 1;
        }
    }

    /*Check GUTI*/

    return 0;
}

uint32_t TASK_EMM_ForgeTAUAccept(uint8_t *returnbuffer, uint32_t *bsize, Signal *signal){

    NAS_tai_list_t tAIl;
    uint8_t *pointer, t3412, guti[11];
    uint32_t tbcd_plmn;

    /* Forge TAU Accept*/
    pointer = returnbuffer;
    newNASMsg_EMM(&pointer, EPSMobilityManagementMessages, PlainNAS);

    encaps_EMM(&pointer, TrackingAreaUpdateAccept);

    /*EPS update result*/
    nasIe_v_t1_h(&pointer, 0); /* TA updated */
    //pointer++;

    /* T3412 value */
    /*t3412 = 0x23;
      nasIe_v_t3(&pointer, &t3412, 1); *//* EPS only */

    /* GUTI */
    /*    memcpy(&tbcd_plmn, SELF_ON_SIG->servedGUMMEIs->item[0]->servedPLMNs->item[0]->tbc.s, 3);
    PDATA->user_ctx->guti.tbcd_plmn = tbcd_plmn;
    memcpy(&(PDATA->user_ctx->guti.mmegi), SELF_ON_SIG->servedGUMMEIs->item[0]->servedGroupIDs->item[0]->s, 2);
    memcpy(&(PDATA->user_ctx->guti.mmec), SELF_ON_SIG->servedGUMMEIs->item[0]->servedMMECs->item[0]->s, 1);
    PDATA->user_ctx->guti.mtmsi = 0x80000001;*/

    /*guti[0]=0xF6;*/   /*1111 0 110 - spare, odd/even , GUTI id*/
    /*memcpy(guti+1, &(PDATA->user_ctx->guti), 10);

    nasIe_tlv_t4(&pointer, 0x50, guti, 11);
    *(pointer-12) = 11;*/

    /* TAI list *//*
    memset(&tAIl,0,sizeof(NAS_tai_list_t));
    tAIl.numOfElem = 0;*/ /* 1 - 1*//*
    tAIl.type = 0;
    tAIl.list.singlePLMNnonconsec.plmn = encapPLMN(PDATA->user_ctx->tAI.MCC, PDATA->user_ctx->tAI.MNC);
    tAIl.list.singlePLMNnonconsec.tAC[0] = PDATA->user_ctx->tAI.tAC;
    nasIe_lv_t4(&pointer, (uint8_t*)&tAIl, 6);
                                    */
    *bsize = pointer-returnbuffer;
    return 0;
}


/* ======================================================================
 * NAS API Implementation
 * ====================================================================== */


void NAS_process(uint8_t *returnbuffer, uint32_t *bsize, void *pack, uint32_t size, Signal *signal){

    GenericNASMsg_t msg;
    struct user_ctx_t *user = PDATA->user_ctx;

    if(bsize != NULL){
        *bsize=0;
    }

    dec_NAS(&msg, (uint8_t*)pack, size);

    if(msg.header.protocolDiscriminator.v == EPSMobilityManagementMessages){
        /*Execute EMM State*/
        log_msg(LOG_DEBUG, 0, "Execute EMM State, %u", user->stateNAS_EMM);
        eMMstate[user->stateNAS_EMM](returnbuffer, bsize, &msg, signal);
    }else if(msg.header.protocolDiscriminator.v == EPSSessionManagementMessages){
        /*Execute ESM State*/
        log_msg(LOG_DEBUG, 0, "Execute ESM State");
        eSMstate[user->stateNAS_ESM](returnbuffer, bsize, &msg, signal);
    }else{
        log_msg(LOG_ERR, 0, "protocolDiscriminator = %u not valid, not EMM (%u) or ESM (%u)",
                msg.header.protocolDiscriminator.v, EPSMobilityManagementMessages, EPSSessionManagementMessages);
    }
}

void NAS_sessionAvailable(uint8_t *returnbuffer, uint32_t *bsize, Signal *signal){

    /*GenericNASMsg_t *msg = (GenericNASMsg_t *)signal->data;*/
    struct user_ctx_t *user = PDATA->user_ctx;

    eMMstate[PDATA->user_ctx->stateNAS_EMM](returnbuffer, bsize, NULL, signal);


    return;
}
