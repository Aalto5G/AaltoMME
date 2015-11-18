/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   NASConstants.h
 * @Author Vicent Ferrer
 * @date   May, 2013
 * @brief  NAS Constants
 */

#ifndef _NAS_CONSTANTS_H
#define _NAS_CONSTANTS_H
#include <stdint.h>

/*Security header type (octet 1), 3gpp 24.301 Table 9.3.1: Security header type */
typedef enum SecurityHeaderType_c{
    PlainNAS = 0,
    /*Security protected NAS Message*/
    IntegrityProtected,
    IntegrityProtectedAndCiphered,
    IntegrityProtectedWithNewEPSSecurityContext,
    IntegrityProtectedAndCipheredWithNewEPSSecurityContext,
    /*Non-standard L3 message*/
    SecurityHeaderForServiceRequestMessage = 12,
    /* 1101 1111 to These values are not used in this version of the protocol.
     * If received they shall be interpreted as '1100'.*/
}SecurityHeaderType_t;


/* 3gpp 24.007, Table 11.2: Protocol discriminator values */
typedef enum ProtocolDiscriminator_c{
    GroupCallControl                    = 0x0,
    BroadcastCallControl                = 0x1,
    EPSSessionManagementMessages        = 0x2,  /*NAS - ESM*/
    CallControl                         = 0x3,
    GPRSTransparentTransportProtocol    = 0x4,  /*GTTP*/
    MobilityManagementMessages          = 0x5,
    RadioResourcesManagementMessages    = 0x6,
    EPSMobilityManagementMessages       = 0x7,  /*NAS - EMM*/
    GPRSMobilityManagementMessages      = 0x8,
    SMSMessages                         = 0x9,
    GPRSSessionManagementMessages       = 0xa,
    NonCallRelatedSSMessages            = 0xb,
    LocationServices                    = 0xc,  /*Specified In 3GPP TS 44.071 [8a]*/
/*0xd Reserved for extension of the PD to one octet length
0xf used by tests procedures described in 3GPP TS 44.014 [5a], 3GPP TS 34.109 [17a] and 3GPP TS 36.509 [26].*/
}ProtocolDiscriminator_t;

/* 3gpp 24.007, Table 11.6: Procedure transaction identity*/
typedef enum ProcedureTransactionId_c{
    NoPTIAssigned = 0x00,
    /*Procedure transaction identity values*/
    PTIReserved = 0xff
}ProcedureTransactionId_t;

/*3gpp 24.301  Table 9.8.1: Message types for EPS mobility management*/
typedef enum NASMessageType_c{
    /* 01.. .... EPS mobility management messages */
    AttachRequest                               = 0x41,
    AttachAccept                                = 0x42,
    AttachComplete                              = 0x43,
    AttachReject                                = 0x44,
    DetachRequest                               = 0x45,
    DetachAccept                                = 0x46,
    TrackingAreaUpdateRequest                   = 0x48,
    TrackingAreaUpdateAccept                    = 0x49,
    TrackingAreaUpdateComplete                  = 0x4A,
    TrackingAreaUpdateReject                    = 0x4B,
    ExtendedServiceRequest                      = 0x4C,
    ServiceReject                               = 0x4E,
    GUTIReallocationCommand                     = 0x50,
    GUTIReallocationComplete                    = 0x51,
    AuthenticationRequest                       = 0x52,
    AuthenticationResponse                      = 0x53,
    AuthenticationReject                        = 0x54,
    AuthenticationFailure                       = 0x5C,
    IdentityRequest                             = 0x55,
    IdentityResponse                            = 0x56,
    SecurityModeCommand                         = 0x5D,
    SecurityModeComplete                        = 0x5E,
    SecurityModeReject                          = 0x5F,
    EMMStatus                                   = 0x60,
    EMMInformation                              = 0x61,
    DownlinkNASTransport                        = 0x62,
    UplinkNASTransport                          = 0x63,
    CSServiceNotification                       = 0x64,
    DownlinkGenericNASTransport                 = 0x68,
    UplinkGenericNASTransport                   = 0x69,
    /* 11.. .... EPS session management messages*/
    ActivateDefaultEPSBearerContextRequest      = 0xC1,
    ActivateDefaultEPSBearerContextAccept       = 0xC2,
    ActivateDefaultEPSBearerContextReject       = 0xC3,
    ActivateDedicatedEPSBearerContextRequest    = 0xC5,
    ActivateDedicatedEPSBearerContextAccept     = 0xC6,
    ActivateDedicatedEPSBearerContextReject     = 0xC7,
    ModifyEPSBearerContextRequest               = 0xC9,
    ModifyEPSBearerContextAccept                = 0xCA,
    ModifyEPSBearerContextReject                = 0xCB,
    DeactivateEPSBearerContextRequest           = 0xCD,
    DeactivateEPSBearerContextAccept            = 0xCE,
    PDNConnectivityRequest                      = 0xD0,
    PDNConnectivityReject                       = 0xD1,
    PDNDisconnectRequest                        = 0xD2,
    PDNDisconnectReject                         = 0xD3,
    BearerResourceAllocationRequest             = 0xD4,
    BearerResourceAllocationReject              = 0xD5,
    BearerResourceModificationRequest           = 0xD6,
    BearerResourceModificationReject            = 0xD7,
    ESMInformationRequest                       = 0xD9,
    ESMInformationResponse                      = 0xDA,
    Notification                                = 0xDB,
    ESMStatus                                   = 0xE8,
}NASMessageType_t;


