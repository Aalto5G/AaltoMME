/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S1AP.h
 * @Author Vicent Ferrer
 * @date   April, 2013
 * @brief  S1AP Constant definitions
 *
 */

#ifndef _S1AP_CONSTANTS_H
#define _S1AP_CONSTANTS_H

/* **************************************************************
--
-- Elementary Procedures
--
-- *************************************************************/

typedef enum ProcedureCode_c{
    id_HandoverPreparation = 0,
    id_HandoverResourceAllocation,
    id_HandoverNotification,
    id_PathSwitchRequest,
    id_HandoverCancel,
    id_ERABSetup,
    id_ERABModify,
    id_ERABRelease,
    id_ERABReleaseIndication,
    id_InitialContextSetup,
    id_Paging,
    id_downlinkNASTransport,
    id_initialUEMessage,
    id_uplinkNASTransport,
    id_Reset,
    id_ErrorIndication,
    id_NASNonDeliveryIndication,
    id_S1Setup,
    id_UEContextReleaseRequest,
    id_DownlinkS1cdma2000tunneling,
    id_UplinkS1cdma2000tunneling,
    id_UEContextModification,
    id_UECapabilityInfoIndication,
    id_UEContextRelease,
    id_eNBStatusTransfer,
    id_MMEStatusTransfer,
    id_DeactivateTrace,
    id_TraceStart,
    id_TraceFailureIndication,
    id_ENBConfigurationUpdate,
    id_MMEConfigurationUpdate,
    id_LocationReportingControl,
    id_LocationReportingFailureIndication,
    id_LocationReport,
    id_OverloadStart,
    id_OverloadStop,
    id_WriteReplaceWarning,
    id_eNBDirectInformationTransfer,
    id_MMEDirectInformationTransfer,
    id_PrivateMessage,
    id_eNBConfigurationTransfer,
    id_MMEConfigurationTransfer,
    id_CellTrafficTrace,
    id_Kill,
    id_downlinkUEAssociatedLPPaTransport,
    id_uplinkUEAssociatedLPPaTransport,
    id_downlinkNonUEAssociatedLPPaTransport,
    id_uplinkNonUEAssociatedLPPaTransport,
    id_UERadioCapabilityMatch
}ProcedureCode_e;

extern const char* elementaryProcedureName[];

/* **************************************************************
--
-- Extension constants
--
-- *************************************************************/

#define maxPrivateIEs                        65535
#define maxProtocolExtensions                65535
#define maxProtocolIEs                       65535

/* **************************************************************
--
-- Lists
--
-- *************************************************************/

#define maxNrOfCSGs                             256
#define maxNrOfERABs                            256
#define maxnoofTAIs                             256
#define maxnoofTACs                             256
#define maxNrOfErrors                           256
#define maxnoofBPLMNs                           6
#define maxnoofPLMNsPerMME                      32
#define maxnoofEPLMNs                           15
#define maxnoofEPLMNsPlusOne                    16
#define maxnoofForbLACs                         4096
#define maxnoofForbTACs                         4096
#define maxNrOfIndividualS1ConnectionsToReset   256
#define maxnoofCells                            16
#define maxnoofTAIforWarning                    65535
#define maxnoofCellID                           65535
#define maxnoofEmergencyAreaID                  65535
#define maxnoofCellinTAI                        65535
#define maxnoofCellinEAI                        65535
#define maxnoofeNBX2TLAs                        2
#define maxnoofeNBX2ExtTLAs                     16
#define maxnoofeNBX2GTPTLAs                     16
#define maxnoofRATs                             8
#define maxnoofGroupIDs                         65535
#define maxnoofMMECs                            256
#define maxnoofCellIDforMDT                     32
#define maxnoofTAforMDT                         8

/* **************************************************************
--
-- IEs
--
-- *************************************************************/

