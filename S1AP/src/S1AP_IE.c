/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S1AP_IE.c
 * @Author Vicent Ferrer
 * @date   April, 2013
 * @brief  S1AP IE definitions
 *
 * S1AP protocol Information Element (IE) types definition.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "S1AP_IE.h"
#include "S1APlog.h"

/* Dictionaries*/
const char *PagingDRXName []  = {"v32", "v64", "v128", "v256"};
const char *TypeOfErrorName [] = {"not_understood", "missing" };

/* Cause dictionaries*/
const char *CauseRadioNetworkName[] = {"unspecified","tx2relocoverall-expiry","successful-handover","release-due-to-eutran-generated-reason","handover-cancelled","partial-handover","ho-failure-in-target-EPC-eNB-or-target-system","ho-target-not-allowed","tS1relocoverall-expiry","tS1relocprep-expiry","cell-not-available","unknown-targetID","no-radio-resources-available-in-target-cell","unknown-mme-ue-s1ap-id","unknown-enb-ue-s1ap-id","unknown-pair-ue-s1ap-id","handover-desirable-for-radio-reason","time-critical-handover","resource-optimisation-handover","reduce-load-in-serving-cell","user-inactivity","radio-connection-with-ue-lost","load-balancing-tau-required","cs-fallback-triggered","ue-not-available-for-ps-service","radio-resources-not-available","failure-in-radio-interface-procedure","invalid-qos-combination","interrat-redirection","interaction-with-other-procedure","unknown-E-RAB-ID","multiple-E-RAB-ID-instances","encryption-and-or-integrity-protection-algorithms-not-supported","s1-intra-system-handover-triggered","s1-inter-system-handover-triggered","x2-handover-triggered"};
const char *CauseRadioNetwork_extName[] = {"redirection-towards-1xRTT","not-supported-QCI-value","invalid-CSG-Id"};

const char *CauseTransportName[] = {"transport-resource-unavailable", "unspecified"};

const char *CauseNASName[] = {"normal-release","authentication-failure","detach","unspecified"};
const char *CauseNAS_extName[] = {"csg-subscription-expiry"};

const char *CauseProtocolName[] = {"transfer-syntax-error", "abstract-syntax-error-reject", "abstract-syntax-error-ignore-and-notify", "message-not-compatible-with-receiver-state", "semantic-error", "abstract-syntax-error-falsely-constructed-message", "unspecified"};

const char *CauseMiscName[] = {"control-processing-overload", "not-enough-user-plane-processing-resources", "hardware-failure", "om-intervention", "unspecified", "unknown-PLMN"};
/* End Cause dictionaries*/

const char *RRC_Establishment_Cause_Name[] = {"emergency", "highPriorityAccess", "mt-Access", "mo-Signalling", "mo-Data"};

const char *Pre_emptionCapabilityName[] = {"shall-not-trigger-pre-emption", "may-trigger-pre-emption"};

const char *Pre_emptionVulnerabilityName[] = {"not-pre-emptable", "pre-emptable"};

const char *HandoverTypeName[] = {"intralte", "ltetoutran", "ltetogeran", "utrantolte", "gerantolte"};

iEconstructor getconstructor[]={
        (iEconstructor)new_MME_UE_S1AP_ID,
        (iEconstructor)new_HandoverType,
        (iEconstructor)new_Cause,
        (iEconstructor)NULL, /*new_SourceID,*/
        (iEconstructor)new_TargetID,
        (iEconstructor)NULL,
        (iEconstructor)NULL,
        (iEconstructor)NULL,
        (iEconstructor)new_ENB_UE_S1AP_ID,
        (iEconstructor)NULL,
        (iEconstructor)NULL,
        (iEconstructor)NULL,
        (iEconstructor)new_E_RABList, /* Container : new_E_RABSubjecttoDataForwardingList,*/
        (iEconstructor)new_E_RABList, /*ERAB-toReleaseListHOCmd,*/
        (iEconstructor)NULL, /* Sequence : new_E_RABDataForwardingItem,*//* NOt Implemented*/
        (iEconstructor)NULL, /*new_E_RABReleaseItemBearerRelComp,*//* NOt Implemented*/
        (iEconstructor)new_E_RABToBeSetupListBearerSUReq,
        (iEconstructor)new_E_RABToBeSetupItemBearerSUReq,
        (iEconstructor)NULL, /*new_E_RABAdmittedList,*//* NOt Implemented*/
        (iEconstructor)NULL, /* Container : new_E_RABFailedToSetupListHOReqAck,*/
        (iEconstructor)new_E_RABAdmittedItem,
        (iEconstructor)NULL, /* Item Of Container : new_E_RABFailedtoSetupItemHOReqAck,*/
        (iEconstructor)new_E_RABSetupListBearerSURes, /* new_E_RABToBeSwitchedDLList,*/ /* NOt Implemented*/
        (iEconstructor)new_E_RABSetupItemBearerSURes, /* new_E_RABToBeSwitchedDLItem, *//*NOt Implemented*/
        (iEconstructor)new_E_RABToBeSetupListCtxtSUReq,
        (iEconstructor)NULL, /* new_TraceActivation,*//*NOt Implemented*/
        (iEconstructor)new_Unconstrained_Octed_String, /*NAS_PDU,*/
        (iEconstructor)new_E_RABToBeSetupItemHOReq,
        (iEconstructor)new_E_RABSetupListBearerSURes,
        (iEconstructor)new_E_RABList, /*new_E_RABFailedToSetupListBearerSURes,*/
        (iEconstructor)NULL, /*Container : new_E_RABToBeModifiedListBearerModReq,*/
        (iEconstructor)NULL, /*Container : new_E_RABModifyListBearerModRes, */
        (iEconstructor)new_E_RABList, /*new_E_RABFailedToModifyList,*/
        (iEconstructor)new_E_RABList, /*new_E_RABToBeReleasedList,*/
        (iEconstructor)new_E_RABList, /*new_E_RABFailedToReleaseList,*/
        (iEconstructor)new_E_RABItem,
        (iEconstructor)new_E_RABToBeModifiedItemBearerModReq,
        (iEconstructor)NULL, /* Item of container new_E_RABModifyItemBearerModRes,*/
        (iEconstructor)NULL, /*new_E_RABReleaseItem, NOT Found on ASN.1 description*/
        (iEconstructor)new_E_RABSetupItemBearerSURes,
        (iEconstructor)new_SecurityContext,
        (iEconstructor)NULL, /* new_HandoverRestrictionList,*/ /* Not implemented*/
        (iEconstructor)NULL,
        (iEconstructor)NULL, /* new_UEPagingID,*/ /* Not implemented*/
        (iEconstructor)new_PagingDRX,
        (iEconstructor)NULL,
        (iEconstructor)NULL, /* Container new_TAIList,*/
        (iEconstructor)NULL, /* new_TAIItem,*//* Not implemented*/
        (iEconstructor)new_E_RABList, /*new_E_RABFailedToSetupListCtxtSURes,*/
        (iEconstructor)NULL, /*new_E_RABReleaseItemHOCmd, NOT Found on ASN.1 description*/
        (iEconstructor)new_E_RABSetupItemCtxtSURes, /* new_E_RABSetupItemCtxtSURes,*/ /* Alias*/
        (iEconstructor)new_E_RABSetupListCtxtSURes, /* Container new_E_RABSetupListCtxtSURes,*/ /* Alias*/
        (iEconstructor)new_E_RABToBeSetupItemCtxtSUReq,
        (iEconstructor)new_E_RABToBeSetupListHOReq, /* Container new_E_RABToBeSetupListHOReq,*/
        (iEconstructor)NULL,
        (iEconstructor)NULL, /* new_GERANtoLTEHOInformationRes,*/ /* Not implemented*/
        (iEconstructor)NULL,
        (iEconstructor)NULL, /*new_UTRANtoLTEHOInformationRes,*/ /* Not implemented*/
        (iEconstructor)new_CriticalityDiagnostics,
        (iEconstructor)new_Global_ENB_ID,
        (iEconstructor)new_ENBname,
        (iEconstructor)new_MMEname,
        (iEconstructor)NULL,
        (iEconstructor)new_ServedPLMNs,
        (iEconstructor)new_SupportedTAs,
        (iEconstructor)new_TimeToWait,
        (iEconstructor)new_UEAggregateMaximumBitrate,
        (iEconstructor)new_TAI,
        (iEconstructor)NULL,
        (iEconstructor)NULL, /* Container new_E_RABReleaseListBearerRelComp,*/
        (iEconstructor)NULL, /*new_cdma2000PDU,*//* Not implemented*/
        (iEconstructor)NULL, /*new_cdma2000RATType,*//* Not implemented*/
        (iEconstructor)NULL, /*new_cdma2000SectorID,*//* Not implemented*/
        (iEconstructor)new_SecurityKey,
        (iEconstructor)new_UERadioCapability,
        (iEconstructor)NULL, /* new_GUMMEI,*//* Not Implemented*/
        (iEconstructor)NULL,
        (iEconstructor)NULL,
        (iEconstructor)NULL, /* new_E_RABInformationListItem, *//* Not implemented*/
        (iEconstructor)new_Direct_Forwarding_Path_Availability,
        (iEconstructor)NULL, /*new_UEIdentityIndexValue,*//* Not implemented*/
        (iEconstructor)NULL,
        (iEconstructor)NULL,
        (iEconstructor)NULL, /*new_cdma2000HOStatus,*//* Not implemented*/
        (iEconstructor)NULL, /*new_cdma2000HORequiredIndication,*/ /* Not implemented*/
        (iEconstructor)NULL,
        (iEconstructor)NULL, /* new_E_UTRAN_Trace_ID,*/ /* Not implemented*/
        (iEconstructor)new_RelativeMMECapacity,
        (iEconstructor)new_MME_UE_S1AP_ID, /*SourceMME_UE_S1AP_ID*/
        (iEconstructor)new_Bearers_SubjectToStatusTransfer_Item,
        (iEconstructor)NULL, /*new_eNB_StatusTransfer_TransparentContainer,*/ /* Not implemented*/
        (iEconstructor)NULL, /*new_UE_associatedLogicalS1_ConnectionItem,*/ /* Not implemented*/
        (iEconstructor)NULL, /* new_ResetType,*/ /* Not implemented*/
        (iEconstructor)NULL, /*new_UE_associatedLogicalS1_ConnectionListResAck,*/ /* Not implemented*/
        (iEconstructor)new_E_RABSetupItemBearerSURes, /*new_E_RABToBeSwitchedULItem,*/ /* Not implemented*/
        (iEconstructor)new_E_RABToBeSwitchedULList, /*new_E_RABToBeSwitchedULList,*/ /* Not implemented*/
        (iEconstructor)new_S_TMSI, /*new_S_TMSI,*/
        (iEconstructor)NULL, /*new_cdma2000OneXRAND,*/ /* Not implemented*/
        (iEconstructor)NULL, /*new_RequestType,*/ /* Not implemented*/
        (iEconstructor)new_UE_S1AP_IDs,
        (iEconstructor)new_EUTRAN_CGI,
        (iEconstructor)NULL, /*new_OverloadResponse,*/ /* Not implemented*/
        (iEconstructor)NULL, /*new_cdma2000OneXSRVCCInfo,*/ /* Not implemented*/
        (iEconstructor)NULL, /*new_E_RABFailedToBeReleasedList,*/ /* Not implemented*/
        (iEconstructor)new_Unconstrained_Octed_String, /* new_Source_ToTarget_TransparentContainer,*/
        (iEconstructor)new_ServedGUMMEIs,
        (iEconstructor)new_SubscriberProfileIDforRFP,
        (iEconstructor)new_UESecurityCapabilities,
        (iEconstructor)NULL, /*new_CSFallbackIndicator,*/ /* Not implemented*/
        (iEconstructor)NULL, /*new_CNDomain,*/ /* Not implemented*/
        (iEconstructor)new_E_RABList, /*new_E_RABReleasedList,*/
        (iEconstructor)NULL, /*new_MessageIdentifier,*/ /* Not implemented*/
        (iEconstructor)NULL, /*new_SerialNumber,*/ /* Not implemented*/
        (iEconstructor)NULL, /*new_WarningAreaList,*/ /* Not implemented*/
        (iEconstructor)NULL, /*new_RepetitionPeriod,*/ /* Not implemented*/
        (iEconstructor)NULL, /*new_NumberofBroadcastRequest,*/ /* Not implemented*/
        (iEconstructor)NULL, /*new_WarningType,*/ /* Not implemented*/
        (iEconstructor)NULL, /*new_WarningSecurityInfo,*/ /* Not implemented*/
        (iEconstructor)NULL, /*new_DataCodingScheme,*/ /* Not implemented*/
        (iEconstructor)NULL, /*new_WarningMessageContents,*/ /* Not implemented*/
        (iEconstructor)NULL, /*new_BroadcastCompletedAreaList,*/ /* Not implemented*/
        (iEconstructor)NULL, /*new_Inter_SystemInformationTransferTypeEDT,*/ /* Not implemented*/
        (iEconstructor)NULL, /*new_Inter_SystemInformationTransferTypeMDT,*/ /* Not implemented*/
        (iEconstructor)new_Unconstrained_Octed_String, /*new_Target_ToSource_TransparentContainer,*/
        (iEconstructor)NULL, /*new_SRVCCOperationPossible,*/ /* Not implemented*/
        (iEconstructor)NULL, /*new_SRVCCHOIndication,*/ /* Not implemented*/
        (iEconstructor)NULL, /*new_NAS_DownlinkCount,*/ /* Not implemented*/
        (iEconstructor)new_CSG_id,
        (iEconstructor)new_CSG_IdList,
        (iEconstructor)NULL, /*new_SONConfigurationTransferECT,*/ /* Not implemented*/
        (iEconstructor)NULL, /*new_SONConfigurationTransferMCT,*/ /* Not implemented*/
        (iEconstructor)NULL, /*new_TraceCollectionEntityIPAddress,*/ /* Not implemented*/
        (iEconstructor)NULL, /*new_MSClassmark2,*/ /* Not implemented*/
        (iEconstructor)NULL, /*new_MSClassmark3,*/ /* Not implemented*/
        (iEconstructor)new_RRC_Establishment_Cause,
        (iEconstructor)new_Unconstrained_Octed_String, /*new_NASSecurityParametersfromE_UTRAN,*/
        (iEconstructor)new_Unconstrained_Octed_String, /*new_NASSecurityParameterstoE_UTRAN,*/
        (iEconstructor)new_PagingDRX, /*new_DefaultPagingDRX,*/
        (iEconstructor)new_Unconstrained_Octed_String, /*new_Source_ToTarget_TransparentContainer_Secondary,*/
        (iEconstructor)new_Unconstrained_Octed_String, /*new_Target_ToSource_TransparentContainer_Secondary,*/
        (iEconstructor)NULL, /*new_EUTRANRoundTripDelayEstimationInfo,*//* Not implemented*/
        (iEconstructor)NULL, /*new_BroadcastCancelledAreaList,*//* Not implemented*/
        (iEconstructor)NULL, /*new_ConcurrentWarningMessageIndicator,*//* Not implemented*/
        (iEconstructor)NULL, /*new_Data_Forwarding_Not_Possible,*//* Not implemented*/
        (iEconstructor)NULL, /*new_ExtendedRepetitionPeriod,*//* Not implemented*/
        (iEconstructor)NULL, /*new_CellAccessMode,*//* Not implemented*/
        (iEconstructor)NULL, /*new_CSGMembershipStatus,*//* Not implemented*/
        (iEconstructor)NULL, /*new_LPPa_PDU,*//* Not implemented*/
        (iEconstructor)NULL, /*new_Routing_ID,*//* Not implemented*/
        (iEconstructor)NULL, /*new_Time_Synchronization_Info,*//* Not implemented*/
        (iEconstructor)NULL, /*new_PS_ServiceNotAvailable,*//* Not implemented*/
        (iEconstructor)NULL, /*new_PagingPriority,*//* Not implemented*/
        (iEconstructor)NULL, /*new_x2TNLConfigurationInfo,*//* Not implemented*/
        (iEconstructor)NULL, /*new_eNBX2ExtendedTransportLayerAddresses,*//* Not implemented*/
        (iEconstructor)NULL, /*new_GUMMEIList,*//* Not implemented*/
        (iEconstructor)new_TransportLayerAddress, /*GW_TransportLayerAddress,*/
        (iEconstructor)NULL, /*new_Correlation_ID,*//* Not implemented*/
        (iEconstructor)NULL, /*new_SourceMME_GUMMEI,*//* Not implemented*/
        (iEconstructor)NULL, /*new_MME_UE_S1AP_new_2,*//* Not implemented*/
        (iEconstructor)NULL, /*new_RegisteredLAI,*//* Not implemented*/
        (iEconstructor)NULL, /*new_RelayNode_Indicator,*//* Not implemented*/
        (iEconstructor)NULL, /*new_TrafficLoadReductionIndication,*//* Not implemented*/
        (iEconstructor)NULL, /*new_MDTConfiguration,*//* Not implemented*/
        (iEconstructor)new_MMERelaySupportIndicator,
        (iEconstructor)NULL, /*new_GWContextReleaseIndication,*//* Not implemented*/
        (iEconstructor)NULL, /*new_ManagementBasedMDTAllowed,*//* Not implemented*/
        (iEconstructor)NULL, /*new_PrivacyIndicator,*//* Not implemented*/
        (iEconstructor)NULL, /*new_Time_UE_StayedInCell_EnhancedGranularity,*/
        (iEconstructor)new_Cause, /*new_HO_Cause,*/
        (iEconstructor)NULL, /*new_VoiceSupportMatchIndicator*//* Not implemented*/
        (iEconstructor)NULL, /*new_GUMMEIType,*//* Not implemented*/
};


/* ************************************************************ */
/* ********************** Other IE Types ********************** */
/* ************************************************************ */


/* *********************** PLMN identity ********************** */
/** @brief PLMNidentity IE Destructor
 *
 * Deallocate the PLMNidentity_t structure.
 * */
void free_PLMNidentity(void* data){
    PLMNidentity_t *self = (PLMNidentity_t*)data;

    if(!self){
        return;
    }

    free(self);
}

void plmnId_tbcd2MccMnc(PLMNidentity_t* self){
    self->MCC = ((self->tbc.s[0]&0xF0)>>4)*10 + (self->tbc.s[0]&0x0F)*100 + (self->tbc.s[1]&0x0F);
    self->MNC = (self->tbc.s[2]&0x0F)*10 + ((self->tbc.s[2]&0xF0)>>4);
    if( ((self->tbc.s[1]&0xF0)>>4) != 0xF){
        self->MNC+= ((self->tbc.s[1]&0xF0)>>4)*100;
    }
}