typedef enum ESMCause_c{
    ESM_OperatorDeterminedBarring                                    = 0x08,
    ESM_Insufficientresources                                        = 0x1A,
    ESM_MissingorunknownAPN                                          = 0x1B,
    ESM_UnknownPDNtype                                               = 0x1C,
    ESM_Userauthenticationfailed                                     = 0x1D,
    ESM_RequestRejectedByServingGWorPDNGW                            = 0x1E,
    ESM_RequestRejectedUnspecified                                   = 0x1F,
    ESM_ServiceOptionNotSupported                                    = 0x20,
    ESM_RequestedServiceOptionNotSubscribed                          = 0x21,
    ESM_ServiceOptionTemporarilyOutOfOrder                           = 0x22,
    ESM_PTIAlreadyInUse                                              = 0x23,
    ESM_RegularDeactivation                                          = 0x24,
    ESM_EPSQoSNotAccepted                                            = 0x25,
    ESM_NetworkFailure                                               = 0x26,
    ESM_ReactivationRequested                                        = 0x27,
    ESM_SemanticErrorInTheTFTOperation                               = 0x29,
    ESM_SyntacticalErrorInTheTFTOperation                            = 0x2A,
    ESM_InvalidEPSBearerIdentity                                     = 0x2B,
    ESM_SemanticErrorsInPacketFilter                                 = 0x2C,
    ESM_SyntacticalErrorsInPacketFilter                              = 0x2D,
    ESM_Unused                                                       = 0x2E,
    ESM_PTImismatch                                                  = 0x2F,
    ESM_LastPDNDisconnectionNotAllowed                               = 0x31,
    ESM_PDNTypeIPv4OnlyAllowed                                       = 0x32,
    ESM_PDNTypeIPv6OnlyAllowed                                       = 0x33,
    ESM_SingleAddressBearersOnlyAllowed                              = 0x34,
    ESM_ESMInformationNotReceived                                    = 0x35,
    ESM_PDNConnectionDoesNotExist                                    = 0x36,
    ESM_MultiplePDNConnectionsForAGivenAPNNotAllowed                 = 0x37,
    ESM_CollisionWithNetworkInitiatedRequest                         = 0x38,
    ESM_UnsupportedQCIValue                                          = 0x3B,
    ESM_BearerHandlingNotSupported                                   = 0x3C,
    ESM_MaximumNumberOfEPSBearersReached                             = 0x41,
    ESM_RequestedAPNnotSupportedInCurrentRATAndPLMNCombination       = 0x42,
    ESM_InvalidPTIValue                                              = 0x51,
    ESM_SemanticallyIncorrectMessage                                 = 0x5F,
    ESM_InvalidMandatoryInformation                                  = 0x60,
    ESM_MessageTypeNonExistentOrNotImplemented                       = 0x61,
    ESM_MessageTypeNotCompatibleWithTheProtocolState                 = 0x62,
    ESM_InformationElementNonExistentOrNotImplemented                = 0x63,
    ESM_ConditionalIEError                                           = 0x64,
    ESM_MessageNotCompatibleWithTheProtocolState                     = 0x65,
    ESM_ProtocolErrorUnspecified                                     = 0x6F,
    ESM_APNrestrictionValueIncompatibleWithActiveEPSBearerContext    = 0x70
}ESMCause_t;

typedef enum EMMCause_c{
    EMM_IMSIUnknownInHSS                                 = 0x02,
    EMM_IllegalUE                                        = 0x03,
    EMM_IMEINotAccepted                                  = 0x05,
    EMM_IllegalME                                        = 0x06,
    EMM_EPSServicesNotAllowed                            = 0x07,
    EMM_EPSServicesAndNonEPSServicesNotAllowed           = 0x08,
    EMM_UEIdentityCannotBeDerivedByTheNetwork            = 0x09,
    EMM_ImplicitlyDetached                               = 0x0A,
    EMM_PLMNNotAllowed                                   = 0x0B,
    EMM_TrackingAreaNotAllowed                           = 0x0C,
    EMM_RoamingNotAllowedInThisTrackingArea              = 0x0D,
    EMM_EPSServicesNotAllowedInThisPLMN                  = 0x0E,
    EMM_NoSuitableCellsInTrackingArea                    = 0x0F,
    EMM_MSCTemporarilyNotReachable                       = 0x10,
    EMM_NetworkFailure                                   = 0x11,
    EMM_CSDomainNotAvailable                             = 0x12,
    EMM_ESMFailure                                       = 0x13,
    EMM_MACFailure                                       = 0x14,
    EMM_SynchFailure                                     = 0x15,
    EMM_Congestion                                       = 0x16,
    EMM_UESecurityCapabilitiesMismatch                   = 0x17,
    EMM_SecurityModeRejectedUnspecified                  = 0x18,
    EMM_NotAuthorizedForThisCSG                          = 0x19,
    EMM_NonEPSAuthenticationUnacceptable                 = 0x1A,
    EMM_RequestedServiceOptionNotAuthorizedInThisPLMN    = 0x23,
    EMM_CSServiceTemporarilyNotAvailable                 = 0x27,
    EMM_NoEPSBearerContextActivated                      = 0x28,
    EMM_SevereNetworkFailure                             = 0x2A,
    EMM_SemanticallyNcorrectMessage                      = 0x5F,
    EMM_InvalidMandatoryInformation                      = 0x60,
    EMM_MessageTypeNonExistentOrNotImplemented           = 0x61,
    EMM_MessageTypeNotCompatibleWithTheProtocolState     = 0x62,
    EMM_InformationElementNonExistentOrNotImplemented    = 0x63,
    EMM_ConditionalIEError                               = 0x64,
    EMM_MessageNotCompatibleWithTheProtocolState         = 0x65,
    EMM_ProtocolErrorUnspecified                         = 0x6F
}EMMCause_t;

#endif  /* !_NAS_CONSTANTS_H */