typedef enum IEs_c{
    id_MME_UE_S1AP_ID,
    id_HandoverType,
    id_Cause,
    id_SourceID,
    id_TargetID = 4,
    id_eNB_UE_S1AP_ID = 8,
    id_E_RABSubjecttoDataForwardingList=12,
    id_E_RABtoReleaseListHOCmd,
    id_E_RABDataForwardingItem,
    id_E_RABReleaseItemBearerRelComp,
    id_E_RABToBeSetupListBearerSUReq,
    id_E_RABToBeSetupItemBearerSUReq,
    id_E_RABAdmittedList,
    id_E_RABFailedToSetupListHOReqAck,
    id_E_RABAdmittedItem,
    id_E_RABFailedtoSetupItemHOReqAck,
    id_E_RABToBeSwitchedDLList,
    id_E_RABToBeSwitchedDLItem,
    id_E_RABToBeSetupListCtxtSUReq,
    id_TraceActivation,
    id_NAS_PDU,
    id_E_RABToBeSetupItemHOReq,
    id_E_RABSetupListBearerSURes,
    id_E_RABFailedToSetupListBearerSURes,
    id_E_RABToBeModifiedListBearerModReq,
    id_E_RABModifyListBearerModRes,
    id_E_RABFailedToModifyList,
    id_E_RABToBeReleasedList,
    id_E_RABFailedToReleaseList,
    id_E_RABItem,
    id_E_RABToBeModifiedItemBearerModReq,
    id_E_RABModifyItemBearerModRes,
    id_E_RABReleaseItem,
    id_E_RABSetupItemBearerSURes,
    id_SecurityContext,
    id_HandoverRestrictionList=41,
    id_UEPagingID=43,
    id_pagingDRX=44,
    id_TAIList=46,
    id_TAIItem,
    id_E_RABFailedToSetupListCtxtSURes,
    id_E_RABReleaseItemHOCmd,
    id_E_RABSetupItemCtxtSURes,
    id_E_RABSetupListCtxtSURes,
    id_E_RABToBeSetupItemCtxtSUReq,
    id_E_RABToBeSetupListHOReq=53,
    id_GERANtoLTEHOInformationRes=55,
    id_UTRANtoLTEHOInformationRes=57,
    id_CriticalityDiagnostics,
    id_Global_ENB_ID,
    id_eNBname,
    id_MMEname=61,
    id_ServedPLMNs=63,
    id_SupportedTAs,
    id_TimeToWait,
    id_uEaggregateMaximumBitrate,
    id_TAI=67,
    id_E_RABReleaseListBearerRelComp=69,
    id_cdma2000PDU,
    id_cdma2000RATType,
    id_cdma2000SectorID,
    id_SecurityKey,
    id_UERadioCapability,
    id_GUMMEI_ID=75,
    id_E_RABInformationListItem=78,
    id_Direct_Forwarding_Path_Availability,
    id_UEIdentityIndexValue=80,
    id_cdma2000HOStatus=83,
    id_cdma2000HORequiredIndication=84,
    id_E_UTRAN_Trace_ID=86,
    id_RelativeMMECapacity,
    id_SourceMME_UE_S1AP_ID,
    id_Bearers_SubjectToStatusTransfer_Item,
    id_eNB_StatusTransfer_TransparentContainer,
    id_UE_associatedLogicalS1_ConnectionItem,
    id_ResetType,
    id_UE_associatedLogicalS1_ConnectionListResAck,
    id_E_RABToBeSwitchedULItem,
    id_E_RABToBeSwitchedULList,
    id_S_TMSI,
    id_cdma2000OneXRAND,
    id_RequestType,
    id_UE_S1AP_IDs,
    id_EUTRAN_CGI,
    id_OverloadResponse,
    id_cdma2000OneXSRVCCInfo,
    id_E_RABFailedToBeReleasedList,
    id_Source_ToTarget_TransparentContainer,
    id_ServedGUMMEIs,
    id_SubscriberProfileIDforRFP,
    id_UESecurityCapabilities,
    id_CSFallbackIndicator,
    id_CNDomain,
    id_E_RABReleasedList,
    id_MessageIdentifier,
    id_SerialNumber,
    id_WarningAreaList,
    id_RepetitionPeriod,
    id_NumberofBroadcastRequest,
    id_WarningType,
    id_WarningSecurityInfo,
    id_DataCodingScheme,
    id_WarningMessageContents,
    id_BroadcastCompletedAreaList,
    id_Inter_SystemInformationTransferTypeEDT,
    id_Inter_SystemInformationTransferTypeMDT,
    id_Target_ToSource_TransparentContainer,
    id_SRVCCOperationPossible,
    id_SRVCCHOIndication,
    id_NAS_DownlinkCount,
    id_CSG_Id,
    id_CSG_IdList,
    id_SONConfigurationTransferECT,
    id_SONConfigurationTransferMCT,
    id_TraceCollectionEntityIPAddress,
    id_MSClassmark2,
    id_MSClassmark3,
    id_RRC_Establishment_Cause,
    id_NASSecurityParametersfromE_UTRAN,
    id_NASSecurityParameterstoE_UTRAN,
    id_DefaultPagingDRX,
    id_Source_ToTarget_TransparentContainer_Secondary,
    id_Target_ToSource_TransparentContainer_Secondary,
    id_EUTRANRoundTripDelayEstimationInfo,
    id_BroadcastCancelledAreaList,
    id_ConcurrentWarningMessageIndicator,
    id_Data_Forwarding_Not_Possible,
    id_ExtendedRepetitionPeriod,
    id_CellAccessMode,
    id_CSGMembershipStatus,
    id_LPPa_PDU,
    id_Routing_ID,
    id_Time_Synchronization_Info,
    id_PS_ServiceNotAvailable,
    id_PagingPriority,
    id_x2TNLConfigurationInfo,
    id_eNBX2ExtendedTransportLayerAddresses,
    id_GUMMEIList,
    id_GW_TransportLayerAddress,
    id_Correlation_ID,
    id_SourceMME_GUMMEI,
    id_MME_UE_S1AP_ID_2,
    id_RegisteredLAI,
    id_RelayNode_Indicator,
    id_TrafficLoadReductionIndication,
    id_MDTConfiguration,
    id_MMERelaySupportIndicator,
    id_GWContextReleaseIndication,
    id_ManagementBasedMDTAllowed,
    id_PrivacyIndicator,
    id_Time_UE_StayedInCell_EnhancedGranularity,
    id_HO_Cause,
    id_VoiceSupportMatchIndicator,
    id_GUMMEIType,
}IEs_e;

extern const char* IEName[];

#endif  /* !_S1AP_CONSTANTS_H */