void plmnId_MccMnc2tbcd(PLMNidentity_t* self){
    self->tbc.s[0] = ((self->MCC%100)/10<<4) | self->MCC/100;
    self->tbc.s[1] = (self->MCC%10);
    if(self->MNC/100 == 0){
        self->tbc.s[1]|=0xf0;
        self->tbc.s[2] = (self->MNC/10) | (self->MNC%10)<<4;
    }else{
        self->tbc.s[1]|= (self->MNC%10)<<4;
        self->tbc.s[2] = ((self->MNC%100)/10<<4) | self->MNC/100;
    }
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 *
 * This type (Telephony Binary Coded Decimal String) is used to
 * represent several digits from 0 through 9, *, #, a, b, c, two
 * digits per octet, each digit encoded 0000 to 1001 (0 to 9),
 * 1010 (*), 1011 (#), 1100 (a), 1101 (b) or 1110 (c); 1111 used
 * as filler when there is an odd number of digits.

 * */
void show_PLMNidentity(void* data){

    PLMNidentity_t *self = (PLMNidentity_t*)data;
    if(self->MCC == 0 && self->MNC == 0){
        plmnId_tbcd2MccMnc(self);
    }
    printf("PLMN identity: MCC=%u MNC=%u ", self->MCC, self->MNC);
}

/** @brief Constructor of PLMNidentity type
 *  @return PLMNidentity_t allocated  and initialized structure
 * */
PLMNidentity_t *new_PLMNidentity(){
    PLMNidentity_t *self;

    self = malloc(sizeof(PLMNidentity_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP Global_ENB_ID_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(PLMNidentity_t));

    self->freeIE=free_PLMNidentity;
    self->showIE=show_PLMNidentity;

    return self;
}


/* ************************** eNB-ID ************************** */
/** @brief ENB_ID IE Destructor
 *
 * Deallocate the ENB_ID_t structure.
 * */
void free_ENB_ID(void* data){
    ENB_ID_t *self = (ENB_ID_t*)data;
    if(!self){
        return;
    }
    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_ENB_ID(void* data){
    ENB_ID_t *self = (ENB_ID_t*)data;

    printf("\t\t\tENB-ID: ");

    if(self->choice==0){
        printf("\tMacro eNB ID=%#.5x\n", self->id.macroENB_ID);
    }else{
        printf("\tHome eNB ID  ID=%#.7x\n", self->id.homeENB_ID);
    }
}

/** @brief Constructor of ENB_ID type
 *  @return ENB_ID_t allocated  and initialized structure
 * */
ENB_ID_t *new_ENB_ID(){
    ENB_ID_t *self;

    self = malloc(sizeof(ENB_ID_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP ENB_ID_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(ENB_ID_t));


    /*self->cloneIE=clone_ENB_ID;*/
    self->freeIE=free_ENB_ID;
    self->showIE=show_ENB_ID;

    /*printf("\ntest new_ENB_ID() self(%p)->freeIE(%p)\n", self, *self->freeIE);
    printf("test new_ENB_ID() self(%p)->showIE(%p)\n\n", self, *self->showIE);*/

    return self;
}


/* ************************ MME_Group_ID ********************** */
/** @brief MME_Group_ID IE Destructor
 *
 * Deallocate the MME_Group_ID_t structure.
 * */
void free_MME_Group_ID(void * data){
    MME_Group_ID_t *self = (MME_Group_ID_t*)data;
    if(!self){
        return;
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_MME_Group_ID(void * data){
    MME_Group_ID_t *self = (MME_Group_ID_t*)data;
    printf("\t\t\tMME-Group-ID = %#x %#x\n", self->s[0], self->s[1]);
}

/** @brief Constructor of MME_Group_ID type
 *  @return MME_Group_ID_t allocated  and initialized structure
 * */
MME_Group_ID_t *new_MME_Group_ID(){
    MME_Group_ID_t *self;

    self = malloc(sizeof(MME_Group_ID_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP MME_Group_ID_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(MME_Group_ID_t));

    self->freeIE=free_MME_Group_ID;
    self->showIE=show_MME_Group_ID;

    return self;
}


/* ************************** MME-Code ************************ */
/** @brief MME_Code IE Destructor
 *
 * Deallocate the MME_Code_t structure.
 * */
void free_MME_Code(void * data){
    MME_Code_t *self = (MME_Code_t*)data;
    if(!self){
        return;
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_MME_Code(void * data){
    MME_Code_t *self = (MME_Code_t*)data;
    printf("\t\t\tMME-Code = %#x\n", self->s[0]);
}

/** @brief Constructor of MME_Code type
 *  @return MME_Code_t allocated  and initialized structure
 * */
MME_Code_t *new_MME_Code(){
    MME_Code_t *self;

    self = malloc(sizeof(MME_Code_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP MME_Code_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(MME_Code_t));

    self->freeIE=free_MME_Code;
    self->showIE=show_MME_Code;

    return self;
}



/* ************************************************************ */
/* ************************* IE Types ************************* */
/* ************************************************************ */

/* *********************** Global-ENB-ID ********************** */
/** @brief Global_ENB_ID IE Destructor
 *
 * Deallocate the Global_ENB_ID_t structure.
 * */
void free_Global_ENB_ID(void * data){
    Global_ENB_ID_t *self = (Global_ENB_ID_t*)data;

    if(!self){
        return;
    }

    if(self->pLMNidentity){
        self->pLMNidentity->freeIE(self->pLMNidentity);
    }

    if(self->eNBid){
        /*printf("test before self(%p)->eNBid(%p)->freeIE(%p)\n", self, self->eNBid, *self->eNBid->freeIE);*/
        self->eNBid->freeIE(self->eNBid);
    }

    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }
    free(self);

}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_Global_ENB_ID(void * data){
    Global_ENB_ID_t *self = (Global_ENB_ID_t*)data;

    printf("\t\t\tGlobal-ENB-ID: ");

    if(!self->pLMNidentity)
        printf("\t\tpLMNidentity %p, self %p\n", self->pLMNidentity, self);

    if(self->pLMNidentity){
        printf("\t");
        self->pLMNidentity->showIE(self->pLMNidentity);
        printf("\n");
    }
    if(self->eNBid){
        self->eNBid->showIE(self->eNBid);
        /*printf("\ntest show_Global_ENB_ID() self(%p)->eNBid(%p)->showIE(%p)\n\n", self, self->eNBid, *self->eNBid->showIE);*/
    }
}

/* @brief Clone IE information
 *
 * Tool function to clone the IE
 * *//*
Global_ENB_ID_t *clone_Global_ENB_ID(Global_ENB_ID_t * self){
    Global_ENB_ID_t *clon = new_Global_ENB_ID();

    if(!self->pLMNidentity)
        clon->pLMNidentity = self->pLMNidentity->cloneIE(self->pLMNidentity);

    if(!self->eNBid)
        clon->eNBid = self->eNBid->cloneIE(self->eNBid);
}*/

/** @brief Constructor of Global_ENB_ID type
 *  @return Global_ENB_ID_t allocated  and initialized structure
 * */
Global_ENB_ID_t *new_Global_ENB_ID(){
    Global_ENB_ID_t *self;

    self = malloc(sizeof(Global_ENB_ID_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP Global_ENB_ID_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(Global_ENB_ID_t));

    self->eNBid=new_ENB_ID();
    self->iEext=NULL;

    /*self->cloneIE=clone_Global_ENB_ID;*/
    self->freeIE=free_Global_ENB_ID;
    self->showIE=show_Global_ENB_ID;

    return self;
}


/* ************************** ENBname ************************* */
/** @brief ENBname IE Destructor
 *
 * Deallocate the ENBname_t structure.
 * */
void free_ENBname(void * data){
    ENBname_t *self = (ENBname_t*)data;
    if(!self){
        return;
    }

    if(self->extension!=NULL){
        free(self->extension);
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_ENBname(void * data){
    ENBname_t *self = (ENBname_t*)data;
    printf("\t\t\teNB Name=\"%s\"\n", self->name);
}

/** @brief Constructor of ENBname type
 *  @return ENBname_t allocated  and initialized structure
 * */
ENBname_t *new_ENBname(){
    ENBname_t *self;

    self = malloc(sizeof(ENBname_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP ENBname_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(ENBname_t));

    self->freeIE=free_ENBname;
    self->showIE=show_ENBname;
    return self;
}


/* ************************** MMEname ************************* */
/** @brief MMEname IE Destructor
 *
 * Deallocate the MMEname_t structure.
 * */
void free_MMEname(void * data){
    MMEname_t *self = (MMEname_t*)data;
    if(!self){
        return;
    }

    if(self->extension!=NULL){
        free(self->extension);
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_MMEname(void * data){
    MMEname_t *self = (MMEname_t*)data;
    printf("\t\t\tMME Name=\"%s\"\n", self->name);
}

/** @brief Constructor of MMEname type
 *  @return MMEname_t allocated  and initialized structure
 * */
MMEname_t *new_MMEname(){
    MMEname_t *self;

    self = malloc(sizeof(MMEname_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP MMEname_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(MMEname_t));

    self->freeIE=free_MMEname;
    self->showIE=show_MMEname;
    return self;
}


/* ************************** BPLMNs ************************** */
/** @brief BPLMNs IE Destructor
 *
 * Deallocate the BPLMNs_t structure.
 * */
void free_BPLMNs(void * data){
    uint16_t i;

    BPLMNs_t *self = (BPLMNs_t*)data;
    if(!self){
        return;
    }

    for(i=0; i<self->n;i++){
        if(self->pLMNidentity[i]->freeIE){
            self->pLMNidentity[i]->freeIE(self->pLMNidentity[i]);
        }
    }
    free(self->pLMNidentity);
    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_BPLMNs(void * data){
    BPLMNs_t *self = (BPLMNs_t*)data;
    uint16_t i;

    /*printf("\tTotal pLMNidentities=%u\n", self->size);*/
    if(self->size!=self->n){
        printf("The expected (%u) and current number (%u) of IE is different", self->size, self->n);
    }
    for(i=0; i < self->n; i++){
        if(&(self->pLMNidentity[i]) == NULL){
            printf("\nItem #%u not found\n", i);
            continue;
        }
        if(self->pLMNidentity[i]->showIE){
            /*printf("\t\tItem #%u ", i);*/
            self->pLMNidentity[i]->showIE(self->pLMNidentity[i]);
        }else{
            printf("\t\tItem #%u: show function not found\n", i);
        }
    }

}

void BPLMNs_addPLMNid(BPLMNs_t* c, PLMNidentity_t* item){
    PLMNidentity_t** vector;
    if(c->n+1==maxnoofTACs){
        s1ap_msg(ERROR, 0, "maxnoofTACs reached");
        return;
    }

    c->n++;
    vector = (PLMNidentity_t**) realloc (c->pLMNidentity, c->n * sizeof(PLMNidentity_t*));

    /*Error Check*/
    if (vector!=NULL) {
        c->pLMNidentity=vector;
        c->pLMNidentity[c->n-1]=item;
        /*printf("\t\t\t*added PLMNidentity_t %p\n", c->pLMNidentity[c->n-1]);*/
        /*c->pLMNidentity[c->n-1]->showIE(c->pLMNidentity[c->n-1]);*/
    }
    else {
        free (c->showIE);
        s1ap_msg(ERROR, 0, "Error (re)allocating memory");
    }
}

/** @brief Constructor of BPLMNs type
 *  @return BPLMNs_t allocated  and initialized structure
 * */
BPLMNs_t *new_BPLMNs(){
    BPLMNs_t *self;

    self = malloc(sizeof(BPLMNs_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP BPLMNs_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(BPLMNs_t));

    self->freeIE=free_BPLMNs;
    self->showIE=show_BPLMNs;
    self->addPLMNid=BPLMNs_addPLMNid;

    return self;
}


/* ********************* SupportedTAs-Item ******************** */
/** @brief SupportedTAs_Item IE Destructor
 *
 * Deallocate the SupportedTAs_Item_t structure.
 * */
void free_SupportedTAs_Item(void * data){
    SupportedTAs_Item_t *self = (SupportedTAs_Item_t*)data;
    if(!self){
        return;
    }

    if(self->tAC){
        free(self->tAC);
    }

    if(self->broadcastPLMNs){
        self->broadcastPLMNs->freeIE(self->broadcastPLMNs);
    }

    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_SupportedTAs_Item(void * data){
    SupportedTAs_Item_t *self = (SupportedTAs_Item_t*)data;

    if(self->tAC->s != NULL){
        printf("\t\t\tTAC= %u %u ", self->tAC->s[0], self->tAC->s[1]);
    }

    if(self->broadcastPLMNs != NULL){
        self->broadcastPLMNs->showIE(self->broadcastPLMNs);
    }
    printf("\n");
/*
    if(self->iEext != NULL){
        printf("test1\n");
        self->iEext->showExtensionContainer(self->iEext);
    }*/
}

/** @brief Constructor of SupportedTAs_Item type
 *  @return SupportedTAs_Item_t allocated  and initialized structure
 * */
SupportedTAs_Item_t *new_SupportedTAs_Item(){
    SupportedTAs_Item_t *self;

    self = malloc(sizeof(SupportedTAs_Item_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP SupportedTAs_Item_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(SupportedTAs_Item_t));

    self->tAC = malloc(sizeof(TAC_t));
    if(!self->tAC){
        s1ap_msg(ERROR, 0, "S1AP TAC_t not allocated correctly");
        free(self);
        return NULL;
    }
    memset(self, 0, sizeof(TAC_t));

    self->freeItem=free_SupportedTAs_Item;
    self->showItem=show_SupportedTAs_Item;

    return self;
}


/* *********************** SupportedTAs *********************** */
/** @brief SupportedTAs IE Destructor
 *
 * Deallocate the SupportedTAs_t structure.
 * */
void free_SupportedTAs(void * data){
    uint16_t i;

    SupportedTAs_t *self = (SupportedTAs_t*)data;
    if(!self){
        return;
    }

    for(i=0; i<self->size;i++){
        if(self->item[i]->freeItem){
            self->item[i]->freeItem(self->item[i]);
        }
    }
    free(self->item);
    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_SupportedTAs(void * data){
    SupportedTAs_t *self = (SupportedTAs_t*)data;
    uint16_t i;

    /*printf("\tTotal SupportedTAs=%u\n", self->size);*/

    for(i=0; i < self->size; i++){
        if(&(self->item[i]) == NULL){
            printf("\nSupportedTAs Item #%u not found\n", i);
            continue;
        }
        if(self->item[i]->showItem){
            /*printf("\t\tItem #%u:", i);*/
            self->item[i]->showItem(self->item[i]);
        }else{
            printf("\t\tSupportedTAs Item #%u: show function not found\n", i);
        }
    }

}

void SupportedTAs_addItem(SupportedTAs_t* c, SupportedTAs_Item_t* item){
    SupportedTAs_Item_t** vector;
    if(c->size+1==maxnoofTACs){
        s1ap_msg(ERROR, 0, "maxnoofTACs reached");
        return;
    }

    c->size++;
    vector = (SupportedTAs_Item_t**) realloc (c->item, c->size * sizeof(SupportedTAs_Item_t*));

    /*Error Check*/
    if (vector!=NULL) {
        c->item=vector;
        c->item[c->size-1]=item;
    }
    else {
      free (c->item);
      s1ap_msg(ERROR, 0, "Error (re)allocating memory");
    }
}

/** @brief Constructor of SupportedTAs type
 *  @return SupportedTAs_t allocated  and initialized structure
 * */
SupportedTAs_t *new_SupportedTAs(){
    SupportedTAs_t *self;

    self = malloc(sizeof(SupportedTAs_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP SupportedTAs_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(SupportedTAs_t));

    self->freeIE=free_SupportedTAs;
    self->showIE=show_SupportedTAs;
    self->additem=SupportedTAs_addItem;

    return self;
}

/* ************************* PagingDRX ************************ */


/* ************************* PagingDRX ************************ */
/** @brief PagingDRX IE Destructor
 *
 * Deallocate the PagingDRX_t structure.
 * */
void free_PagingDRX(void * data){
    PagingDRX_t *self = (PagingDRX_t*)data;
    if(!self){
        return;
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */

void show_PagingDRX(void * data){
    PagingDRX_t *self = (PagingDRX_t*)data;
    printf("\t\t\tPagingDRXName = %s\n", PagingDRXName[self->pagingDRX]);
}

/** @brief Constructor of PagingDRX type
 *  @return PagingDRX_t allocated  and initialized structure
 * */

PagingDRX_t *new_PagingDRX(){
    PagingDRX_t *self;

    self = malloc(sizeof(PagingDRX_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP PagingDRX_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(PagingDRX_t));

    self->freeIE=free_PagingDRX;
    self->showIE=show_PagingDRX;

    return self;
}

/* ************************** CSG-Id ************************** */


/* ************************** CSG_id ************************** */
/** @brief CSG_id IE Destructor
 *
 * Deallocate the cSG_id_t structure.
 * */
void free_CSG_id(void * data){
    cSG_id_t *self = (cSG_id_t*)data;
    if(!self){
        return;
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */

void show_CSG_id(void * data){
    cSG_id_t *self = (cSG_id_t*)data;

    printf("\t\tCSG-Id = %#x\n", self->id);
}

/** @brief Constructor of cSG_id_t type
 *  @return cSG_id_t allocated  and initialized structure
 * */

cSG_id_t *new_CSG_id(){
    cSG_id_t *self;

    self = malloc(sizeof(cSG_id_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP cSG_id_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(cSG_id_t));

    self->freeIE=free_CSG_id;
    self->showIE=show_CSG_id;

    return self;
}

/* ********************** CSG-IdList-Item ********************* */


/* ********************** CSG_IdList_Item ********************* */
/** @brief CSG_IdList_Item IE Destructor
 *
 * Deallocate the CSG_IdList_Item_t structure.
 * */
void free_CSG_IdList_Item(void * data){
    CSG_IdList_Item_t *self = (CSG_IdList_Item_t*)data;
    if(!self){
        return;
    }

    if(self->cSG_id){
        self->cSG_id->freeIE(self->cSG_id);
    }

    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_CSG_IdList_Item(void * data){
    CSG_IdList_Item_t *self = (CSG_IdList_Item_t*)data;

    if(self->cSG_id){
        self->cSG_id->showIE(self->cSG_id);
    }
/*
    if(self->iEext != NULL){
        self->iEext->showExtensionContainer(self->iEext);
    }*/
}

/** @brief Constructor of CSG-IdList-Item type
 *  @return CSG_IdList_Item_t allocated  and initialized structure
 * */
CSG_IdList_Item_t *new_CSG_IdList_Item(){
    CSG_IdList_Item_t *self;

    self = malloc(sizeof(CSG_IdList_Item_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP CSG_IdList_Item_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(CSG_IdList_Item_t));

    self->freeItem=free_CSG_IdList_Item;
    self->showItem=show_CSG_IdList_Item;

    return self;
}


/* ************************ CSG_IdList ************************ */
/** @brief CSG_IdList IE Destructor
 *
 * Deallocate the CSG_IdList_t structure.
 * */
void free_CSG_IdList(void * data){
    uint16_t i;
    CSG_IdList_t *self = (CSG_IdList_t*)data;
    if(!self){
        return;
    }

    for(i=0; i<self->size;i++){
        if(self->item[i]->freeItem){
            self->item[i]->freeItem(self->item[i]);
        }
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_CSG_IdList(void * data){
    CSG_IdList_t *self = (CSG_IdList_t*)data;
    uint16_t i;

    /*printf("\tTotal SupportedTAs=%u\n", self->size);*/
    for(i=0; i < self->size; i++){
        if(&(self->item[i]) == NULL){
            printf("\nCSG_Id Item #%u not found\n", i);
            continue;
        }
        if(self->item[i]->showItem){
            /*printf("\t\tItem #%u:", i);*/
            self->item[i]->showItem(self->item[i]);
        }else{
            printf("\t\tCSG_Id Item #%u: show function not found\n", i);
        }
    }

}

void CSG_IdList_addItem(CSG_IdList_t* c, CSG_IdList_Item_t* item){
    CSG_IdList_Item_t** vector;
    if(c->size+1==maxNrOfCSGs){
        s1ap_msg(ERROR, 0, "maxNrOfCSGs reached");
        return;
    }

    c->size++;
    vector = (CSG_IdList_Item_t**) realloc (c->item, c->size * sizeof(CSG_IdList_Item_t*));

    /*Error Check*/
    if (vector!=NULL) {
        c->item=vector;
        c->item[c->size-1]=item;
    }
    else {
      free (c->item);
      s1ap_msg(ERROR, 0, "Error (re)allocating memory");
    }
}

/** @brief Constructor of CSG_IdList type
 *  @return CSG_IdList_t allocated  and initialized structure
 * */
CSG_IdList_t *new_CSG_IdList(){
    CSG_IdList_t *self;

    self = malloc(sizeof(CSG_IdList_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP CSG_IdList_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(CSG_IdList_t));

    self->freeIE=free_CSG_IdList;
    self->showIE=show_CSG_IdList;
    self->additem=CSG_IdList_addItem;

    return self;
}


/* *********************** ServedGroupIDs ********************* */
/** @brief ServedGroupIDs IE Destructor
 *
 * Deallocate the ServedGroupIDs_t structure.
 * */
void free_ServedGroupIDs(void * data){
    uint16_t i;
    ServedGroupIDs_t *self = (ServedGroupIDs_t*)data;
    if(!self){
        return;
    }

    for(i=0; i<self->size;i++){
        if(self->item[i]->freeIE){
            self->item[i]->freeIE(self->item[i]);
        }
    }
    free(self->item);
    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_ServedGroupIDs(void * data){
    ServedGroupIDs_t *self = (ServedGroupIDs_t*)data;
    uint16_t i;

    /*printf("\tTotal SupportedTAs=%u\n", self->size);*/
    for(i=0; i < self->size; i++){
        if(&(self->item[i]) == NULL){
            printf("\nMME_Group_ID Item #%u not found\n", i);
            continue;
        }
        if(self->item[i]->showIE){
            self->item[i]->showIE(self->item[i]);
        }else{
            printf("\t\tMME_Group_ID Item #%u: show function not found\n", i);
        }
    }

}

void ServedGroupIDs_addItem(ServedGroupIDs_t* c, MME_Group_ID_t* item){
    MME_Group_ID_t** vector;
    if(c->size+1==maxnoofGroupIDs){
        s1ap_msg(ERROR, 0, "maxnoofGroupIDs reached");
        return;
    }

    c->size++;
    vector = (MME_Group_ID_t**) realloc (c->item, c->size * sizeof(MME_Group_ID_t*));

    /*Error Check*/
    if (vector!=NULL) {
        c->item=vector;
        c->item[c->size-1]=item;
    }
    else {
        free (c->item);
        s1ap_msg(ERROR, 0, "Error (re)allocating memory");
    }
}

/** @brief Constructor of ServedGroupIDs type
 *  @return ServedGroupIDs_t allocated  and initialized structure
 * */
ServedGroupIDs_t *new_ServedGroupIDs(){
    ServedGroupIDs_t *self;

    self = malloc(sizeof(ServedGroupIDs_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP ServedGroupIDs_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(ServedGroupIDs_t));

    self->freeIE=free_ServedGroupIDs;
    self->showIE=show_ServedGroupIDs;
    self->additem=ServedGroupIDs_addItem;

    return self;
}


/* ************************ ServedMMECs *********************** */
/** @brief ServedMMECs IE Destructor
 *
 * Deallocate the ServedMMECs_t structure.
 * */
void free_ServedMMECs(void * data){
    uint16_t i;
    ServedMMECs_t *self = (ServedMMECs_t*)data;
    if(!self){
        return;
    }

    for(i=0; i<self->size;i++){
        if(self->item[i]->freeIE){
            self->item[i]->freeIE(self->item[i]);
        }
    }
    free(self->item);
    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_ServedMMECs(void * data){
    ServedMMECs_t *self = (ServedMMECs_t*)data;
    uint16_t i;

    for(i=0; i < self->size; i++){
        if(&(self->item[i]) == NULL){
            printf("\t\t\tMME_Code_t Item #%u not found\n", i);
            continue;
        }
        if(self->item[i]->showIE){
            self->item[i]->showIE(self->item[i]);
        }else{
            printf("\t\t\tMME_Code_t Item #%u: show function not found\n", i);
        }
    }

}

void ServedMMECs_addItem(ServedMMECs_t* c, MME_Code_t* item){
    MME_Code_t** vector;
    if(c->size+1==maxnoofGroupIDs){
        s1ap_msg(ERROR, 0, "maxnoofGroupIDs reached");
        return;
    }

    c->size++;
    vector = (MME_Code_t**) realloc (c->item, c->size * sizeof(MME_Code_t*));

    /*Error Check*/
    if (vector!=NULL) {
        c->item=vector;
        c->item[c->size-1]=item;
    }
    else {
      free (c->item);
      s1ap_msg(ERROR, 0, "Error (re)allocating memory");
    }
}

/** @brief Constructor of ServedMMECs type
 *  @return ServedMMECs_t allocated  and initialized structure
 * */
ServedMMECs_t *new_ServedMMECs(){
    ServedMMECs_t *self;

    self = malloc(sizeof(ServedMMECs_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP ServedMMECs_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(ServedMMECs_t));

    self->freeIE=free_ServedMMECs;
    self->showIE=show_ServedMMECs;
    self->additem=ServedMMECs_addItem;

    return self;
}


/* *********************** ServedPLMNs ************************ */
/** @brief ServedPLMNs IE Destructor
 *
 * Deallocate the ServedPLMNs_t structure.
 * */
void free_ServedPLMNs(void * data){
    uint16_t i;
    ServedPLMNs_t *self = (ServedPLMNs_t*)data;
    if(!self){
        return;
    }

    for(i=0; i<self->size;i++){
        if(self->item[i]->freeIE){
            self->item[i]->freeIE(self->item[i]);
        }
    }
    free(self->item);

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_ServedPLMNs(void * data){
    ServedPLMNs_t *self = (ServedPLMNs_t*)data;
    uint16_t i;

    /*printf("\tTotal SupportedTAs=%u\n", self->size);*/
    for(i=0; i < self->size; i++){
        if(&(self->item[i]) == NULL){
            printf("\t\t\tPLMNidentity_t Item #%u not found\n", i);
            continue;
        }
        if(self->item[i]->showIE){
            self->item[i]->showIE(self->item[i]);
        }else{
            printf("\t\t\tPLMNidentity_t Item #%u: show function not found\n", i);
        }
    }

}

void ServedPLMNs_addItem(ServedPLMNs_t* c, PLMNidentity_t* item){
    PLMNidentity_t** vector;
    if(c->size+1==maxnoofGroupIDs){
        s1ap_msg(ERROR, 0, "maxnoofGroupIDs reached");
        return;
    }

    c->size++;
    vector = (PLMNidentity_t**) realloc (c->item, c->size * sizeof(PLMNidentity_t*));

    /*Error Check*/
    if (vector!=NULL) {
        c->item=vector;
        c->item[c->size-1]=item;
    }
    else {
      free (c->item);
      s1ap_msg(ERROR, 0, "Error (re)allocating memory");
    }
}

/** @brief Constructor of ServedPLMNs type
 *  @return ServedPLMNs_t allocated  and initialized structure
 * */
ServedPLMNs_t *new_ServedPLMNs(){
    ServedPLMNs_t *self;

    self = malloc(sizeof(ServedPLMNs_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP ServedPLMNs_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(ServedPLMNs_t));

    self->freeIE=free_ServedPLMNs;
    self->showIE=show_ServedPLMNs;
    self->additem=ServedPLMNs_addItem;

    return self;
}


/* ********************* ServedGUMMEIsItem ******************** */
/** @brief ServedGUMMEIsItem IE Destructor
 *
 * Deallocate the ServedGUMMEIsItem_t structure.
 * */
void free_ServedGUMMEIsItem(void * data){
    ServedGUMMEIsItem_t *self = (ServedGUMMEIsItem_t*)data;
    if(!self){
        return;
    }

    if(self->servedGroupIDs){
        self->servedGroupIDs->freeIE(self->servedGroupIDs);
    }

    if(self->servedMMECs){
        self->servedMMECs->freeIE(self->servedMMECs);
    }

    if(self->servedPLMNs){
        self->servedPLMNs->freeIE(self->servedPLMNs);
    }

    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_ServedGUMMEIsItem(void * data){
    ServedGUMMEIsItem_t *self = (ServedGUMMEIsItem_t*)data;

    if(self->servedPLMNs){
        printf("\t\t\t");
        self->servedPLMNs->showIE(self->servedPLMNs);
        printf("\n");
    }

    if(self->servedGroupIDs){
        self->servedGroupIDs->showIE(self->servedGroupIDs);
    }

    if(self->servedMMECs){
        self->servedMMECs->showIE(self->servedMMECs);
    }

}

/** @brief Constructor of ServedGUMMEIsItem type
 *  @return ServedGUMMEIsItem_t allocated  and initialized structure
 * */
ServedGUMMEIsItem_t *new_ServedGUMMEIsItem(){
    ServedGUMMEIsItem_t *self;

    self = malloc(sizeof(ServedGUMMEIsItem_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP ServedGUMMEIsItem_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(ServedGUMMEIsItem_t));

    /*self->servedGroupIDs=new_ServedGroupIDs();
    self->servedMMECs=new_ServedMMECs();
    self->servedPLMNs=new_ServedPLMNs();*/
    self->iEext=NULL;

    self->freeIE=free_ServedGUMMEIsItem;
    self->showIE=show_ServedGUMMEIsItem;

    return self;
}


/* *********************** ServedGUMMEIs ********************** */
/** @brief ServedGUMMEIs IE Destructor
 *
 * Deallocate the ServedGUMMEIs_t structure.
 * */
void free_ServedGUMMEIs(void * data){
    uint16_t i;
    ServedGUMMEIs_t *self = (ServedGUMMEIs_t*)data;
    if(!self){
        return;
    }

    for(i=0; i<self->size;i++){
        if(self->item[i]->freeIE){
            self->item[i]->freeIE(self->item[i]);
        }
    }
    free(self->item);
    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_ServedGUMMEIs(void * data){
    ServedGUMMEIs_t *self = (ServedGUMMEIs_t*)data;
    uint16_t i;

    for(i=0; i < self->size; i++){
        if(&(self->item[i]) == NULL){
            printf("\t\t\tMME_Code_t Item #%u not found\n", i);
            continue;
        }
        if(self->item[i]->showIE){
            self->item[i]->showIE(self->item[i]);
        }else{
            printf("\t\t\tMME_Code_t Item #%u: show function not found\n", i);
        }
    }

}

void ServedGUMMEIs_addItem(ServedGUMMEIs_t* c, ServedGUMMEIsItem_t* item){
    ServedGUMMEIsItem_t** vector;
    if(c->size+1==maxnoofGroupIDs){
        s1ap_msg(ERROR, 0, "maxnoofGroupIDs reached");
        return;
    }

    c->size++;
    vector = (ServedGUMMEIsItem_t**) realloc (c->item, c->size * sizeof(ServedGUMMEIsItem_t*));

    /*Error Check*/
    if (vector!=NULL) {
        c->item=vector;
        c->item[c->size-1]=item;
    }
    else {
        free (c->item);
        s1ap_msg(ERROR, 0, "Error (re)allocating memory");
    }
}

/** @brief Constructor of ServedGUMMEIs type
 *  @return ServedGUMMEIs_t allocated  and initialized structure
 * */
ServedGUMMEIs_t *new_ServedGUMMEIs(){
    ServedGUMMEIs_t *self;

    self = malloc(sizeof(ServedGUMMEIs_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP ServedGUMMEIs_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(ServedGUMMEIs_t));

    self->freeIE=free_ServedGUMMEIs;
    self->showIE=show_ServedGUMMEIs;
    self->additem=ServedGUMMEIs_addItem;

    return self;
}

/* ******************** RelativeMMECapacity ******************* */
/** @brief RelativeMMECapacity IE Destructor
 *
 * Deallocate the RelativeMMECapacity_t structure.
 * */
void free_RelativeMMECapacity(void * data){
    RelativeMMECapacity_t *self = (RelativeMMECapacity_t*)data;
    if(!self){
        return;
    }

    free(self);
}
/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_RelativeMMECapacity(void * data){
    RelativeMMECapacity_t *self = (RelativeMMECapacity_t*)data;
    printf("\t\t\tRelative MME Capacity = %#x", self->cap);
}

/** @brief Constructor of RelativeMMECapacity type
 *  @return RelativeMMECapacity_t allocated  and initialized structure
 * */
RelativeMMECapacity_t *new_RelativeMMECapacity(){
    RelativeMMECapacity_t *self;

    self = malloc(sizeof(RelativeMMECapacity_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP RelativeMMECapacity_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(RelativeMMECapacity_t));

    self->freeIE=free_RelativeMMECapacity;
    self->showIE=show_RelativeMMECapacity;

    return self;
}

/* ****************** MMERelaySupportIndicator **************** */
/** @brief MMERelaySupportIndicator IE Destructor
 *
 * Deallocate the MMERelaySupportIndicator_t structure.
 * */
void free_MMERelaySupportIndicator(void * data){
    MMERelaySupportIndicator_t *self = (MMERelaySupportIndicator_t*)data;
    if(!self){
        return;
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_MMERelaySupportIndicator(void * data){
    MMERelaySupportIndicator_t *self = (MMERelaySupportIndicator_t*)data;
    if(self->ext==0){
        printf("\t\t\tMME Relay supported\n");
    }else{
        printf("\t\t\tMMERelaySupportIndicator: extension found. value=%u, not available in current version\n", self->ind);
    }
}

/** @brief Constructor of MMERelaySupportIndicator type
 *  @return MMERelaySupportIndicator_t allocated  and initialized structure
 * */
MMERelaySupportIndicator_t *new_MMERelaySupportIndicator(){
    MMERelaySupportIndicator_t *self;

    self = malloc(sizeof(MMERelaySupportIndicator_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP MMERelaySupportIndicator_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(MMERelaySupportIndicator_t));

    self->freeIE=free_MMERelaySupportIndicator;
    self->showIE=show_MMERelaySupportIndicator;

    return self;
}


/* ************** CriticalityDiagnostics-IE-Item ************** */
/** @brief CriticalityDiagnostics_IE_Item IE Destructor
 *
 * Deallocate the CriticalityDiagnostics_IE_Item_t structure.
 * */
void free_CriticalityDiagnostics_IE_Item(void * data){
    CriticalityDiagnostics_IE_Item_t *self = (CriticalityDiagnostics_IE_Item_t*)data;
    if(!self){
        return;
    }

    if(self->iEext!=NULL){
        self->iEext->freeExtensionContainer(self->iEext);
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_CriticalityDiagnostics_IE_Item(void * data){
    CriticalityDiagnostics_IE_Item_t *self = (CriticalityDiagnostics_IE_Item_t*)data;

    if ( (self->opt&0x8)==0x8){
        printf("iECriticality = %s(%u)  ", CriticalityName[self->iECriticality], self->iECriticality);
    }
    if ( (self->opt&0x4)==0x4){
        printf("iE-ID = %s(%u)  ", IEName[self->iE_ID], self->iE_ID);
    }
    if ( (self->opt&0x2)==0x2){
        printf("typeOfError = %s(%u)  ", TypeOfErrorName[self->typeOfError], self->typeOfError);
    }
    if ( (self->opt&0x1)==0x1){
        self->iEext->showExtensionContainer(self->iEext);
    }
    printf("\n");
}

/** @brief Constructor of CriticalityDiagnostics_IE_Item type
 *  @return CriticalityDiagnostics_IE_Item_t allocated  and initialized structure
 * */
CriticalityDiagnostics_IE_Item_t *new_CriticalityDiagnostics_IE_Item(){
    CriticalityDiagnostics_IE_Item_t *self;

    self = malloc(sizeof(CriticalityDiagnostics_IE_Item_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP CriticalityDiagnostics_IE_Item_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(CriticalityDiagnostics_IE_Item_t));

    self->freeIE=free_CriticalityDiagnostics_IE_Item;
    self->showIE=show_CriticalityDiagnostics_IE_Item;

    return self;
}


/* ************** CriticalityDiagnostics-IE-List ************** */
/** @brief CriticalityDiagnostics_IE_List IE Destructor
 *
 * Deallocate the CriticalityDiagnostics_IE_List_t structure.
 * */
void free_CriticalityDiagnostics_IE_List(void * data){
    uint16_t i;
    CriticalityDiagnostics_IE_List_t *self = (CriticalityDiagnostics_IE_List_t*)data;
    if(!self){
        return;
    }

    for(i=0; i<self->size;i++){
        if(self->item[i]->freeIE){
            self->item[i]->freeIE(self->item[i]);
        }
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_CriticalityDiagnostics_IE_List(void * data){
    CriticalityDiagnostics_IE_List_t *self = (CriticalityDiagnostics_IE_List_t*)data;
    uint16_t i;

    for(i=0; i < self->size; i++){
        if(&(self->item[i]) == NULL){
            printf("\t\t\tCriticalityDiagnostics_IE_Item_t Item #%u not found\n", i);
            continue;
        }
        if(self->item[i]->showIE){
            self->item[i]->showIE(self->item[i]);
        }else{
            printf("\t\t\tCriticalityDiagnostics_IE_Item_t Item #%u: show function not found\n", i);
        }
    }
}

void CriticalityDiagnostics_IE_List_addItem(CriticalityDiagnostics_IE_List_t* c, CriticalityDiagnostics_IE_Item_t* item){
    CriticalityDiagnostics_IE_Item_t** vector;
    if(c->size+1==maxNrOfErrors){
        s1ap_msg(ERROR, 0, "maxNrOfErrors reached");
        return;
    }

    c->size++;
    vector = (CriticalityDiagnostics_IE_Item_t**) realloc (c->item, c->size * sizeof(CriticalityDiagnostics_IE_Item_t*));

    /*Error Check*/
    if (vector!=NULL) {
        c->item=vector;
        c->item[c->size-1]=item;
    }
    else {
      free (c->item);
      s1ap_msg(ERROR, 0, "Error (re)allocating memory");
    }
}

/** @brief Constructor of CriticalityDiagnostics_IE_List type
 *  @return CriticalityDiagnostics_IE_List_t allocated  and initialized structure
 * */
CriticalityDiagnostics_IE_List_t *new_CriticalityDiagnostics_IE_List(){
    CriticalityDiagnostics_IE_List_t *self;

    self = malloc(sizeof(CriticalityDiagnostics_IE_List_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP CriticalityDiagnostics_IE_List_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(CriticalityDiagnostics_IE_List_t));

    self->freeIE=free_CriticalityDiagnostics_IE_List;
    self->showIE=show_CriticalityDiagnostics_IE_List;
    self->additem=CriticalityDiagnostics_IE_List_addItem;

    return self;
}


/* ******************* CriticalityDiagnostics ****************** */
/** @brief CriticalityDiagnostics IE Destructor
 *
 * Deallocate the CriticalityDiagnostics_t structure.
 * */
void free_CriticalityDiagnostics(void * data){
    CriticalityDiagnostics_t *self = (CriticalityDiagnostics_t*)data;
    if(!self){
        return;
    }

    if((self->opt&0x2) == 0x2){
        self->iEsCriticalityDiagnostics->freeIE(self->iEsCriticalityDiagnostics);
    }

    if((self->opt&0x1) == 0x1){
        self->iEext->freeExtensionContainer(self->iEext);
    }

    free(self);
}
/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_CriticalityDiagnostics(void * data){
    CriticalityDiagnostics_t *self = (CriticalityDiagnostics_t*)data;

    if((self->opt&0x10) == 0x10){
        printf("procedureCode = %s(%u)  ", elementaryProcedureName[self->procedureCode], self->procedureCode);

    }
    if((self->opt&0x8) == 0x8){
        printf("triggeringMessage = %s(%u)  ", MessageName[self->triggeringMessage], self->triggeringMessage);

    }
    if((self->opt&0x4) == 0x4){
        printf("procedureCriticality = %s(%u)  ", CriticalityName[self->procedureCriticality], self->procedureCriticality);
    }
    if((self->opt&0x2) == 0x2){
        self->iEsCriticalityDiagnostics->showIE(self->iEsCriticalityDiagnostics);
    }

    if((self->opt&0x1) == 0x1){
        self->iEext->showExtensionContainer(self->iEext);
    }
}

/** @brief Constructor of CriticalityDiagnostics type
 *  @return CriticalityDiagnostics_t allocated  and initialized structure
 * */
CriticalityDiagnostics_t *new_CriticalityDiagnostics(){
    CriticalityDiagnostics_t *self;

    self = malloc(sizeof(CriticalityDiagnostics_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP CriticalityDiagnostics_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(CriticalityDiagnostics_t));

    self->freeIE=free_CriticalityDiagnostics;
    self->showIE=show_CriticalityDiagnostics;

    return self;
}


/* *************************** Cause *************************** */
/** @brief Cause IE Destructor
 *
 * Deallocate the Cause_t structure.
 * */
void free_Cause(void * data){
    Cause_t *self = (Cause_t*)data;
    if(!self){
        return;
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_Cause(void * data){
    Cause_t *self = (Cause_t*)data;
    if(self->ext==0){
        switch(self->choice){
        case 0: /*CauseRadioNetwork*/
            if(self->cause.radioNetwork.ext==0){
                printf("\t\t\tCauseRadioNetwork %s(%u)\n", CauseRadioNetworkName[self->cause.radioNetwork.cause.noext] ,self->cause.radioNetwork.cause.noext);
            }
            else{
                printf("\t\t\tCauseRadioNetwork %s(%u)\n", CauseRadioNetwork_extName[self->cause.radioNetwork.cause.ext] ,self->cause.radioNetwork.cause.ext);
            }
            break;
        case 1: /*CauseTransport*/
            if(self->cause.transport.ext==0){
                printf("\t\t\tCauseTransport %s(%u)\n", CauseTransportName[self->cause.transport.cause.noext] ,self->cause.transport.cause.noext);
            }
            else{
                printf("\t\t\tCauseTransport extension detected. Not available in current version.\n");
            }
            break;

        case 2: /*CauseNAS*/
            if(self->cause.nas.ext==0){
                printf("\t\t\tCauseNAS %s(%u)\n", CauseNASName[self->cause.nas.cause.noext] ,self->cause.nas.cause.noext);
            }
            else{
                printf("\t\t\tCauseNAS %s(%u)\n", CauseNAS_extName[self->cause.nas.cause.ext] ,self->cause.nas.cause.ext);
            }
            break;

        case 3: /*CauseProtocol*/
            if(self->cause.protocol.ext==0){
                printf("\t\t\tCauseProtocol %s(%u)\n", CauseProtocolName[self->cause.protocol.cause.noext] ,self->cause.protocol.cause.noext);
            }
            else{
                printf("\t\t\tCauseProtocol extension detected. Not available in current version.\n");
            }
            break;

        case 4: /*CauseMisc*/
            if(self->cause.misc.ext==0){
                printf("\t\t\tCauseMisc %s(%u)\n", CauseMiscName[self->cause.misc.cause.noext] ,self->cause.misc.cause.noext);
            }
            else{
                printf("\t\t\tCauseMisc extension detected. Not available in current version.\n");
            }
            break;
        }
    }else{
        printf("\t\t\tExtension not supported with this protocol version\n");
    }
}

/** @brief Constructor of Cause type
 *  @return Cause_t allocated  and initialized structure
 * */
Cause_t *new_Cause(){
    Cause_t *self;

    self = malloc(sizeof(Cause_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP Cause_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(Cause_t));

    self->freeIE=free_Cause;
    self->showIE=show_Cause;

    return self;
}


/* ************************* TimeToWait ************************ */
/** @brief TimeToWait IE Destructor
 *
 * Deallocate the TimeToWait_t structure.
 * */
void free_TimeToWait(void * data){
    TimeToWait_t *self = (TimeToWait_t*)data;
    if(!self){
        return;
    }

    free(self);
}
/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_TimeToWait(void * data){
    TimeToWait_t *self = (TimeToWait_t*)data;
    if(self->ext==0){
        switch(self->time.noext){
        case 0: printf("\t\t\t TimeToWait v1s\n"); break;
        case 1: printf("\t\t\t TimeToWait v2s\n"); break;
        case 2: printf("\t\t\t TimeToWait v5s\n"); break;
        case 3: printf("\t\t\t TimeToWait v10s\n");break;
        case 4: printf("\t\t\t TimeToWait v20s\n");break;
        case 5: printf("\t\t\t TimeToWait v60s\n");break;
        }
    }
}

/** @brief Constructor of TimeToWait type
 *  @return TimeToWait_t allocated  and initialized structure
 * */
TimeToWait_t *new_TimeToWait(){
    TimeToWait_t *self;

    self = malloc(sizeof(TimeToWait_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP TimeToWait_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(TimeToWait_t));

    self->freeIE=free_TimeToWait;
    self->showIE=show_TimeToWait;

    return self;
}


/* *********************** ENB_UE_S1AP_ID ********************** */
/** @brief ENB_UE_S1AP_ID IE Destructor
 *
 * Deallocate the ENB_UE_S1AP_ID_t structure.
 * */
void free_ENB_UE_S1AP_ID(void * data){
    ENB_UE_S1AP_ID_t *self = (ENB_UE_S1AP_ID_t*)data;
    if(!self){
        return;
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_ENB_UE_S1AP_ID(void * data){
    ENB_UE_S1AP_ID_t *self = (ENB_UE_S1AP_ID_t*)data;
    printf("\t\t\tENB-UE-S1AP-ID = %u\n", self->eNB_id);

}

/** @brief Constructor of ENB_UE_S1AP_ID type
 *  @return ENB_UE_S1AP_ID_t allocated  and initialized structure
 * */
ENB_UE_S1AP_ID_t *new_ENB_UE_S1AP_ID(){
    ENB_UE_S1AP_ID_t *self;

    self = malloc(sizeof(ENB_UE_S1AP_ID_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP ENB_UE_S1AP_ID_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(ENB_UE_S1AP_ID_t));

    self->freeIE=free_ENB_UE_S1AP_ID;
    self->showIE=show_ENB_UE_S1AP_ID;

    return self;
}

/* *********************** MME_UE_S1AP_ID ********************** */
/** @brief MME_UE_S1AP_ID IE Destructor
 *
 * Deallocate the MME_UE_S1AP_ID_t structure.
 * */
void free_MME_UE_S1AP_ID(void * data){
    MME_UE_S1AP_ID_t *self = (MME_UE_S1AP_ID_t*)data;
    if(!self){
        return;
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_MME_UE_S1AP_ID(void * data){
    MME_UE_S1AP_ID_t *self = (MME_UE_S1AP_ID_t*)data;
    printf("\t\t\tMME-UE-S1AP-ID = %u\n", self->mme_id);

}

/** @brief Constructor of MME_UE_S1AP_ID type
 *  @return MME_UE_S1AP_ID_t allocated  and initialized structure
 * */
MME_UE_S1AP_ID_t *new_MME_UE_S1AP_ID(){
    MME_UE_S1AP_ID_t *self;

    self = malloc(sizeof(MME_UE_S1AP_ID_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP MME_UE_S1AP_ID_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(MME_UE_S1AP_ID_t));

    self->freeIE=free_MME_UE_S1AP_ID;
    self->showIE=show_MME_UE_S1AP_ID;

    return self;
}


/* ************************** NAS-PDU ************************** */
/** @brief NAS_PDU IE Destructor
 *
 * Deallocate the NAS_PDU_t structure.
 * *//*
void free_NAS_PDU(void * data){
    NAS_PDU_t *self = (NAS_PDU_t*)data;
    if(!self){
        return;
    }
    if(self->str!=NULL){
        free(self->str);
    }

    free(self);
}*/
/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * *//*
void show_NAS_PDU(void * data){
    NAS_PDU_t *self = (NAS_PDU_t*)data;
    printf("\t\t\tNAS-PDU: %#x %#x .. %#x %#x, len= %u\n", self->str[0], self->str[1], self->str[self->len-2], self->str[self->len-1], self->len);
}
*/
/** @brief Constructor of NAS_PDU type
 *  @return NAS_PDU_t allocated  and initialized structure
 * *//*
NAS_PDU_t *new_NAS_PDU(){
    NAS_PDU_t *self;

    self = malloc(sizeof(NAS_PDU_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP NAS_PDU_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(NAS_PDU_t));

    self->freeIE=free_NAS_PDU;
    self->showIE=show_NAS_PDU;

    return self;
}
*/

/* **************************** TAI **************************** */
/** @brief TAI IE Destructor
 *
 * Deallocate the TAI_t structure.
 * */
void free_TAI(void * data){
    TAI_t *self = (TAI_t*)data;
    if(!self){
        return;
    }

    if(self->pLMNidentity){
        self->pLMNidentity->freeIE(self->pLMNidentity);
    }

    if(self->tAC){
        free(self->tAC);
    }

    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_TAI(void * data){
    TAI_t *self = (TAI_t*)data;

    printf("\t\t\tTAI :");
    if(self->pLMNidentity){
        printf("\t\t");
        self->pLMNidentity->showIE(self->pLMNidentity);
    }

    if(self->tAC){
        printf("\tTAC : %.2x%.2x \n", self->tAC->s[0], self->tAC->s[1]);
    }

    if(self->iEext){
        self->iEext->showExtensionContainer(self->iEext);
    }

}

/** @brief Constructor of TAI type
 *  @return TAI_t allocated  and initialized structure
 * */
TAI_t *new_TAI(){
    TAI_t *self;

    self = malloc(sizeof(TAI_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP TAI_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(TAI_t));

    self->tAC = malloc(sizeof(TAC_t));
    if(!self->tAC){
        s1ap_msg(ERROR, 0, "S1AP TAC_t not allocated correctly");
        free(self);
        return NULL;
    }
    memset(self->tAC, 0, sizeof(TAC_t));

    self->iEext=NULL;

    self->freeIE=free_TAI;
    self->showIE=show_TAI;

    return self;
}


/* ************************ EUTRAN-CGI ************************* */
/** @brief EUTRAN_CGI IE Destructor
 *
 * Deallocate the EUTRAN_CGI_t structure.
 * */
void free_EUTRAN_CGI(void * data){
    EUTRAN_CGI_t *self = (EUTRAN_CGI_t*)data;
    if(!self){
        return;
    }

    if(self->pLMNidentity){
        self->pLMNidentity->freeIE(self->pLMNidentity);
    }

    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_EUTRAN_CGI(void * data){
    EUTRAN_CGI_t *self = (EUTRAN_CGI_t*)data;

    if(self->pLMNidentity){
        printf("\t\t\t");
        self->pLMNidentity->showIE(self->pLMNidentity);
    }


    printf("\tCell-ID : %#x\n", self->cell_ID.id);


    if((self->opt&0x1) == 0x1 && self->iEext!=NULL){
        self->iEext->showExtensionContainer(self->iEext);
    }
}

/** @brief Constructor of EUTRAN_CGI type
 *  @return EUTRAN_CGI_t allocated  and initialized structure
 * */
EUTRAN_CGI_t *new_EUTRAN_CGI(){
    EUTRAN_CGI_t *self;

    self = malloc(sizeof(EUTRAN_CGI_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP EUTRAN_CGI_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(EUTRAN_CGI_t));

    self->freeIE=free_EUTRAN_CGI;
    self->showIE=show_EUTRAN_CGI;

    return self;
}


/* ****************** RRC-Establishment-Cause  ***************** */
/** @brief RRC_Establishment_Cause IE Destructor
 *
 * Deallocate the RRC_Establishment_Cause_t structure.
 * */
void free_RRC_Establishment_Cause(void * data){
    RRC_Establishment_Cause_t *self = (RRC_Establishment_Cause_t*)data;
    if(!self){
        return;
    }

    free(self);
}
/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_RRC_Establishment_Cause(void * data){
    RRC_Establishment_Cause_t *self = (RRC_Establishment_Cause_t*)data;
    if(self->ext==0){
        printf("\t\t\tRRC-Establishment-Cause : %s(%u)", RRC_Establishment_Cause_Name[self->cause.noext], self->cause.noext);
    }else{
        printf("\t\t\tRRC-Establishment-Cause : delay-TolerantAccess(%u)", self->cause.ext);
    }

}

/** @brief Constructor of RRC_Establishment_Cause type
 *  @return RRC_Establishment_Cause_t allocated  and initialized structure
 * */
RRC_Establishment_Cause_t *new_RRC_Establishment_Cause(){
    RRC_Establishment_Cause_t *self;

    self = malloc(sizeof(RRC_Establishment_Cause_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP RRC_Establishment_Cause_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(RRC_Establishment_Cause_t));

    self->freeIE=free_RRC_Establishment_Cause;
    self->showIE=show_RRC_Establishment_Cause;

    return self;
}


/* ****************** UEAggregateMaximumBitrate **************** */
/** @brief UEAggregateMaximumBitrate IE Destructor
 *
 * Deallocate the UEAggregateMaximumBitrate_t structure.
 * */
void free_UEAggregateMaximumBitrate(void * data){
    UEAggregateMaximumBitrate_t *self = (UEAggregateMaximumBitrate_t*)data;
    if(!self){
        return;
    }

    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_UEAggregateMaximumBitrate(void * data){
    UEAggregateMaximumBitrate_t *self = (UEAggregateMaximumBitrate_t*)data;
    printf("\t\t\t UEAggregateMaximumBitrateDL : %" PRIu64 ", uEaggregateMaximumBitRateUL : %" PRIu64 "\n",
            self->uEaggregateMaximumBitRateDL.rate, self->uEaggregateMaximumBitRateUL.rate);

    if(self->opt == 0x1 && self->iEext!=NULL){
        self->iEext->showExtensionContainer(self->iEext);
    }
}

/** @brief Constructor of UEAggregateMaximumBitrate type
 *  @return UEAggregateMaximumBitrate_t allocated  and initialized structure
 * */
UEAggregateMaximumBitrate_t *new_UEAggregateMaximumBitrate(){
    UEAggregateMaximumBitrate_t *self;

    self = malloc(sizeof(UEAggregateMaximumBitrate_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP UEAggregateMaximumBitrate_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(UEAggregateMaximumBitrate_t));

    self->freeIE=free_UEAggregateMaximumBitrate;
    self->showIE=show_UEAggregateMaximumBitrate;

    return self;
}


/* *************** AllocationAndRetentionPriority ************** */
/** @brief AllocationAndRetentionPriority IE Destructor
 *
 * Deallocate the AllocationAndRetentionPriority_t structure.
 * */
void free_AllocationAndRetentionPriority(void * data){
    AllocationAndRetentionPriority_t *self = (AllocationAndRetentionPriority_t*)data;
    if(!self){
        return;
    }

    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_AllocationAndRetentionPriority(void * data){
    AllocationAndRetentionPriority_t *self = (AllocationAndRetentionPriority_t*)data;

    printf("\t\t\tPriority Level : %u\n\t\t\tpre-emptionCapability : %s(%u)\n\t\t\tpre-emptionVulnerability : %s(%u)\n",
            self->priorityLevel,
            Pre_emptionCapabilityName[self->pre_emptionCapability], self->pre_emptionCapability,
            Pre_emptionVulnerabilityName[self->pre_emptionVulnerability], self->pre_emptionVulnerability);

    if(self->opt == 0x1 && self->iEext!=NULL){
        self->iEext->showExtensionContainer(self->iEext);
    }
}

/** @brief Constructor of AllocationAndRetentionPriority type
 *  @return AllocationAndRetentionPriority_t allocated  and initialized structure
 * */
AllocationAndRetentionPriority_t *new_AllocationAndRetentionPriority(){
    AllocationAndRetentionPriority_t *self;

    self = malloc(sizeof(AllocationAndRetentionPriority_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP AllocationAndRetentionPriority_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(AllocationAndRetentionPriority_t));

    self->freeIE=free_AllocationAndRetentionPriority;
    self->showIE=show_AllocationAndRetentionPriority;

    return self;
}


/* ***************** GBR_QosInformation ************************ */
/** @brief GBR_QosInformation IE Destructor
 *
 * Deallocate the GBR_QosInformation_t structure.
 * */
void free_GBR_QosInformation(void * data){
    GBR_QosInformation_t *self = (GBR_QosInformation_t*)data;
    if(!self){
        return;
    }

    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_GBR_QosInformation(void * data){
    GBR_QosInformation_t *self = (GBR_QosInformation_t*)data;

    printf("\t\t\te-RAB-MaximumBitrateDL : %" PRIu64 ", e-RAB-MaximumBitrateUL : %" PRIu64 ",\n"
            "\t\t\te-RAB-GuaranteedBitrateDL : %" PRIu64 ", e-RAB-GuaranteedBitrateUL : %" PRIu64 "\n",
            self->eRAB_GuaranteedBitrateDL.rate, self->eRAB_GuaranteedBitrateUL.rate,
            self->eRAB_MaximumBitrateDL.rate, self->eRAB_MaximumBitrateUL.rate);

    if(self->opt == 0x1 && self->iEext!=NULL){
        self->iEext->showExtensionContainer(self->iEext);
    }
}

/** @brief Constructor of GBR_QosInformation type
 *  @return GBR_QosInformation_t allocated  and initialized structure
 * */
GBR_QosInformation_t *new_GBR_QosInformation(){
    GBR_QosInformation_t *self;

    self = malloc(sizeof(GBR_QosInformation_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP GBR_QosInformation_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(GBR_QosInformation_t));

    self->freeIE=free_GBR_QosInformation;
    self->showIE=show_GBR_QosInformation;

    return self;
}


/* ***************** E_RABLevelQoSParameters ******************* */
/** @brief E_RABLevelQoSParameters IE Destructor
 *
 * Deallocate the E_RABLevelQoSParameters_t structure.
 * */
void free_E_RABLevelQoSParameters(void * data){
    E_RABLevelQoSParameters_t *self = (E_RABLevelQoSParameters_t*)data;
    if(!self){
        return;
    }

    if(self->allocationRetentionPriority){
        self->allocationRetentionPriority->freeIE(self->allocationRetentionPriority);
    }

    if(self->gbrQosInformation){
        self->gbrQosInformation->freeIE(self->gbrQosInformation);
    }

    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_E_RABLevelQoSParameters(void * data){
    E_RABLevelQoSParameters_t *self = (E_RABLevelQoSParameters_t*)data;
    printf("\t\t\t\tQCI : %u\n", self->qCI);

    if(self->allocationRetentionPriority!=NULL){
        printf("\t\t\t\tallocationRetentionPriority : \n");
        self->allocationRetentionPriority->showIE(self->allocationRetentionPriority);
    }

    if(self->gbrQosInformation!=NULL){
        printf("\t\t\t\tgbrQosInformation : \n");
        self->gbrQosInformation->showIE(self->gbrQosInformation);
    }

    if(self->opt == 0x1 && self->iEext!=NULL){
        printf("\t\t\t\tExtensionContainer : \n");
        self->iEext->showExtensionContainer(self->iEext);
    }
}

/** @brief Constructor of E_RABLevelQoSParameters type
 *  @return E_RABLevelQoSParameters_t allocated  and initialized structure
 * */
E_RABLevelQoSParameters_t *new_E_RABLevelQoSParameters(){
    E_RABLevelQoSParameters_t *self;

    self = malloc(sizeof(E_RABLevelQoSParameters_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP E_RABLevelQoSParameters_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(E_RABLevelQoSParameters_t));

    self->freeIE=free_E_RABLevelQoSParameters;
    self->showIE=show_E_RABLevelQoSParameters;

    return self;
}


/* ******************** TransportLayerAddress ****************** */
/** @brief TransportLayerAddress IE Destructor
 *
 * Deallocate the TransportLayerAddress_t structure.
 * */
void free_TransportLayerAddress(void * data){
    TransportLayerAddress_t *self = (TransportLayerAddress_t*)data;
    if(!self){
        return;
    }

    if(self->ext == 1 && self->extension != NULL ){
        free(self->extension);
    }

    free(self);
}
/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_TransportLayerAddress(void * data){
    uint8_t i;
    TransportLayerAddress_t *self = (TransportLayerAddress_t*)data;
    printf("\t\t\tTransportLayerAddress : ");
    for( i=0; i<self->len/8; i++){
        printf("%u.", self->addr[i]);
    }
    if(self->ext==1){
        for( i=0; i<self->len/8; i++){
            printf("%#x ", self->addr[i]);
        }
    }
    printf("\n");
}

/** @brief Constructor of TransportLayerAddress type
 *  @return TransportLayerAddress_t allocated  and initialized structure
 * */
TransportLayerAddress_t *new_TransportLayerAddress(){
    TransportLayerAddress_t *self;

    self = malloc(sizeof(TransportLayerAddress_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP TransportLayerAddress_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(TransportLayerAddress_t));

    self->freeIE=free_TransportLayerAddress;
    self->showIE=show_TransportLayerAddress;

    return self;
}


/* **************** E_RABToBeSetupItemCtxtSUReq **************** */
/** @brief E_RABToBeSetupItemCtxtSUReq IE Destructor
 *
 * Deallocate the E_RABToBeSetupItemCtxtSUReq_t structure.
 * */
void free_E_RABToBeSetupItemCtxtSUReq(void * data){
    E_RABToBeSetupItemCtxtSUReq_t *self = (E_RABToBeSetupItemCtxtSUReq_t*)data;
    if(!self){
        return;
    }

    if(self->eRABlevelQoSParameters){
        self->eRABlevelQoSParameters->freeIE(self->eRABlevelQoSParameters);
    }

    if(self->transportLayerAddress){
        self->transportLayerAddress->freeIE(self->transportLayerAddress);
    }

    if(self->nAS_PDU){
        self->nAS_PDU->freeIE(self->nAS_PDU);
    }

    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_E_RABToBeSetupItemCtxtSUReq(void * data){
    E_RABToBeSetupItemCtxtSUReq_t *self = (E_RABToBeSetupItemCtxtSUReq_t*)data;

    printf("\t\t\teRAB_ID : %u\n", self->eRAB_ID.id);

    if(self->eRABlevelQoSParameters!=NULL){
        printf("\t\t\t*E-RABlevelQoSParameters: \n");
        self->eRABlevelQoSParameters->showIE(self->eRABlevelQoSParameters);
    }

    if(self->transportLayerAddress!=NULL){
        printf("\t\t\t*TransportLayerAddress: \n");
        self->transportLayerAddress->showIE(self->transportLayerAddress);
    }

    printf("\t\t\tGTP_TEID : %#x %#x %#x %#x\n",
                self->gTP_TEID.teid[0], self->gTP_TEID.teid[1], self->gTP_TEID.teid[2], self->gTP_TEID.teid[3]);

    if( (self->opt&0x80)== 0x80 && self->nAS_PDU != NULL){
        self->nAS_PDU->showIE(self->nAS_PDU);
    }

    if((self->opt&0x40)== 0x40 && self->iEext!=NULL){
        self->iEext->showExtensionContainer(self->iEext);
    }
}

/** @brief Constructor of E_RABToBeSetupItemCtxtSUReq type
 *  @return E_RABToBeSetupItemCtxtSUReq_t allocated  and initialized structure
 * */
E_RABToBeSetupItemCtxtSUReq_t *new_E_RABToBeSetupItemCtxtSUReq(){
    E_RABToBeSetupItemCtxtSUReq_t *self;

    self = malloc(sizeof(E_RABToBeSetupItemCtxtSUReq_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP E_RABToBeSetupItemCtxtSUReq_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(E_RABToBeSetupItemCtxtSUReq_t));

    self->freeIE=free_E_RABToBeSetupItemCtxtSUReq;
    self->showIE=show_E_RABToBeSetupItemCtxtSUReq;

    return self;
}


/* ************** E_RABToBeSetupListCtxtSUReq ****************** */
/** @brief E_RABToBeSetupListCtxtSUReq IE Destructor
 *
 * Deallocate the E_RABToBeSetupListCtxtSUReq_t structure.
 * */
void free_E_RABToBeSetupListCtxtSUReq(void * data){
    uint16_t i;
    E_RABToBeSetupListCtxtSUReq_t *self = (E_RABToBeSetupListCtxtSUReq_t*)data;
    if(!self){
        return;
    }

    for(i=0; i<self->size;i++){
        if(self->item[i]->freeIE){
            self->item[i]->freeIE(self->item[i]);
        }
    }
    free(self->item);
    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_E_RABToBeSetupListCtxtSUReq(void * data){
    E_RABToBeSetupListCtxtSUReq_t *self = (E_RABToBeSetupListCtxtSUReq_t*)data;
    S1AP_PROTOCOL_IES_t *item;
    uint16_t i;

    for(i=0; i < self->size; i++){
        if(&(self->item[i]) == NULL){
            printf("\t\t\tE_E_RABToBeSetupItemCtxtSUReq_t_t Item #%u not found\n", i);
            continue;
        }

        item = (S1AP_PROTOCOL_IES_t*) self->item[i];
        if(item->showIE){
            item->showIE(item);
        }else{
            printf("\t\t\tE_E_RABToBeSetupItemCtxtSUReq_t_t Item #%u: show function not found\n", i);
        }
    }

}

void E_RABToBeSetupListCtxtSUReq_addItem(E_RABToBeSetupListCtxtSUReq_t* c, ProtocolIE_SingleContainer_t* item){
    ProtocolIE_SingleContainer_t** vector;
    if(c->size+1==maxNrOfERABs){
        s1ap_msg(ERROR, 0, "maxNrOfE-RABs reached");
        return;
    }

    c->size++;
    vector = (ProtocolIE_SingleContainer_t**) realloc (c->item, c->size * sizeof(ProtocolIE_SingleContainer_t*));

    /*Error Check*/
    if (vector!=NULL) {
        c->item=vector;
        c->item[c->size-1]=item;
    }
    else {
      free (c->item);
      s1ap_msg(ERROR, 0, "Error (re)allocating memory");
    }
}

void *E_RABToBeSetupListCtxtSUReq_newItem(struct E_RABToBeSetupListCtxtSUReq_c* eRABlist){
    S1AP_PROTOCOL_IES_t* ie = newProtocolIE();
    E_RABToBeSetupItemCtxtSUReq_t *eRABitem = new_E_RABToBeSetupItemCtxtSUReq();
    ie->value = eRABitem;
    ie->showValue = eRABitem->showIE;
    ie->freeValue = eRABitem->freeIE;
    ie->id = id_E_RABToBeSetupItemCtxtSUReq;
    ie->presence = optional;
    ie->criticality = reject;
    eRABlist->additem(eRABlist, ie);
    return eRABitem;
}

/** @brief Constructor of E_RABToBeSetupListCtxtSUReq type
 *  @return E_RABToBeSetupListCtxtSUReq_t allocated  and initialized structure
 * */
E_RABToBeSetupListCtxtSUReq_t *new_E_RABToBeSetupListCtxtSUReq(){
    E_RABToBeSetupListCtxtSUReq_t *self;

    self = malloc(sizeof(E_RABToBeSetupListCtxtSUReq_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP E_RABToBeSetupListCtxtSUReq_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(E_RABToBeSetupListCtxtSUReq_t));

    self->freeIE=free_E_RABToBeSetupListCtxtSUReq;
    self->showIE=show_E_RABToBeSetupListCtxtSUReq;
    self->additem=E_RABToBeSetupListCtxtSUReq_addItem;
    self->newItem = E_RABToBeSetupListCtxtSUReq_newItem;

    return self;
}


/* ************************ SecurityKey ************************ */
/** @brief SecurityKey IE Destructor
 *
 * Deallocate the SecurityKey_t structure.
 * */
void free_SecurityKey(void * data){
    SecurityKey_t *self = (SecurityKey_t*)data;
    if(!self){
        return;
    }

    free(self);
}
/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_SecurityKey(void * data){
    uint8_t i;
    SecurityKey_t *self = (SecurityKey_t*)data;

    printf("\t\t\tSecurity Key: ");
    for(i=0; i<32; i++){
        if(i%16==0){
            printf("\n\t\t\t");
        }
        printf("%.2x ", self->key[i]);
    }
    printf("\n");

}

/** @brief Constructor of SecurityKey type
 *  @return SecurityKey_t allocated  and initialized structure
 * */
SecurityKey_t *new_SecurityKey(){
    SecurityKey_t *self;

    self = malloc(sizeof(SecurityKey_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP SecurityKey_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(SecurityKey_t));

    self->freeIE=free_SecurityKey;
    self->showIE=show_SecurityKey;

    return self;
}


/* **************** SubscriberProfileIDforRFP ****************** */
/** @brief SubscriberProfileIDforRFP IE Destructor
 *
 * Deallocate the SubscriberProfileIDforRFP_t structure.
 * */
void free_SubscriberProfileIDforRFP(void * data){
    SubscriberProfileIDforRFP_t *self = (SubscriberProfileIDforRFP_t*)data;
    if(!self){
        return;
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_SubscriberProfileIDforRFP(void * data){
    SubscriberProfileIDforRFP_t *self = (SubscriberProfileIDforRFP_t*)data;
    printf("\t\t\tSubscriberProfileIDforRFP : %u\n", self->subscriberProfileIDforRFP);
}

/** @brief Constructor of SubscriberProfileIDforRFPtype
 *  @return SubscriberProfileIDforRFP_t allocated  and initialized structure
 * */
SubscriberProfileIDforRFP_t *new_SubscriberProfileIDforRFP(){
    SubscriberProfileIDforRFP_t *self;

    self = malloc(sizeof(SubscriberProfileIDforRFP_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP SubscriberProfileIDforRFP_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(SubscriberProfileIDforRFP_t));

    self->freeIE=free_SubscriberProfileIDforRFP;
    self->showIE=show_SubscriberProfileIDforRFP;

    return self;
}


/* ******************* UESecurityCapabilities ****************** */
/** @brief UESecurityCapabilities  Destructor
 *
 * Deallocate the UESecurityCapabilities_t structure.
 * */
void free_UESecurityCapabilities(void * data){
    UESecurityCapabilities_t *self = (UESecurityCapabilities_t*)data;
    if(!self){
        return;
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_UESecurityCapabilities(void * data){
    UESecurityCapabilities_t *self = (UESecurityCapabilities_t*)data;
    printf("\t\t\tEncryptionAlgorithms : %#x\n", self->encryptionAlgorithms.v);
    printf("\t\t\tIntegrityProtectionAlgorithms : %#x\n", self->integrityProtectionAlgorithms.v);

}

/** @brief Constructor of UESecurityCapabilities
 *  @return UESecurityCapabilities_t allocated  and initialized structure
 * */
UESecurityCapabilities_t *new_UESecurityCapabilities(){
    UESecurityCapabilities_t *self;

    self = malloc(sizeof(UESecurityCapabilities_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP UESecurityCapabilities_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(UESecurityCapabilities_t));

    self->freeIE=free_UESecurityCapabilities;
    self->showIE=show_UESecurityCapabilities;

    return self;
}


/* ********************* UERadioCapability ********************* */
/** @brief UERadioCapability IE Destructor
 *
 * Deallocate the UERadioCapability_t structure.
 * */
void free_UERadioCapability(void * data){
    UERadioCapability_t *self = (UERadioCapability_t*)data;
    if(!self){
        return;
    }
    if(self->str!=NULL){
        free(self->str);
    }

    free(self);
}
/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_UERadioCapability(void * data){
    UERadioCapability_t *self = (UERadioCapability_t*)data;
    printf("\t\t\tUERadioCapability: %#x %#x .. %#x %#x, len= %u\n", self->str[0], self->str[1], self->str[self->len-2], self->str[self->len-1], self->len);
}

/** @brief Constructor of UERadioCapability type
 *  @return UERadioCapability_t allocated  and initialized structure
 * */
UERadioCapability_t *new_UERadioCapability(){
    UERadioCapability_t *self;

    self = malloc(sizeof(UERadioCapability_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP UERadioCapability_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(UERadioCapability_t));

    self->freeIE=free_UERadioCapability;
    self->showIE=show_UERadioCapability;

    return self;
}



/* *********************** UE_S1AP_ID_pair ********************* */
/** @brief UE_S1AP_ID_pair  Destructor
 *
 * Deallocate the UE_S1AP_ID_pair_t structure.
 * */
void free_UE_S1AP_ID_pair(void * data){
    UE_S1AP_ID_pair_t *self = (UE_S1AP_ID_pair_t*)data;
    if(!self){
        return;
    }

    if(self->mME_UE_S1AP_ID){
        self->mME_UE_S1AP_ID->freeIE(self->mME_UE_S1AP_ID);
    }
    if(self->eNB_UE_S1AP_ID){
        self->eNB_UE_S1AP_ID->freeIE(self->eNB_UE_S1AP_ID);
    }
    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_UE_S1AP_ID_pair(void * data){
    UE_S1AP_ID_pair_t *self = (UE_S1AP_ID_pair_t*)data;

    if(self->mME_UE_S1AP_ID){
        self->mME_UE_S1AP_ID->showIE(self->mME_UE_S1AP_ID);
    }
    if(self->eNB_UE_S1AP_ID){
        self->eNB_UE_S1AP_ID->showIE(self->eNB_UE_S1AP_ID);
    }
    if(self->iEext){
        self->iEext->showExtensionContainer(self->iEext);
    }

}

/** @brief Constructor of UE_S1AP_ID_pair
 *  @return UE_S1AP_ID_pair_t allocated  and initialized structure
 * */
UE_S1AP_ID_pair_t *new_UE_S1AP_ID_pair(){
    UE_S1AP_ID_pair_t *self;

    self = malloc(sizeof(UE_S1AP_ID_pair_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP UE_S1AP_ID_pair_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(UE_S1AP_ID_pair_t));

    self->eNB_UE_S1AP_ID = new_ENB_UE_S1AP_ID();
    self->mME_UE_S1AP_ID = new_MME_UE_S1AP_ID();

    self->freeIE=free_UE_S1AP_ID_pair;
    self->showIE=show_UE_S1AP_ID_pair;

    return self;
}



/* *********************** UE-S1AP-IDs ************************* */
/** @brief UE_S1AP_IDs IE Destructor
 *
 * Deallocate the UE_S1AP_IDs_t structure.
 * */
void free_UE_S1AP_IDs(void* data){
    UE_S1AP_IDs_t *self = (UE_S1AP_IDs_t*)data;
    if(!self){
        return;
    }

    if(self->choice==0){
        self->uE_S1AP_ID.uE_S1AP_ID_pair->freeIE(self->uE_S1AP_ID.uE_S1AP_ID_pair);
    }else{
        self->uE_S1AP_ID.mME_UE_S1AP_ID->freeIE(self->uE_S1AP_ID.mME_UE_S1AP_ID);
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_UE_S1AP_IDs(void* data){
    UE_S1AP_IDs_t *self = (UE_S1AP_IDs_t*)data;

    if(self->choice==0){
        self->uE_S1AP_ID.uE_S1AP_ID_pair->showIE(self->uE_S1AP_ID.uE_S1AP_ID_pair);
    }else{
        self->uE_S1AP_ID.mME_UE_S1AP_ID->showIE(self->uE_S1AP_ID.mME_UE_S1AP_ID);
    }
}

/** @brief Constructor of UE_S1AP_IDs type
 *  @return UE_S1AP_IDs_t allocated  and initialized structure
 * */
UE_S1AP_IDs_t *new_UE_S1AP_IDs(){
    UE_S1AP_IDs_t *self;

    self = malloc(sizeof(UE_S1AP_IDs_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP UE_S1AP_IDs_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(UE_S1AP_IDs_t));

    self->freeIE=free_UE_S1AP_IDs;
    self->showIE=show_UE_S1AP_IDs;

    /*printf("\ntest new_UE_S1AP_IDs() self(%p)->freeIE(%p)\n", self, *self->freeIE);
    printf("test new_UE_S1AP_IDs() self(%p)->showIE(%p)\n\n", self, *self->showIE);*/

    return self;
}



/* ************************* COUNTvalue ************************ */
/** @brief COUNTvalue  Destructor
 *
 * Deallocate the COUNTvalue_t structure.
 * */
void free_COUNTvalue(void * data){
    COUNTvalue_t *self = (COUNTvalue_t*)data;
    if(!self){
        return;
    }

    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }
    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_COUNTvalue(void * data){
    COUNTvalue_t *self = (COUNTvalue_t*)data;

    printf("\t\t\tPDCP-SN : %#x,\tHFN : %#x\n", self->pDCP_SN.pDCP_SN, self->hFN.hFN);

    if(self->iEext){
        self->iEext->showExtensionContainer(self->iEext);
    }
}

/** @brief Constructor of COUNTvalue
 *  @return COUNTvalue_t allocated  and initialized structure
 * */
COUNTvalue_t *new_COUNTvalue(){
    COUNTvalue_t *self;

    self = malloc(sizeof(COUNTvalue_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP COUNTvalue_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(COUNTvalue_t));

    self->freeIE=free_COUNTvalue;
    self->showIE=show_COUNTvalue;

    return self;
}


/* *********** Bearers_SubjectToStatusTransfer_Item ************ */
/** @brief Bearers_SubjectToStatusTransfer_Item  Destructor
 *
 * Deallocate the Bearers_SubjectToStatusTransfer_Item_t structure.
 * */
void free_Bearers_SubjectToStatusTransfer_Item(void * data){
    Bearers_SubjectToStatusTransfer_Item_t *self = (Bearers_SubjectToStatusTransfer_Item_t*)data;
    if(!self){
        return;
    }

    if(self->dL_COUNTvalue){
        self->dL_COUNTvalue->freeIE(self->dL_COUNTvalue);
    }

    if(self->uL_COUNTvalue){
        self->uL_COUNTvalue->freeIE(self->uL_COUNTvalue);
    }

    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_Bearers_SubjectToStatusTransfer_Item(void * data){
    Bearers_SubjectToStatusTransfer_Item_t *self = (Bearers_SubjectToStatusTransfer_Item_t*)data;

    printf("\t\t\teRAB_ID : %u\n", self->eRAB_ID.id);

    if(self->dL_COUNTvalue){
        self->dL_COUNTvalue->showIE(self->dL_COUNTvalue);
    }

    if(self->uL_COUNTvalue){
        self->uL_COUNTvalue->showIE(self->uL_COUNTvalue);
    }

    if( (self->opt&0x80) == 0x80){
        printf("\t\t\tReceiveStatusofULPDCPSDUs : NOT IMPLEMENTED\n");
    }

    if(self->iEext!=NULL && (self->opt&0x40) == 0x40){
        self->iEext->showExtensionContainer(self->iEext);
    }
}

/** @brief Constructor of Bearers_SubjectToStatusTransfer_Item
 *  @return Bearers_SubjectToStatusTransfer_Item_t allocated  and initialized structure
 * */
Bearers_SubjectToStatusTransfer_Item_t *new_Bearers_SubjectToStatusTransfer_Item(){
    Bearers_SubjectToStatusTransfer_Item_t *self;

    self = malloc(sizeof(Bearers_SubjectToStatusTransfer_Item_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP Bearers_SubjectToStatusTransfer_Item_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(Bearers_SubjectToStatusTransfer_Item_t));

    self->freeIE=free_Bearers_SubjectToStatusTransfer_Item;
    self->showIE=show_Bearers_SubjectToStatusTransfer_Item;

    return self;
}


/* ******************* Generic Template Seq of****************** */
/** @brief Bearers-SubjectToStatusTransferList Destructor
 *
 * Deallocate the Bearers_SubjectToStatusTransferList_t structure.
 * */
void free_Bearers_SubjectToStatusTransferList(void * data){
    uint16_t i;
    Bearers_SubjectToStatusTransferList_t *self = (Bearers_SubjectToStatusTransferList_t*)data;
    if(!self){
        return;
    }

    for(i=0; i<self->size;i++){
        if(self->item[i]->freeIE){
            self->item[i]->freeIE(self->item[i]);
        }
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_Bearers_SubjectToStatusTransferList(void * data){
    Bearers_SubjectToStatusTransferList_t *self = (Bearers_SubjectToStatusTransferList_t*)data;
    uint16_t i;

    for(i=0; i < self->size; i++){
        if(&(self->item[i]) == NULL){
            printf("\t\t\t(*ie_item)_t Item #%u not found\n", i);
            continue;
        }
        if(self->item[i]->showIE){
            self->item[i]->showIE(self->item[i]);
        }else{
            printf("\t\t\t(*ie_item)_t Item #%u: show function not found\n", i);
        }
    }

}

void Bearers_SubjectToStatusTransferList_addItem(Bearers_SubjectToStatusTransferList_t* c, ProtocolIE_SingleContainer_t* item){
    ProtocolIE_SingleContainer_t** vector;
    if(c->size+1==maxnoofGroupIDs){
        s1ap_msg(ERROR, 0, "maxnoofGroupIDs reached");
        return;
    }

    c->size++;
    vector = (ProtocolIE_SingleContainer_t**) realloc (c->item, c->size * sizeof(ProtocolIE_SingleContainer_t*));

    /*Error Check*/
    if (vector!=NULL) {
        c->item=vector;
        c->item[c->size-1]=item;
    }
    else {
      free (c->item);
      s1ap_msg(ERROR, 0, "Error (re)allocating memory");
    }
}

/** @brief Constructor of Bearers_SubjectToStatusTransferList type
 *  @return Bearers_SubjectToStatusTransferList_t allocated  and initialized structure
 * */
Bearers_SubjectToStatusTransferList_t *new_Bearers_SubjectToStatusTransferList(){
    Bearers_SubjectToStatusTransferList_t *self;

    self = malloc(sizeof(Bearers_SubjectToStatusTransferList_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP Bearers_SubjectToStatusTransferList_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(Bearers_SubjectToStatusTransferList_t));

    self->freeIE=free_Bearers_SubjectToStatusTransferList;
    self->showIE=show_Bearers_SubjectToStatusTransferList;
    self->additem=Bearers_SubjectToStatusTransferList_addItem;

    return self;
}


/* ********** ENB_StatusTransfer_TransparentContainer ********** */
/** @brief ENB_StatusTransfer_TransparentContainer  Destructor
 *
 * Deallocate the ENB_StatusTransfer_TransparentContainer_t structure.
 * */
void free_ENB_StatusTransfer_TransparentContainer(void * data){
    ENB_StatusTransfer_TransparentContainer_t *self = (ENB_StatusTransfer_TransparentContainer_t*)data;
    if(!self){
        return;
    }

    if(self->bearers_SubjectToStatusTransferList){
        self->bearers_SubjectToStatusTransferList->freeIE(self->bearers_SubjectToStatusTransferList);
    }

    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }
    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_ENB_StatusTransfer_TransparentContainer(void * data){
    ENB_StatusTransfer_TransparentContainer_t *self = (ENB_StatusTransfer_TransparentContainer_t*)data;

    if(self->bearers_SubjectToStatusTransferList){
        self->bearers_SubjectToStatusTransferList->showIE(self->bearers_SubjectToStatusTransferList);
    }

    if(self->iEext){
        self->iEext->showExtensionContainer(self->iEext);
    }
}

/** @brief Constructor of ENB_StatusTransfer_TransparentContainer
 *  @return ENB_StatusTransfer_TransparentContainer_t allocated  and initialized structure
 * */
ENB_StatusTransfer_TransparentContainer_t *new_ENB_StatusTransfer_TransparentContainer(){
    ENB_StatusTransfer_TransparentContainer_t *self;

    self = malloc(sizeof(ENB_StatusTransfer_TransparentContainer_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP ENB_StatusTransfer_TransparentContainer_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(ENB_StatusTransfer_TransparentContainer_t));

    self->freeIE=free_ENB_StatusTransfer_TransparentContainer;
    self->showIE=show_ENB_StatusTransfer_TransparentContainer;

    return self;
}


/* ************************* E-RABItem ************************* */
/** @brief E_RABItem  Destructor
 *
 * Deallocate the E_RABItem_t structure.
 * */
void free_E_RABItem(void * data){
    E_RABItem_t *self = (E_RABItem_t*)data;
    if(!self){
        return;
    }

    if(self->cause){
        self->cause->freeIE(self->cause);
    }

    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }
    free(self);
}
/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_E_RABItem(void * data){
    E_RABItem_t *self = (E_RABItem_t*)data;

    printf("\t\t\teRAB_ID : %u\n", self->eRAB_ID.id);

    if(self->cause){
        self->cause->showIE(self->cause);
    }

    if(self->iEext){
        self->iEext->showExtensionContainer(self->iEext);
    }
}

/** @brief Constructor of E_RABItem
 *  @return E_RABItem_t allocated  and initialized structure
 * */
E_RABItem_t *new_E_RABItem(){
    E_RABItem_t *self;

    self = malloc(sizeof(E_RABItem_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP E_RABItem_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(E_RABItem_t));

    self->freeIE=free_E_RABItem;
    self->showIE=show_E_RABItem;

    return self;
}


/* ************************** E_RABList ************************ */
/** @brief E_RABList Destructor
 *
 * Deallocate the E_RABList_t structure.
 * */
void free_E_RABList(void * data){
    uint16_t i;
    E_RABList_t *self = (E_RABList_t*)data;
    if(!self){
        return;
    }

    for(i=0; i<self->size;i++){
        if(self->item[i]->freeIE){
            self->item[i]->freeIE(self->item[i]);
        }
    }
    free(self->item);
    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_E_RABList(void * data){
    E_RABList_t *self = (E_RABList_t*)data;
    uint16_t i;

    for(i=0; i < self->size; i++){
        if(&(self->item[i]) == NULL){
            printf("\t\t\t(*ie_item)_t Item #%u not found\n", i);
            continue;
        }
        if(self->item[i]->showIE){
            self->item[i]->showIE(self->item[i]);
        }else{
            printf("\t\t\t(*ie_item)_t Item #%u: show function not found\n", i);
        }
    }

}

void E_RABList_addItem(E_RABList_t* c, ProtocolIE_SingleContainer_t* item){
    ProtocolIE_SingleContainer_t** vector;
    if(c->size+1==maxnoofGroupIDs){
        s1ap_msg(ERROR, 0, "maxnoofGroupIDs reached");
        return;
    }

    c->size++;
    vector = (ProtocolIE_SingleContainer_t**) realloc (c->item, c->size * sizeof(ProtocolIE_SingleContainer_t*));

    /*Error Check*/
    if (vector!=NULL) {
        c->item=vector;
        c->item[c->size-1]=item;
    }
    else {
      free (c->item);
      s1ap_msg(ERROR, 0, "Error (re)allocating memory");
    }
}

/** @brief Constructor of E_RABList type
 *  @return E_RABList_t allocated  and initialized structure
 * */
E_RABList_t *new_E_RABList(){
    E_RABList_t *self;

    self = malloc(sizeof(E_RABList_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP E_RABList_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(E_RABList_t));

    self->freeIE=free_E_RABList;
    self->showIE=show_E_RABList;
    self->additem=E_RABList_addItem;

    return self;
}


/* ************* E-RABToBeModifiedItemBearerModReq ************* */
/** @brief E_RABToBeModifiedItemBearerModReq  Destructor
 *
 * Deallocate the E_RABToBeModifiedItemBearerModReq_t structure.
 * */
void free_E_RABToBeModifiedItemBearerModReq(void * data){
    E_RABToBeModifiedItemBearerModReq_t *self = (E_RABToBeModifiedItemBearerModReq_t*)data;
    if(!self){
        return;
    }

    if(self->eRABLevelQoSParameters){
        self->eRABLevelQoSParameters->freeIE(self->eRABLevelQoSParameters);
    }

    if(self->nAS_PDU){
        self->nAS_PDU->freeIE(self->nAS_PDU);
    }

    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }
    free(self);
}
/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_E_RABToBeModifiedItemBearerModReq(void * data){
    E_RABToBeModifiedItemBearerModReq_t *self = (E_RABToBeModifiedItemBearerModReq_t*)data;

    printf("\t\t\teRAB_ID : %u\n", self->eRAB_ID.id);

    if(self->eRABLevelQoSParameters){
        self->eRABLevelQoSParameters->showIE(self->eRABLevelQoSParameters);
    }

    if(self->nAS_PDU){
        self->nAS_PDU->showIE(self->nAS_PDU);
    }

    if(self->iEext){
        self->iEext->showExtensionContainer(self->iEext);
    }
}

/** @brief Constructor of E_RABToBeModifiedItemBearerModReq
 *  @return E_RABToBeModifiedItemBearerModReq_t allocated  and initialized structure
 * */
E_RABToBeModifiedItemBearerModReq_t *new_E_RABToBeModifiedItemBearerModReq(){
    E_RABToBeModifiedItemBearerModReq_t *self;

    self = malloc(sizeof(E_RABToBeModifiedItemBearerModReq_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP E_RABToBeModifiedItemBearerModReq_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(E_RABToBeModifiedItemBearerModReq_t));

    self->freeIE=free_E_RABToBeModifiedItemBearerModReq;
    self->showIE=show_E_RABToBeModifiedItemBearerModReq;

    return self;
}


/* ************* E-RABToBeModifiedListBearerModReq ************* */
/** @brief E_RABToBeModifiedListBearerModReq Destructor
 *
 * Deallocate the E_RABToBeModifiedListBearerModReq_t structure.
 * */
void free_E_RABToBeModifiedListBearerModReq(void * data){
    uint16_t i;
    E_RABToBeModifiedListBearerModReq_t *self = (E_RABToBeModifiedListBearerModReq_t*)data;
    if(!self){
        return;
    }

    for(i=0; i<self->size;i++){
        if(self->item[i]->freeIE){
            self->item[i]->freeIE(self->item[i]);
        }
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_E_RABToBeModifiedListBearerModReq(void * data){
    E_RABToBeModifiedListBearerModReq_t *self = (E_RABToBeModifiedListBearerModReq_t*)data;
    uint16_t i;

    for(i=0; i < self->size; i++){
        if(&(self->item[i]) == NULL){
            printf("\t\t\t(*ie_item)_t Item #%u not found\n", i);
            continue;
        }
        if(self->item[i]->showIE){
            self->item[i]->showIE(self->item[i]);
        }else{
            printf("\t\t\t(*ie_item)_t Item #%u: show function not found\n", i);
        }
    }

}

void E_RABToBeModifiedListBearerModReq_addItem(E_RABToBeModifiedListBearerModReq_t* c, ProtocolIE_SingleContainer_t* item){
    ProtocolIE_SingleContainer_t** vector;
    if(c->size+1==maxnoofGroupIDs){
        s1ap_msg(ERROR, 0, "maxnoofGroupIDs reached");
        return;
    }

    c->size++;
    vector = (ProtocolIE_SingleContainer_t**) realloc (c->item, c->size * sizeof(ProtocolIE_SingleContainer_t*));

    /*Error Check*/
    if (vector!=NULL) {
        c->item=vector;
        c->item[c->size-1]=item;
    }
    else {
      free (c->item);
      s1ap_msg(ERROR, 0, "Error (re)allocating memory");
    }
}

/** @brief Constructor of E_RABToBeModifiedListBearerModReq type
 *  @return E_RABToBeModifiedListBearerModReq_t allocated  and initialized structure
 * */
E_RABToBeModifiedListBearerModReq_t *new_E_RABToBeModifiedListBearerModReq(){
    E_RABToBeModifiedListBearerModReq_t *self;

    self = malloc(sizeof(E_RABToBeModifiedListBearerModReq_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP E_RABToBeModifiedListBearerModReq_t_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(E_RABToBeModifiedListBearerModReq_t));

    self->freeIE=free_E_RABToBeModifiedListBearerModReq;
    self->showIE=show_E_RABToBeModifiedListBearerModReq;
    self->additem=E_RABToBeModifiedListBearerModReq_addItem;

    return self;
}


/* ************* E-RABToBeSetupItemBearerSUReq ************* */
/** @brief E_RABToBeSetupItemBearerSUReq  Destructor
 *
 * Deallocate the E_RABToBeSetupItemBearerSUReq_t structure.
 * */
void free_E_RABToBeSetupItemBearerSUReq(void * data){
    E_RABToBeSetupItemBearerSUReq_t *self = (E_RABToBeSetupItemBearerSUReq_t*)data;
    if(!self){
        return;
    }

    if(self->eRABLevelQoSParameters){
        self->eRABLevelQoSParameters->freeIE(self->eRABLevelQoSParameters);
    }

    if(self->transportLayerAddress){
        self->transportLayerAddress->freeIE(self->transportLayerAddress);
    }

    if(self->nAS_PDU){
        self->nAS_PDU->freeIE(self->nAS_PDU);
    }

    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }
    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_E_RABToBeSetupItemBearerSUReq(void * data){
    E_RABToBeSetupItemBearerSUReq_t *self = (E_RABToBeSetupItemBearerSUReq_t*)data;

    printf("\t\t\teRAB_ID : %u\n", self->eRAB_ID.id);

    if(self->eRABLevelQoSParameters){
        self->eRABLevelQoSParameters->showIE(self->eRABLevelQoSParameters);
    }

    if(self->nAS_PDU){
        self->nAS_PDU->showIE(self->nAS_PDU);
    }

    if(self->transportLayerAddress){
        self->transportLayerAddress->showIE(self->transportLayerAddress);
    }

    if(self->iEext){
        self->iEext->showExtensionContainer(self->iEext);
    }
}

/** @brief Constructor of E_RABToBeSetupItemBearerSUReq
 *  @return E_RABToBeSetupItemBearerSUReq_t allocated  and initialized structure
 * */
E_RABToBeSetupItemBearerSUReq_t *new_E_RABToBeSetupItemBearerSUReq(){
    E_RABToBeSetupItemBearerSUReq_t *self;

    self = malloc(sizeof(E_RABToBeSetupItemBearerSUReq_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP E_RABToBeSetupItemBearerSUReq_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(E_RABToBeSetupItemBearerSUReq_t));

    self->freeIE=free_E_RABToBeSetupItemBearerSUReq;
    self->showIE=show_E_RABToBeSetupItemBearerSUReq;

    return self;
}


/* ************* E_RABToBeSetupListBearerSUReq ************* */
/** @brief E_RABToBeSetupListBearerSUReq Destructor
 *
 * Deallocate the E_RABToBeSetupListBearerSUReq_t structure.
 * */
void free_E_RABToBeSetupListBearerSUReq(void * data){
    uint16_t i;
    E_RABToBeSetupListBearerSUReq_t *self = (E_RABToBeSetupListBearerSUReq_t*)data;
    if(!self){
        return;
    }

    for(i=0; i<self->size;i++){
        if(self->item[i]->freeIE){
            self->item[i]->freeIE(self->item[i]);
        }
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_E_RABToBeSetupListBearerSUReq(void * data){
    E_RABToBeSetupListBearerSUReq_t *self = (E_RABToBeSetupListBearerSUReq_t*)data;
    uint16_t i;

    for(i=0; i < self->size; i++){
        if(&(self->item[i]) == NULL){
            printf("\t\t\t(*ie_item)_t Item #%u not found\n", i);
            continue;
        }
        if(self->item[i]->showIE){
            self->item[i]->showIE(self->item[i]);
        }else{
            printf("\t\t\t(*ie_item)_t Item #%u: show function not found\n", i);
        }
    }

}

void E_RABToBeSetupListBearerSUReq_addItem(E_RABToBeSetupListBearerSUReq_t* c, ProtocolIE_SingleContainer_t* item){
    ProtocolIE_SingleContainer_t** vector;
    if(c->size+1==maxnoofGroupIDs){
        s1ap_msg(ERROR, 0, "maxnoofGroupIDs reached");
        return;
    }

    c->size++;
    vector = (ProtocolIE_SingleContainer_t**) realloc (c->item, c->size * sizeof(ProtocolIE_SingleContainer_t*));

    /*Error Check*/
    if (vector!=NULL) {
        c->item=vector;
        c->item[c->size-1]=item;
    }
    else {
      free (c->item);
      s1ap_msg(ERROR, 0, "Error (re)allocating memory");
    }
}

/** @brief Constructor of E_RABToBeSetupListBearerSUReq type
 *  @return E_RABToBeSetupListBearerSUReq_t allocated  and initialized structure
 * */
E_RABToBeSetupListBearerSUReq_t *new_E_RABToBeSetupListBearerSUReq(){
    E_RABToBeSetupListBearerSUReq_t *self;

    self = malloc(sizeof(E_RABToBeSetupListBearerSUReq_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP E_RABToBeSetupListBearerSUReq_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(E_RABToBeSetupListBearerSUReq_t));

    self->freeIE=free_E_RABToBeSetupListBearerSUReq;
    self->showIE=show_E_RABToBeSetupListBearerSUReq;
    self->additem=E_RABToBeSetupListBearerSUReq_addItem;

    return self;
}


/* ************* E-RABSetupItemBearerSUReq ************* */
/** @brief E_RABSetupItemBearerSUReq  Destructor
 *
 * Deallocate the E_RABSetupItemBearerSUReq_t structure.
 * */
void free_E_RABSetupItemBearerSURes(void * data){
    E_RABSetupItemBearerSURes_t *self = (E_RABSetupItemBearerSURes_t*)data;
    if(!self){
        return;
    }

    if(self->transportLayerAddress){
        self->transportLayerAddress->freeIE(self->transportLayerAddress);
    }

    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }
    free(self);
}
/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_E_RABSetupItemBearerSURes(void * data){
    E_RABSetupItemBearerSURes_t *self = (E_RABSetupItemBearerSURes_t*)data;

    printf("\t\t\teRAB_ID : %u\n", self->eRAB_ID.id);

    if(self->transportLayerAddress){
        self->transportLayerAddress->showIE(self->transportLayerAddress);
    }

    printf("\t\t\tGTP_TEID : %#x %#x %#x %#x\n",
                self->gTP_TEID.teid[0], self->gTP_TEID.teid[1], self->gTP_TEID.teid[2], self->gTP_TEID.teid[3]);


    if(self->iEext){
        self->iEext->showExtensionContainer(self->iEext);
    }
}

/** @brief Constructor of E_RABSetupItemBearerSUReq
 *  @return E_RABSetupItemBearerSUReq_t allocated  and initialized structure
 * */
E_RABSetupItemBearerSURes_t *new_E_RABSetupItemBearerSURes(){
    E_RABSetupItemBearerSURes_t *self;

    self = malloc(sizeof(E_RABSetupItemBearerSURes_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP E_RABSetupItemBearerSUReq_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(E_RABSetupItemBearerSURes_t));

    self->freeIE=free_E_RABSetupItemBearerSURes;
    self->showIE=show_E_RABSetupItemBearerSURes;

    return self;
}


/* ************* E_RABSetupListBearerSUReq ************* */
/** @brief E_RABSetupListBearerSUReq Destructor
 *
 * Deallocate the E_RABSetupListBearerSUReq_t structure.
 * */
void free_E_RABSetupListBearerSURes(void * data){
    uint16_t i;
    E_RABSetupListBearerSURes_t *self = (E_RABSetupListBearerSURes_t*)data;
    if(!self){
        return;
    }

    for(i=0; i<self->size;i++){
        if(self->item[i]->freeIE){
            self->item[i]->freeIE(self->item[i]);
        }
    }
    free(self->item);
    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_E_RABSetupListBearerSURes(void * data){
    E_RABSetupListBearerSURes_t *self = (E_RABSetupListBearerSURes_t*)data;
    uint16_t i;

    for(i=0; i < self->size; i++){
        if(&(self->item[i]) == NULL){
            printf("\t\t\t(*ie_item)_t Item #%u not found\n", i);
            continue;
        }
        if(self->item[i]->showIE){
            self->item[i]->showIE(self->item[i]);
        }else{
            printf("\t\t\t(*ie_item)_t Item #%u: show function not found\n", i);
        }
    }

}

void E_RABSetupListBearerSURes_addItem(E_RABSetupListBearerSURes_t* c, ProtocolIE_SingleContainer_t* item){
    ProtocolIE_SingleContainer_t** vector;
    if(c->size+1==maxnoofGroupIDs){
        s1ap_msg(ERROR, 0, "maxnoofGroupIDs reached");
        return;
    }

    c->size++;
    vector = (ProtocolIE_SingleContainer_t**) realloc (c->item, c->size * sizeof(ProtocolIE_SingleContainer_t*));

    /*Error Check*/
    if (vector!=NULL) {
        c->item=vector;
        c->item[c->size-1]=item;
    }
    else {
        free (c->item);
        s1ap_msg(ERROR, 0, "Error (re)allocating memory");
    }
}

void *E_RABSetupListBearerSURes_newItem(struct E_RABSetupListBearerSURes_c* eRABlist){
    S1AP_PROTOCOL_IES_t* ie = newProtocolIE();
    E_RABSetupItemBearerSURes_t *eRABitem = new_E_RABSetupItemBearerSURes();
    ie->value = eRABitem;
    ie->showValue = eRABitem->showIE;
    ie->freeValue = eRABitem->freeIE;
    ie->id = id_E_RABSetupItemBearerSURes;
    ie->presence = optional;
    ie->criticality = reject;
    eRABlist->additem(eRABlist, ie);
    return eRABitem;
}


/** @brief Constructor of E_RABSetupListBearerSUReq type
 *  @return E_RABSetupListBearerSUReq_t allocated  and initialized structure
 * */
E_RABSetupListBearerSURes_t *new_E_RABSetupListBearerSURes(){
    E_RABSetupListBearerSURes_t *self;

    self = malloc(sizeof(E_RABSetupListBearerSURes_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP E_RABSetupListBearerSUReq_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(E_RABSetupListBearerSURes_t));

    self->freeIE=free_E_RABSetupListBearerSURes;
    self->showIE=show_E_RABSetupListBearerSURes;
    self->additem=E_RABSetupListBearerSURes_addItem;
    self->newItem=E_RABSetupListBearerSURes_newItem;

    return self;
}


/* ************************ HandoverType *********************** */
/** @brief HandoverType  Destructor
 *
 * Deallocate the HandoverType_t structure.
 * */
void free_HandoverType(void * data){
    HandoverType_t *self = (HandoverType_t*)data;
    if(!self){
        return;
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_HandoverType(void * data){
    HandoverType_t *self = (HandoverType_t*)data;
    if (self->ext==0){
        printf("\t\t\tHandover Type : %s(%u)\n", HandoverTypeName[self->ht.noext], self->ht.noext);
    }else{
        s1ap_msg(WARN, 0, "Handover Type with extension detected. Current version doesn't support it.");
    }
}

/** @brief Constructor of HandoverType
 *  @return HandoverType_t allocated  and initialized structure
 * */
HandoverType_t *new_HandoverType(){
    HandoverType_t *self;

    self = malloc(sizeof(HandoverType_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP HandoverType_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(HandoverType_t));

    self->freeIE=free_HandoverType;
    self->showIE=show_HandoverType;

    return self;
}


/* ********************** E_TargeteNB_ID ********************* */
/** @brief E_TargeteNB  Destructor
 *
 * Deallocate the E_TargeteNB_t structure.
 * */
void free_TargeteNB_ID(void * data){
    TargeteNB_ID_t *self = (TargeteNB_ID_t*)data;
    if(!self){
        return;
    }

    if(self->global_ENB_ID){
        self->global_ENB_ID->freeIE(self->global_ENB_ID);
    }

    if(self->global_ENB_ID){
        self->selected_TAI->freeIE(self->selected_TAI);
    }

    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_TargeteNB_ID(void * data){
    TargeteNB_ID_t *self = (TargeteNB_ID_t*)data;

    if(self->global_ENB_ID){
        self->global_ENB_ID->showIE(self->global_ENB_ID);
    }

    if(self->selected_TAI){
        self->selected_TAI->showIE(self->selected_TAI);
    }

    if(self->iEext){
        self->iEext->showExtensionContainer(self->iEext);
    }
}

/** @brief Constructor of E_TargeteNB
 *  @return E_TargeteNB_t allocated  and initialized structure
 * */
TargeteNB_ID_t *new_TargeteNB_ID(){
    TargeteNB_ID_t *self;

    self = malloc(sizeof(TargeteNB_ID_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP E_TargeteNB_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(TargeteNB_ID_t));

    self->freeIE=free_TargeteNB_ID;
    self->showIE=show_TargeteNB_ID;

    return self;
}

/* ********************** E_SourceeNB_ID ********************* */

SourceeNB_ID_t *new_SourceeNB_ID(){
    return (SourceeNB_ID_t *)new_TargeteNB_ID();
}

/* ***************************** LAI *************************** */
/** @brief LAI  Destructor
 *
 * Deallocate the LAI_t structure.
 * */
void free_LAI(void * data){
    LAI_t *self = (LAI_t*)data;
    if(!self){
        return;
    }

    if(self->pLMNidentity){
        self->pLMNidentity->freeIE(self->pLMNidentity);
    }

    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_LAI(void * data){
    LAI_t *self = (LAI_t*)data;
    if(self->pLMNidentity){
        self->pLMNidentity->showIE(self->pLMNidentity);
    }

    printf("\t\t\tLAC %#x %#x\n", self->lAC.s[0], self->lAC.s[1]);

    if(self->iEext){
        self->iEext->showExtensionContainer(self->iEext);
    }
}

/** @brief Constructor of LAI
 *  @return LAI_t allocated  and initialized structure
 * */
LAI_t *new_LAI(){
    LAI_t *self;

    self = malloc(sizeof(LAI_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP LAI_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(LAI_t));

    self->freeIE=free_LAI;
    self->showIE=show_LAI;

    return self;
}


/* ************************ TargetRNC-ID *********************** */
/** @brief TargetRNC_ID  Destructor
 *
 * Deallocate the TargetRNC_ID_t structure.
 * */
void free_TargetRNC_ID(void * data){
    TargetRNC_ID_t *self = (TargetRNC_ID_t*)data;
    if(!self){
        return;
    }

    if(self->lAI){
        self->lAI->freeIE(self->lAI);
    }

    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_TargetRNC_ID(void * data){
    TargetRNC_ID_t *self = (TargetRNC_ID_t*)data;

    if(self->lAI){
        self->lAI->showIE(self->lAI);
    }

    printf("\t\t\tRNC-ID %#x\n", self->rNC_ID.pDCP_SN);

    if(self->iEext){
        self->iEext->showExtensionContainer(self->iEext);
    }
}

/** @brief Constructor of TargetRNC_ID
 *  @return TargetRNC_ID_t allocated  and initialized structure
 * */
TargetRNC_ID_t *new_TargetRNC_ID(){
    TargetRNC_ID_t *self;

    self = malloc(sizeof(TargetRNC_ID_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP TargetRNC_ID_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(TargetRNC_ID_t));

    self->freeIE=free_TargetRNC_ID;
    self->showIE=show_TargetRNC_ID;

    return self;
}


/* ***************************** CGI *************************** */
/** @brief (*ie_name)  Destructor
 *
 * Deallocate the CGI_t structure.
 * */
void free_CGI(void * data){
    CGI_t *self = (CGI_t*)data;
    if(!self){
        return;
    }

    if(self->pLMNidentity){
        self->pLMNidentity->freeIE(self->pLMNidentity);
    }

    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_CGI(void * data){
    CGI_t *self = (CGI_t*)data;

    if(self->pLMNidentity){
        self->pLMNidentity->showIE(self->pLMNidentity);
    }
    printf("\t\t\tLAC %#x %#x\n", self->lAC.s[0], self->lAC.s[1]);

    printf("\t\t\tCI %#x %#x\n", self->cI.s[0], self->cI.s[1]);

    if(self->iEext){
        self->iEext->showExtensionContainer(self->iEext);
    }
}

/** @brief Constructor of CGI
 *  @return CGI_t allocated  and initialized structure
 * */
CGI_t *new_CGI(){
    CGI_t *self;

    self = malloc(sizeof(CGI_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP CGI_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(CGI_t));

    self->freeIE=free_CGI;
    self->showIE=show_CGI;

    return self;
}


/* ************************* TargetID ************************* */
/** @brief TargetID  Destructor
 *
 * Deallocate the TargetID_t structure.
 * */
void free_TargetID(void * data){
    TargetID_t *self = (TargetID_t*)data;
    if(!self){
        return;
    }

    if(self->choice==0){
        self->targetID.targeteNB_ID->freeIE(self->targetID.targeteNB_ID);
    }else if(self->choice==1){
        self->targetID.targetRNC_ID->freeIE(self->targetID.targetRNC_ID);
    }else if(self->choice==2){
        self->targetID.cGI->freeIE(self->targetID.cGI);
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_TargetID(void * data){
    TargetID_t *self = (TargetID_t*)data;

    if(self->choice==0){
        self->targetID.targeteNB_ID->showIE(self->targetID.targeteNB_ID);
    }else if(self->choice==1){
        self->targetID.targetRNC_ID->showIE(self->targetID.targetRNC_ID);
    }else if(self->choice==2){
        self->targetID.cGI->showIE(self->targetID.cGI);
    }
}

/** @brief Constructor of TargetID
 *  @return TargetID_t allocated  and initialized structure
 * */
TargetID_t *new_TargetID(){
    TargetID_t *self;

    self = malloc(sizeof(TargetID_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP TargetID_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(TargetID_t));

    self->freeIE=free_TargetID;
    self->showIE=show_TargetID;

    return self;
}


/* ****************** Direct_Forwarding_Path_Availability **************** */
/** @brief Direct_Forwarding_Path_Availability IE Destructor
 *
 * Deallocate the Direct_Forwarding_Path_Availability_t structure.
 * */
void free_Direct_Forwarding_Path_Availability(void * data){
    Direct_Forwarding_Path_Availability_t *self = (Direct_Forwarding_Path_Availability_t*)data;
    if(!self){
        return;
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_Direct_Forwarding_Path_Availability(void * data){
    Direct_Forwarding_Path_Availability_t *self = (Direct_Forwarding_Path_Availability_t*)data;
    if(self->ext==0){
        printf("\t\t\tdirectPathAvailable\n");
    }else{
        printf("\t\t\tDirect_Forwarding_Path_Availability: extension found. value=%u, not available in current version\n", self->ind);
    }
}

/** @brief Constructor of Direct_Forwarding_Path_Availability type
 *  @return Direct_Forwarding_Path_Availability_t allocated  and initialized structure
 * */
Direct_Forwarding_Path_Availability_t *new_Direct_Forwarding_Path_Availability(){
    Direct_Forwarding_Path_Availability_t *self;

    self = malloc(sizeof(Direct_Forwarding_Path_Availability_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP Direct_Forwarding_Path_Availability_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(Direct_Forwarding_Path_Availability_t));

    self->freeIE=free_Direct_Forwarding_Path_Availability;
    self->showIE=show_Direct_Forwarding_Path_Availability;

    return self;
}



/* **************** Source-ToTarget-TransparentContainer ***************** */
/** @brief Source_ToTarget_TransparentContainer IE Destructor
 *
 * Deallocate the Source_ToTarget_TransparentContainer_t structure.
 * */
void free_Unconstrained_Octed_String(void * data){
    Unconstrained_Octed_String_t *self = (Unconstrained_Octed_String_t*)data;
    if(!self){
        return;
    }
    /*
    if(self->len != 0 && self->str!= NULL){
        free(self->str);
        self->str=NULL;
    }*/

    free(self);
}
/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_Unconstrained_Octed_String(void * data){
    Unconstrained_Octed_String_t *self = (Unconstrained_Octed_String_t*)data;
    printf("\t\t\tData: %#x %#x .. %#x %#x, len= %u\n", self->str[0], self->str[1], self->str[self->len-2], self->str[self->len-1], self->len);
}

/** @brief Constructor of Source_ToTarget_TransparentContainer type
 *  @return Source_ToTarget_TransparentContainer_t allocated  and initialized structure
 * */
Unconstrained_Octed_String_t *new_Unconstrained_Octed_String(){
    Unconstrained_Octed_String_t *self;

    self = malloc(sizeof(Unconstrained_Octed_String_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP Source_ToTarget_TransparentContainer_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(Unconstrained_Octed_String_t));

    self->freeIE=free_Unconstrained_Octed_String;
    self->showIE=show_Unconstrained_Octed_String;

    return self;
}



/* **************** E_RABToBeSetupItemHOReq **************** */
/** @brief E_RABToBeSetupItemHOReq IE Destructor
 *
 * Deallocate the E_RABToBeSetupItemHOReq_t structure.
 * */
void free_E_RABToBeSetupItemHOReq(void * data){
    E_RABToBeSetupItemHOReq_t *self = (E_RABToBeSetupItemHOReq_t*)data;
    if(!self){
        return;
    }

    if(self->transportLayerAddress){
        self->transportLayerAddress->freeIE(self->transportLayerAddress);
    }

    if(self->eRABLevelQoSParameters){
        self->eRABLevelQoSParameters->freeIE(self->eRABLevelQoSParameters);
    }

    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_E_RABToBeSetupItemHOReq(void * data){
    E_RABToBeSetupItemHOReq_t *self = (E_RABToBeSetupItemHOReq_t*)data;

    printf("\t\t\teRAB_ID : %u\n", self->eRAB_ID.id);

    if(self->transportLayerAddress!=NULL){
        printf("\t\t\t*TransportLayerAddress: \n");
        self->transportLayerAddress->showIE(self->transportLayerAddress);
    }

    printf("\t\t\tGTP_TEID : %#x %#x %#x %#x\n",
                self->gTP_TEID.teid[0], self->gTP_TEID.teid[1], self->gTP_TEID.teid[2], self->gTP_TEID.teid[3]);

    if(self->eRABLevelQoSParameters!=NULL){
        printf("\t\t\t*E-RABlevelQoSParameters: \n");
        self->eRABLevelQoSParameters->showIE(self->eRABLevelQoSParameters);
    }

    if((self->opt&0x40)== 0x40 && self->iEext!=NULL){
        self->iEext->showExtensionContainer(self->iEext);
    }
}

/** @brief Constructor of E_RABToBeSetupItemHOReq type
 *  @return E_RABToBeSetupItemHOReq_t allocated  and initialized structure
 * */
E_RABToBeSetupItemHOReq_t *new_E_RABToBeSetupItemHOReq(){
    E_RABToBeSetupItemHOReq_t *self;

    self = malloc(sizeof(E_RABToBeSetupItemHOReq_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP E_RABToBeSetupItemHOReq_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(E_RABToBeSetupItemHOReq_t));

    self->freeIE=free_E_RABToBeSetupItemHOReq;
    self->showIE=show_E_RABToBeSetupItemHOReq;

    return self;
}

/* ********************** Generic Template ********************* */
/** @brief SecurityContext  Destructor
 *
 * Deallocate the SecurityContext_t structure.
 * */
void free_SecurityContext(void * data){
    SecurityContext_t *self = (SecurityContext_t*)data;
    if(!self){
        return;
    }

    if(self->nextHopParameter){
        self->nextHopParameter->freeIE(self->nextHopParameter);
    }

    if(self->iEext){
        self->iEext->freeExtensionContainer(self->iEext);
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_SecurityContext(void * data){
    SecurityContext_t *self = (SecurityContext_t*)data;

    printf("\t\t\tNextHopChainingCount : %u\n", self->nextHopChainingCount);

    if(self->nextHopParameter){
        self->nextHopParameter->showIE(self->nextHopParameter);
    }

    if(self->iEext){
        self->iEext->showExtensionContainer(self->iEext);
    }
}

/** @brief Constructor of SecurityContext
 *  @return SecurityContext_t allocated  and initialized structure
 * */
SecurityContext_t *new_SecurityContext(){
    SecurityContext_t *self;

    self = malloc(sizeof(SecurityContext_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP SecurityContext_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(SecurityContext_t));

    self->freeIE=free_SecurityContext;
    self->showIE=show_SecurityContext;

    return self;
}


/* ********************** E-RABAdmittedList ******************** */
E_RABAdmittedList_t *new_E_RABAdmittedList(){
    return new_E_RABToBeSetupListCtxtSUReq();
}


/* ********************** E-RABAdmittedItem ******************** */
/** @brief E_RABAdmittedItem  Destructor
 *
 * Deallocate the E_RABAdmittedItem_t structure.
 * */
void free_E_RABAdmittedItem(void * data){
    E_RABAdmittedItem_t *self = (E_RABAdmittedItem_t*)data;
    if(!self){
        return;
    }

    if(self->transportLayerAddress){
        if(self->transportLayerAddress->freeIE){
            self->transportLayerAddress->freeIE(self->transportLayerAddress);
        }
    }

    if(self->dL_transportLayerAddress){
        if(self->dL_transportLayerAddress->freeIE){
            self->dL_transportLayerAddress->freeIE(self->dL_transportLayerAddress);
        }
    }

    if(self->uL_transportLayerAddress){
        if(self->uL_transportLayerAddress->freeIE){
            self->uL_transportLayerAddress->freeIE(self->uL_transportLayerAddress);
        }
    }

    if(self->iEext){
        if(self->iEext->freeExtensionContainer){
            self->iEext->freeExtensionContainer(self->iEext);
        }
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_E_RABAdmittedItem(void * data){
    E_RABAdmittedItem_t *self = (E_RABAdmittedItem_t*)data;


    printf("\t\t\teRAB_ID : %u\n", self->eRAB_ID.id);

    if(self->transportLayerAddress!=NULL){
        printf("\t\t\t*TransportLayerAddress: \n");
        self->transportLayerAddress->showIE(self->transportLayerAddress);
    }

    printf("\t\t\tGTP_TEID : %#x %#x %#x %#x\n",
                self->gTP_TEID.teid[0], self->gTP_TEID.teid[1], self->gTP_TEID.teid[2], self->gTP_TEID.teid[3]);

    if(self->dL_transportLayerAddress!=NULL && (self->opt&0x80)== 0x80){
        printf("\t\t\t*dL_transportLayerAddress: \n");
        self->dL_transportLayerAddress->showIE(self->dL_transportLayerAddress);
    }

    if(&(self->dL_GTP_TEID)!=NULL && (self->opt&0x40)== 0x40){
        printf("\t\t\tdL_GTP_TEID : %#x %#x %#x %#x\n",
                    self->dL_GTP_TEID.teid[0], self->dL_GTP_TEID.teid[1], self->dL_GTP_TEID.teid[2], self->dL_GTP_TEID.teid[3]);
    }

    if(self->uL_transportLayerAddress!=NULL && (self->opt&0x20)== 0x20){
        printf("\t\t\t*uL_transportLayerAddress: \n");
        self->uL_transportLayerAddress->showIE(self->uL_transportLayerAddress);
    }

    if(&(self->dL_GTP_TEID) != NULL && (self->opt&0x10)== 0x10){
        printf("\t\t\tuL_GTP_TEID : %#x %#x %#x %#x\n",
                    self->uL_GTP_TEID.teid[0], self->uL_GTP_TEID.teid[1], self->uL_GTP_TEID.teid[2], self->uL_GTP_TEID.teid[3]);
    }

    if((self->opt&0x08)== 0x08 && self->iEext!=NULL){
        self->iEext->showExtensionContainer(self->iEext);
    }
}

/** @brief Constructor of E_RABAdmittedItem
 *  @return E_RABAdmittedItem_t allocated  and initialized structure
 * */
E_RABAdmittedItem_t *new_E_RABAdmittedItem(){
    E_RABAdmittedItem_t *self;

    self = malloc(sizeof(E_RABAdmittedItem_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP E_RABAdmittedItem_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(E_RABAdmittedItem_t));

    self->freeIE=free_E_RABAdmittedItem;
    self->showIE=show_E_RABAdmittedItem;

    return self;
}


/* ************** E-RABSubjecttoDataForwardingList ************* */
E_RABSubjecttoDataForwardingList_t *new_E_RABSubjecttoDataForwardingList(){
    return new_E_RABToBeSetupListCtxtSUReq();
}

/* ******************* E-RABDataForwardingItem ***************** */
/** @brief E_RABDataForwardingItem  Destructor
 *
 * Deallocate the E_RABDataForwardingItem_t structure.
 * */
void free_E_RABDataForwardingItem(void * data){
    E_RABDataForwardingItem_t *self = (E_RABDataForwardingItem_t*)data;
    if(!self){
        return;
    }

    if(self->dL_transportLayerAddress){
        if(self->dL_transportLayerAddress->freeIE){
            self->dL_transportLayerAddress->freeIE(self->dL_transportLayerAddress);
        }
    }

    if(self->uL_transportLayerAddress){
        if(self->uL_transportLayerAddress->freeIE){
            self->uL_transportLayerAddress->freeIE(self->uL_transportLayerAddress);
        }
    }

    if(self->iEext){
        if(self->iEext->freeExtensionContainer){
            self->iEext->freeExtensionContainer(self->iEext);
        }
    }

    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_E_RABDataForwardingItem(void * data){
    E_RABDataForwardingItem_t *self = (E_RABDataForwardingItem_t*)data;


    printf("\t\t\teRAB_ID : %u\n", self->eRAB_ID.id);

    if(self->dL_transportLayerAddress!=NULL && (self->opt&0x80)== 0x80){
        printf("\t\t\t*dL_transportLayerAddress: \n");
        self->dL_transportLayerAddress->showIE(self->dL_transportLayerAddress);
    }

    if(&(self->dL_GTP_TEID)!=NULL && (self->opt&0x40)== 0x40){
        printf("\t\t\tdL_GTP_TEID : %#x %#x %#x %#x\n",
                    self->dL_GTP_TEID.teid[0], self->dL_GTP_TEID.teid[1], self->dL_GTP_TEID.teid[2], self->dL_GTP_TEID.teid[3]);
    }

    if(self->uL_transportLayerAddress!=NULL && (self->opt&0x20)== 0x20){
        printf("\t\t\t*uL_transportLayerAddress: \n");
        self->uL_transportLayerAddress->showIE(self->uL_transportLayerAddress);
    }

    if(&(self->dL_GTP_TEID) != NULL && (self->opt&0x10)== 0x10){
        printf("\t\t\tuL_GTP_TEID : %#x %#x %#x %#x\n",
                    self->uL_GTP_TEID.teid[0], self->uL_GTP_TEID.teid[1], self->uL_GTP_TEID.teid[2], self->uL_GTP_TEID.teid[3]);
    }

    if((self->opt&0x08)== 0x08 && self->iEext!=NULL){
        self->iEext->showExtensionContainer(self->iEext);
    }
}

/** @brief Constructor of E_RABDataForwardingItem
 *  @return E_RABDataForwardingItem_t allocated  and initialized structure
 * */
E_RABDataForwardingItem_t *new_E_RABDataForwardingItem(){
    E_RABDataForwardingItem_t *self;

    self = malloc(sizeof(E_RABDataForwardingItem_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP E_RABDataForwardingItem_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(E_RABDataForwardingItem_t));

    self->freeIE=free_E_RABDataForwardingItem;
    self->showIE=show_E_RABDataForwardingItem;

    return self;
}


/* ************** E_RABToBeSetupListHOReq ****************** */
/** @brief E_RABToBeSetupListHOReq IE Destructor
 *
 * Deallocate the E_RABToBeSetupListHOReq_t structure.
 * */
void free_E_RABToBeSetupListHOReq(void * data){
    uint16_t i;
    E_RABToBeSetupListHOReq_t *self = (E_RABToBeSetupListHOReq_t*)data;
    if(!self){
        return;
    }

    for(i=0; i<self->size;i++){
        if(self->item[i]->freeIE){
            self->item[i]->freeIE(self->item[i]);
        }
    }
    free(self->item);
    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_E_RABToBeSetupListHOReq(void * data){
    E_RABToBeSetupListHOReq_t *self = (E_RABToBeSetupListHOReq_t*)data;
    S1AP_PROTOCOL_IES_t *item;
    uint16_t i;

    for(i=0; i < self->size; i++){
        if(&(self->item[i]) == NULL){
            printf("\t\t\tE_E_RABToBeSetupListHOReq_t Item #%u not found\n", i);
            continue;
        }

        item = (S1AP_PROTOCOL_IES_t*) self->item[i];
        if(item->showIE){
            item->showIE(item);
        }else{
            printf("\t\t\tE_E_RABToBeSetupListHOReq_t Item #%u: show function not found\n", i);
        }
    }

}

void E_RABToBeSetupListHOReq_addItem(E_RABToBeSetupListHOReq_t* c, ProtocolIE_SingleContainer_t* item){
    ProtocolIE_SingleContainer_t** vector;
    if(c->size+1==maxNrOfERABs){
        s1ap_msg(ERROR, 0, "maxNrOfE-RABs reached");
        return;
    }

    c->size++;
    vector = (ProtocolIE_SingleContainer_t**) realloc (c->item, c->size * sizeof(ProtocolIE_SingleContainer_t*));

    /*Error Check*/
    if (vector!=NULL) {
        c->item=vector;
        c->item[c->size-1]=item;
    }
    else {
       free (c->item);
       s1ap_msg(ERROR, 0, "Error (re)allocating memory");
    }
}

/** @brief Constructor of E_RABToBeSetupListHOReq type
 *  @return E_RABToBeSetupListHOReq_t allocated  and initialized structure
 * */
E_RABToBeSetupListHOReq_t *new_E_RABToBeSetupListHOReq(){
    E_RABToBeSetupListHOReq_t *self;

    self = malloc(sizeof(E_RABToBeSetupListHOReq_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP E_RABToBeSetupListHOReq_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(E_RABToBeSetupListHOReq_t));

    self->freeIE=free_E_RABToBeSetupListHOReq;
    self->showIE=show_E_RABToBeSetupListHOReq;
    self->additem=E_RABToBeSetupListHOReq_addItem;

    return self;
}


/* *********************** S_TMSI ************************** */
/** @brief S_TMSI  Destructor
 *
 * Deallocate the S_TMSI_t structure.
 * */
void free_S_TMSI(void * data){
    S_TMSI_t *self = (S_TMSI_t*)data;
    if(!self){
        return;
    }

    self->mMEC->freeIE(self->mMEC);

    free(self);
}
/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_S_TMSI(void * data){
    S_TMSI_t *self = (S_TMSI_t*)data;

    self->mMEC->showIE(self->mMEC);

    printf("\t\tM-TMSI : %#.2x%.2x%.2x%.2x\n",
                self->m_TMSI.s[0], self->m_TMSI.s[1], self->m_TMSI.s[2], self->m_TMSI.s[3]);
}
/** @brief Constructor of S_TMSI
 *  @return S_TMSI_t allocated  and initialized structure
 * */
S_TMSI_t *new_S_TMSI(){
    S_TMSI_t *self;

    self = malloc(sizeof(S_TMSI_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP S_TMSI_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(S_TMSI_t));

    self->freeIE=free_S_TMSI;
    self->showIE=show_S_TMSI;

    self->mMEC = new_MME_Code();

    return self;
}


/* ************** E_RABToBeSwitchedULList ************** */
/** @brief E_RABSetupListBearerSUReq Destructor
 *
 * Deallocate the E_RABToBeSwitchedULList_t structure.
 * */
void free_E_RABToBeSwitchedULList(void * data){
    uint16_t i;
    E_RABToBeSwitchedULList_t *self = (E_RABToBeSwitchedULList_t*)data;
    if(!self){
        return;
    }

    for(i=0; i<self->size;i++){
        if(self->item[i]->freeIE){
            self->item[i]->freeIE(self->item[i]);
        }
    }
    free(self->item);
    free(self);
}

/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * */
void show_E_RABToBeSwitchedULList(void * data){
    E_RABToBeSwitchedULList_t *self = (E_RABToBeSwitchedULList_t*)data;
    uint16_t i;

    for(i=0; i < self->size; i++){
        if(&(self->item[i]) == NULL){
            printf("\t\t\t(*ie_item)_t Item #%u not found\n", i);
            continue;
        }
        if(self->item[i]->showIE){
            self->item[i]->showIE(self->item[i]);
        }else{
            printf("\t\t\t(*ie_item)_t Item #%u: show function not found\n", i);
        }
    }

}

void E_RABToBeSwitchedULList_addItem(E_RABToBeSwitchedULList_t* c, ProtocolIE_SingleContainer_t* item){
    ProtocolIE_SingleContainer_t** vector;
    if(c->size+1==maxnoofGroupIDs){
        s1ap_msg(ERROR, 0, "maxnoofGroupIDs reached");
        return;
    }

    c->size++;
    vector = (ProtocolIE_SingleContainer_t**) realloc (c->item, c->size * sizeof(ProtocolIE_SingleContainer_t*));

    /*Error Check*/
    if (vector!=NULL) {
        c->item=vector;
        c->item[c->size-1]=item;
    }
    else {
        free (c->item);
        s1ap_msg(ERROR, 0, "Error (re)allocating memory");
    }
}

void *E_RABToBeSwitchedULList_newItem(E_RABToBeSwitchedULList_t* eRABlist){
    S1AP_PROTOCOL_IES_t* ie = newProtocolIE();
    E_RABToBeSwitchedULItem_t *eRABitem = new_E_RABToBeSwitchedULItem();
    ie->value = eRABitem;
    ie->showValue = eRABitem->showIE;
    ie->freeValue = eRABitem->freeIE;
    ie->id = id_E_RABToBeSwitchedULItem;
    ie->presence = optional;
    ie->criticality = reject;
    eRABlist->additem(eRABlist, ie);
    return eRABitem;
}


/** @brief Constructor of E_RABSetupListBearerSUReq type
 *  @return E_RABSetupListBearerSUReq_t allocated  and initialized structure
 * */
E_RABToBeSwitchedULList_t *new_E_RABToBeSwitchedULList(){
    E_RABToBeSwitchedULList_t *self;

    self = malloc(sizeof(E_RABToBeSwitchedULList_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP E_RABToBeSwitchedULList_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(E_RABToBeSwitchedULList_t));

    self->freeIE=free_E_RABToBeSwitchedULList;
    self->showIE=show_E_RABToBeSwitchedULList;
    self->additem=E_RABToBeSwitchedULList_addItem;
    self->newItem=E_RABToBeSwitchedULList_newItem;

    return self;
}

/* ********************** Generic Template ********************* */
/** @brief (*ie_name)  Destructor
 *
 * Deallocate the (*ie_name)_t structure.
 * *//*
void free_(*ie_name)(void * data){
    (*ie_name)_t *self = ((*ie_name)_t*)data;
    if(!self){
        return;
    }

    free(self);
}
*/
/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * *//*
void show_(*ie_name)(void * data){
    (*ie_name)_t *self = ((*ie_name)_t*)data;

}
*/
/** @brief Constructor of (*ie_name)
 *  @return (*ie_name)_t allocated  and initialized structure
 * *//*
(*ie_name)_t *new_(*ie_name)(){
    (*ie_name)_t *self;

    self = malloc(sizeof((*ie_name)_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP (*ie_name)_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof((*ie_name)_t));

    self->freeIE=free_(*ie_name);
    self->showIE=show_(*ie_name);

    return self;
}
*/

/* ******************* Generic Template Seq of****************** */
/** @brief (*ie_name) Destructor
 *
 * Deallocate the (*ie_name)_t structure.
 * *//*
void free_(*ie_name)(void * data){
    uint16_t i;
    (*ie_name)_t *self = ((*ie_name)_t*)data;
    if(!self){
        return;
    }

    for(i=0; i<self->size;i++){
        if(self->item[i]->freeIE){
            self->item[i]->freeIE(self->item[i]);
        }
    }

    free(self);
}
*/
/** @brief Show IE information
 *
 * Tool function to print the information on stdout
 * *//*
void show_(*ie_name)(void * data){
    (*ie_name)_t *self = ((*ie_name)_t*)data;
    uint16_t i;

    for(i=0; i < self->size; i++){
        if(&(self->item[i]) == NULL){
            printf("\t\t\t(*ie_item)_t Item #%u not found\n", i);
            continue;
        }
        if(self->item[i]->showIE){
            self->item[i]->showIE(self->item[i]);
        }else{
            printf("\t\t\t(*ie_item)_t Item #%u: show function not found\n", i);
        }
    }

}

void (*ie_name)_addItem((*ie_name)_t* c, (*ie_item)_t* item){
    (*ie_item)_t** vector;
    if(c->size+1==maxnoofGroupIDs){
        s1ap_msg(ERROR, 0, "maxnoofGroupIDs reached");
        return;
    }

    c->size++;
    vector = ((*ie_item)_t**) realloc (c->item, c->size * sizeof((*ie_item)_t*));

    *//*Error Check*//*
    if (vector!=NULL) {
        c->item=vector;
        c->item[c->size-1]=item;
    }
    else {
      free (c->item);
      s1ap_msg(ERROR, 0, "Error (re)allocating memory");
    }
}
*/
/** @brief Constructor of (*ie_name) type
 *  @return (*ie_name)_t allocated  and initialized structure
 * *//*
(*ie_name)_t *new_(*ie_name)(){
    (*ie_name)_t *self;

    self = malloc(sizeof((*ie_name)_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP (*ie_name)_t not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof((*ie_name)_t));

    self->freeIE=free_(*ie_name);
    self->showIE=show_(*ie_name);
    self->additem=(*ie_name)_addItem;

    return self;
}
*/
