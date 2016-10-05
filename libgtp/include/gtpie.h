/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   gtpie.h
 * @author Vicent Ferrer
 * @date   March, 2013
 * @brief  GTPv2 encoding and decoding IE functions. Based on OpenGGSN.
 *
 *
 * Encapsulation
 * - gtpie_tlv, gtpie_tv0, gtpie_tv1, gtpie_tv2 ... Adds information
 * elements to a buffer.
 *
 * Decapsulation
 *  - gtpie_decaps: Returns array with pointers to information elements.
 *  - getie_getie: Returns the pointer of a particular element.
 *  - gtpie_gettlv: Copies tlv information element. Return 0 on success.
 *  - gtpie_gettv: Copies tv information element. Return 0 on success.
 *
 */

#pragma once

/* Macroes for conversion between host and network byte order */
#define hton8(x)  (x)
#define ntoh8(x)  (x)
#define hton16(x) htons(x)
#define ntoh16(x) ntohs(x)
#define hton32(x) htonl(x)
#define ntoh32(x) ntohl(x)

#define hton24(x) hton32(x)>>8
#define ntoh24(x) ntoh32(x)>>8

#if BYTE_ORDER == LITTLE_ENDIAN
static __inline uint64_t
hton64(uint64_t q)
{
    register uint32_t u, l;
    u = q >> 32;
    l = (uint32_t) q;

    return htonl(u) | ((uint64_t)htonl(l) << 32);
}

#define ntoh64(_x)        hton64(_x)

#elif BYTE_ORDER == BIG_ENDIAN

#define hton64(_x)        (_x)
#define ntoh64(_x)        hton64(_x)

#else
#error  "Please fix <machine/endian.h>"
#endif


#define GTPIE_SIZE 256          /* Max number of information elements */
#define GTPIE_MAX  0xffff       /* Max length of information elements */
#define GTPIE_MAX_TV 28         /* Max length of type value pair */
#define GTPIE_MAX_TLV 0xffff-3  /* Max length of TLV (GTP length is 16 bit) */
#define GTP2IE_MAX 0xffff-4     /* Max length of TLIV GTPc2V */


#define GTPIE_DEBUG 0           /* Print debug information */

/*--------------------------------------------------------------------------*
 *   G T P V 2 C    I E  T Y P E     M A C R O     D E F I N I T I O N S    *
 *--------------------------------------------------------------------------*/

/*#define GTPV2C_IE_RESERVED                                           0*/
#define GTPV2C_IE_IMSI                                               1
#define GTPV2C_IE_CAUSE                                              2
#define GTPV2C_IE_RECOVERY                                           3
/* 4 to 50  => reserved for S101 */
#define GTPV2C_IE_STN_SR                                             51
/* 52 to 70 => Reserved for Sv interface */
#define GTPV2C_IE_APN                                                71
#define GTPV2C_IE_AMBR                                               72
#define GTPV2C_IE_EBI                                                73
#define GTPV2C_IE_IP_ADDRESS                                         74
#define GTPV2C_IE_MEI                                                75
#define GTPV2C_IE_MSISDN                                             76
#define GTPV2C_IE_INDICATION                                         77
#define GTPV2C_IE_PCO                                                78
#define GTPV2C_IE_PAA                                                79
#define GTPV2C_IE_BEARER_LEVEL_QOS                                   80
#define GTPV2C_IE_FLOW_QOS                                           81
#define GTPV2C_IE_RAT_TYPE                                           82
#define GTPV2C_IE_SERVING_NETWORK                                    83
#define GTPV2C_IE_BEARER_TFT                                         84
#define GTPV2C_IE_TAD                                                85
#define GTPV2C_IE_ULI                                                86
#define GTPV2C_IE_FTEID                                              87
#define GTPV2C_IE_TMSI                                               88
#define GTPV2C_IE_GLOBAL_CN_ID                                       89
#define GTPV2C_IE_S103PDF                                            90
#define GTPV2C_IE_S1UDF                                              91
#define GTPV2C_IE_DELAY_VALUE                                        92
#define GTPV2C_IE_BEARER_CONTEXT                                     93
#define GTPV2C_IE_CHARGING_ID                                        94
#define GTPV2C_IE_CHARGING_CHARACTERISTICS                           95
#define GTPV2C_IE_TRACE_INFORMATION                                  96
#define GTPV2C_IE_BEARER_FLAGS                                       97
/*#define GTPV2C_RESERVED                                              98 */
#define GTPV2C_IE_PDN_TYPE                                           99
#define GTPV2C_IE_PROCEDURE_TRANSACTION_ID                           100
/* 101 & 102 Reserved
 * #define GTPV2C_IE_DRX_PARAMETER                                      101
 * #define GTPV2C_IE_UE_NETWORK_CAPABILITY                              102*/
#define GTPV2C_IE_MM_CTX_GSM_KEY_3                                   103
#define GTPV2C_IE_MM_CTX_UMTS_KEY_USED_CIPHER _5                     104
#define GTPV2C_IE_MM_CTX_GSM_KEY_USED_CIPHER_5                       105
#define GTPV2C_IE_MM_CTX_UMTS_KEY_5                                  106
#define GTPV2C_IE_MM_CTX_EPS_SEC_CTX_4_5                             107
#define GTPV2C_IE_MM_CTX_UMTS_KEY4_5                                 108
#define GTPV2C_IE_PDN_CONNECTION                                     109
#define GTPV2C_IE_PDU_NUMBERS                                        110
#define GTPV2C_IE_PTMSI                                              111
#define GTPV2C_IE_PTMSI_SIGNATURE                                    112
#define GTPV2C_IE_HOP_COUNTER                                        113
#define GTPV2C_IE_UE_TIME_ZONE                                       114
#define GTPV2C_IE_TRACE_REFERENCE                                    115
#define GTPV2C_IE_COMPLETE_REQUEST_MESSAGE                           116
#define GTPV2C_IE_GUTI                                               117
#define GTPV2C_IE_F_CONTAINER                                        118
#define GTPV2C_IE_F_CAUSE                                            119
#define GTPV2C_IE_PLMN_ID                                            120
#define GTPV2C_IE_TARGET_IDENTIFICATION                              121
#define GTPV2C_IE_PACKET_FLOW_ID                                     123
#define GTPV2C_IE_RAB_CONTEXT                                        124
#define GTPV2C_IE_SOURCE_RNC_PDCP_CONTEXT_INFO                       125
#define GTPV2C_IE_UDP_SOURCE_PORT_NUMBER                             126
#define GTPV2C_IE_APN_RESTRICTION                                    127
#define GTPV2C_IE_SELECTION_MODE                                     128
#define GTPV2C_IE_SOURCE_IDENTIFICATION                              129
/*#define GTPV2C_RESERVED                                             130 */
#define GTPV2C_IE_CHANGE_REPORTING_ACTION                            131
#define GTPV2C_IE_FQ_CSID                                            132
#define GTPV2C_IE_CHANNEL_NEEDED                                     133
#define GTPV2C_IE_EMLPP_PRIORITY                                     134
#define GTPV2C_IE_NODE_TYPE                                          135
#define GTPV2C_IE_FQDN                                               136
#define GTPV2C_IE_TI                                                 137
#define GTPV2C_IE_MBMS_SESSION_DURATION                              138
#define GTPV2C_IE_MBMS_SERIVCE_AREA                                  139
#define GTPV2C_IE_MBMS_SESSION_IDENTIFIER                            140
#define GTPV2C_IE_MBMS_FLOW_IDENTIFIER                               141
#define GTPV2C_IE_MBMS_IP_MULTICAST_DISTRIBUTION                     142
#define GTPV2C_IE_MBMS_IP_DISTRIBUTION_ACK                           143
#define GTPV2C_IE_RFSP_INDEX                                         144
#define GTPV2C_IE_UCI                                                145
#define GTPV2C_IE_CSG_INFORMATION_REPORTING_ACTION                   146
#define GTPV2C_IE_CSG_ID                                             147
#define GTPV2C_IE_CSG_MEMBERSHIP_INDICATION                          148
#define GTPV2C_IE_SERVICE_INDICATOR                                  149
#define GTPV2C_IE_DETACH_TYPE                                        150
#define GTPV2C_IE_LDN                                                151
#define GTPV2C_IE_NODE_FEATURES                                      152
#define GTPV2C_IE_MBMS_TIME_TO_DATA_TRANSFER                         153
#define GTPV2C_IE_THROTTLING                                         154
#define GTPV2C_IE_ARP                                                155
#define GTPV2C_IE_EPC_TIMER                                          156
#define GTPV2C_IE_SIGNAL_PRIORITY_IND                                157
#define GTPV2C_IE_TMGI                                               158
#define GTPV2C_IE_ADDITIONAL_MM_CONTEXT_FOR_SRVCC                    159
#define GTPV2C_IE_ADDITIONAL_FLAGS_FOR_SRVCC                         160
/*#define GTPV2C_RESERVED                                             161*/
#define GTPV2C_IE_MDT_CONFIGURATION                                  162
#define GTPV2C_IE_APCO                                               163
#define GTPV2C_IE_ABSOLUTE_TIME_OF_MBMS_DATA_TRANSFER                164
#define GTPV2C_IE_ENB_INFORMATION_REPORTING                          165
#define GTPV2C_IE_IP4CP                                              166
#define GTPV2C_IE_CHANGE_TO_REPORT_FLAGS                             167
#define GTPV2C_IE_ACTIVATION_INDICATION                              168
#define GTPV2C_IE_TWAN_ID                                            169
/* 170 to 254  => Spare. For future use. */
#define GTPV2C_IE_PRIVATE_EXTENSION                                  255
#define GTPV2C_IE_TYPE_MAXIMUM                                       256

/*--------------------------------------------------------------------------*
 *   G T P V 2 C      C A U S E      V A L U E     D E F I N I T I O N S    *
 *--------------------------------------------------------------------------*/
/*Request / Initial message */

#define GTPV2C_CAUSE_RESERVED                                        1
#define GTPV2C_CAUSE_LOCAL_DETACH                                    2
#define GTPV2C_CAUSE_COMPLET_DETACH                                  3
#define GTPV2C_CAUSE_RAT_CHANGED_FROM_3GPP_TO_NON_3GPP               4
#define GTPV2C_CAUSE_ISR_DEATIVATION                                 5
#define GTPV2C_CAUSE_ERROR_IND_RECEIVED_FROM_RCN_END_S4_SGSN         6
#define GTPV2C_CAUSE_IMSI_DETACH_ONLY                                7
#define GTPV2C_CAUSE_REACTIVATION_REQUESTED                          8
#define GTPV2C_CAUSE_PDN_RECONECTION_TO_THIS_APN_DISALLOWED          9
#define GTPV2C_CAUSE_ACCESS_CHANGED_FROM_NON_3GPP_TO_3GPP            10
#define GTPV2C_CAUSE_PDN_RECONECTION_INACTIVITY_TIME_EXPIRES         11
#define GTPV2C_CAUSE_PGW_NOT_RESPONDING                              12
#define GTPV2C_CAUSE_NETWORK_FAILURE                                 13
#define GTPV2C_CAUSE_QOS_PARAMETER_MISMATCH                          14
/* 15 Spare */
/*240 to 255      Spare.*/

/*Acceptance in a Response /triggered message. */
#define GTPV2C_CAUSE_REQUEST_ACCEPTED                                16
#define GTPV2C_CAUSE_REQUEST_ACCEPTED_PARTIALLY                      17
#define GTPV2C_CAUSE_NEW_PDN_TYPE_DUE_TO_NETWOEK_PREFERENCE          18
#define GTPV2C_CAUSE_NEW_PDN_TYPE_DUE_TO_SINGLE_ADDRESSS_BEARER_ONLY 19
/*20 to -63  Spare. */

/*Rejection in a Response / triggered message. */
#define GTPV2C_CAUSE_CONTEXT_NOT_FOUND                               64
#define GTPV2C_CAUSE_INVALID_MESSAGE_FORMAT                          65
#define GTPV2C_CAUSE_VERSION_NOT_SUPPORTED_BY_NEXT_PEER              66
#define GTPV2C_CAUSE_INVALID_LENGTH                                  67
#define GTPV2C_CAUSE_SERVICE_NOT_SUPPORTED                           68
#define GTPV2C_CAUSE_MANDATORY_IE_INCORRECT                          69
#define GTPV2C_CAUSE_MANDATORY_IE_MISSING                            70
/*71 shall not be used*/
#define GTPV2C_CAUSE_SYSTEM_FAILURE                                  72
#define GTPV2C_CAUSE_NO_RESOURCESS_AVAILABLE                         73
#define GTPV2C_CAUSE_SEMANTIC_ERROR_IN_THE_TFT_OPERATION             74
#define GTPV2C_CAUSE_SYNTACTIC_ERROR_IN_THE_TFT_OPERATION            75
#define GTPV2C_CAUSE_SEMANTIC_ERRORS_IN_PACKET_FILTER                76
#define GTPV2C_CAUSE_SYNTACTIC_ERRORS_IN_PACKET_FILTER               77
#define GTPV2C_CAUSE_MISSING_OR_UNKNOWN_APN                          78
/*79 shall not be used*/
#define GTPV2C_CAUSE_GRE_NOT_FOUND                                   80
#define GTPV2C_CAUSE_RELOCATION_FAILURE                              81
#define GTPV2C_CAUSE_DENIED_IN_RAT                                   82
#define GTPV2C_CAUSE_PREFERRED_PDN_TYPE_NOT_SUPPORTED                83
#define GTPV2C_CAUSE_ALL_DYNAMIC_ADDRESSES_ARE_OCCUPIED              84
#define GTPV2C_CAUSE_UE_CONTEXT_WITHOUT_TFT_ALREADY_ACTIVATED        85
#define GTPV2C_CAUSE_PROTOCOL TYPE_NOT_SUPPORTED                     86
#define GTPV2C_CAUSE_UE_NOT_RESPONDING                               87
#define GTPV2C_CAUSE_UE_REFUSES                                      88
#define GTPV2C_CAUSE_SERVICE_DENIED                                  89
#define GTPV2C_CAUSE_UNABLE_TO_PAGE_UE                               90
#define GTPV2C_CAUSE_NO_MEMORY_AVAILABLE                             91
#define GTPV2C_CAUSE_USER_AUTHENTICATION_FAILED                      92
#define GTPV2C_CAUSE_APN_ACCESS_DENIED_NO_SUBSCRIPTION               93
#define GTPV2C_CAUSE_REQUEST_REJECTED                                94
#define GTPV2C_CAUSE_PTMSI_SIGNATURE_MISMATCH                        95
#define GTPV2C_CAUSE_IMSI_IMEI_NOT_KNOWN                             96
#define GTPV2C_CAUSE_SEMANTIC_ERROR_IN_TAD_OPERATION                 97
#define GTPV2C_CAUSE_SYNTACTIC_ERROR_IN_TAD_OPERATION                98
/* 99  Shall not be used. */
#define GTPV2C_CAUSE_REMOTE_PEER_NOT_RESPONDING                      100
#define GTPV2C_CAUSE_COLLISION_WITH_NETWORK_INITIATED_REQ            101
#define GTPV2C_CAUSE_UNABLE_TO_PAGE_UE_DUE_TO_SUSPENSION             102
#define GTPV2C_CAUSE_CONDITIONAL_IE_MISSING                          103
#define GTPV2C_APN_RESTRICTION_TYPE_IMCOMPATIBLE_WITH_CURRENTLY_ACTIVE_PDN_CONNECTION      104
#define GTPV2C_CAUSE_UNVALID_OVERALL_LENGH_OF_TRIG_RSP_MSG_AND_PIGGYBACKED_INITIAL_MSG     105
#define GTPV2C_CAUSE_DATA_FORWARDING_NOT_SUPPORTED                   106
#define GTPV2C_CAUSE_INVALID_REPLY_FROM_REMOTE_PEER                  107
#define GTPV2C_CAUSE_FALLBACK_TO_GTPV1                               108
#define GTPV2C_CAUSE_INVALID_PEER                                    109
#define GTPV2C_CAUSE_TEMPORARILY_REJECTED_DUE_TO_HANDOVER_PROCEDURE_IN_PROGRESS             110
#define GTPV2C_CAUSE_MODIFICATIONS_NOT_LIMITED_TO_S1U_BEARERS        111
#define GTPV2C_CAUSE_REQ_REJ_FOR_A_PMIPV6_REASON                     112
#define GTPV2C_CAUSE_APN_CONGESTION                                  113
#define GTPV2C_CAUSE_BEARER_HANDLING_NOT_SUPPORTED                   114
#define GTPV2C_CAUSE_UE_ALREADY_REATTACHED                           115
#define GTPV2C_CAUSE_MULTIPLE_PDN_CONNECTIONS_FOR_APN_NOT_ALLOWED    116
/*117 to 239      Spare*/



#define GTPV2C_IE_INSTANCE_ZERO                                      (0)
#define GTPV2C_IE_INSTANCE_ONE                                       (1)
#define GTPV2C_IE_INSTANCE_TWO                                       (2)
#define GTPV2C_IE_INSTANCE_THREE                                     (3)
#define GTPV2C_IE_INSTANCE_FOUR                                      (4)
#define GTPV2C_IE_INSTANCE_MAXIMUM                                   (GTPV2C_IE_INSTANCE_FOUR)

#define GTPV2C_IE_PRESENCE_MANDATORY                                 (1)
#define GTPV2C_IE_PRESENCE_CONDITIONAL                               (2)
#define GTPV2C_IE_PRESENCE_CONDITIONAL_OPTIONAL                      (3)
#define GTPV2C_IE_PRESENCE_OPTIONAL                                  (4)

struct gtp2ie_tliv
{
    uint8_t  t;                   /* Type */
    uint16_t l;                   /* Length */
    uint8_t  i;                   /* Instance */
    uint8_t  v[GTP2IE_MAX];       /* Value */
} __attribute__((packed));

struct gtp2ie_tli
{
    uint8_t  t;
    uint16_t l;
    uint8_t  i;
} __attribute__((packed));


union gtpie_member {
    uint8_t t;
    struct gtp2ie_tliv    tliv;
    struct gtp2ie_tli     tli;
}__attribute__((packed));

/**IE value structures*/
/**F_TEID Bearer QoS    See 3GPP TS 29.274, clause 8.22*/
/** interface type*/
enum iface_type{
    S1U_eNB = 0,        /*S1-U eNodeB GTP-U interface*/
    S1U_SGW,            /*S1-U SGW GTP-U interface*/
    S12_RNC,            /*S12 RNC GTP-U interface*/
    S12_SGW,            /*S12 SGW GTP-U interface*/
    S5S8U_SGW,          /*S5/S8 SGW GTP-U interface*/
    S5S8U_PGW,          /*S5/S8 PGW GTP-U interface*/
    S5S8C_SGW,          /*S5/S8 SGW GTP-C interface*/
    S5S8C_PGW,          /*S5/S8 PGW GTP-C interface*/
    S5S8_PMIPv6_SGW,    /*S5/S8 SGW PMIPv6 interface (the 32 bit GRE key is encoded in 32 bit TEID field and since alternate CoA is not used the control plane and user plane addresses are the same for PMIPv6)*/
    S5S8_PMIPv6_PGW,    /*S5/S8 PGW PMIPv6 interface (the 32 bit GRE key is encoded in 32 bit TEID field and the control plane and user plane addresses are the same for PMIPv6)*/
    S11_MME,            /*S11 MME GTP-C interface*/
    S11S4_SGW,          /*S11/S4 SGW GTP-C interface*/
    S10_MME,            /*S10 MME GTP-C interface*/
    S3_MME,             /*S3 MME GTP-C interface*/
    S3_SGSN,            /*S3 SGSN GTP-C interface*/
    S4U_SGSN,           /*S4 SGSN GTP-U interface*/
    S4U_SGW,            /*S4 SGW GTP-U interface*/
    S4C_SGSN,           /*S4 SGSN GTP-C interface*/
    S16_SGSN,           /*S16 SGSN GTP-C interface*/
    eNB_U_DL,           /*eNodeB GTP-U interface for DL data forwarding*/
    eNB_U_UL,           /*eNodeB GTP-U interface for UL data forwarding*/
    RNC_U,              /*RNC GTP-U interface for data forwarding*/
    SGSN_U,             /*SGSN GTP-U interface for data forwarding*/
    SGW_U_DL,           /*GW GTP-U interface for DL data forwarding*/
    SM_MBMS_C,          /*Sm MBMS GW GTP-C interface*/
    SN_MBMS_C,          /*Sn MBMS GW GTP-C interface*/
    SM_MME_C,           /*Sm MME GTP-C interface*/
    SN_SGSN_C,          /*Sn SGSN GTP-C interface*/
    SGW_U_UL,           /*SGW GTP-U interface for UL data forwarding*/
    Sn_SGSN_U,          /*Sn SGSN GTP-U interface*/
    S2bC_ePDG,          /*S2b ePDG GTP-C interface*/
    S2bU_ePDG,          /*S2b-U ePDG GTP-U interface*/
    S2bC_PGW,           /*S2b PGW GTP-C interface*/
    S2bU_PGW,           /*S2b-U PGW GTP-U interface*/
    S2aU_TWAN,          /*2a TWAN GTP-U interface*/
    S2aC_TWAN,          /*S2a TWAN GTP-C interface*/
    S2aC_PGW,           /*S2a PGW GTP-C interface*/
    S2aU_PGW            /*S2a PGW GTP-U interface*/
};

/*Structure sizes*/
#define FTEID_IP4_SIZE  (9)
#define FTEID_IP6_SIZE  (21)
#define FTEID_IP46_SIZE (25)

struct fteid_t{
    /*The first byte has this order to store it as a network bit order*/
    uint8_t     iface   :6; /*  ..000000    F-TEID Interface type */
    uint8_t     ipv6    :1; /*  .0......    IPv6 flag*/
    uint8_t     ipv4    :1; /*  0.......    IPv6 flag*/
    uint32_t    teid;       /* Stored on network format, use ntohl() to show it*/
    union{
        uint32_t    addrv4;
        uint8_t     addrv6[16];
        struct{
            uint32_t    addrv4;
            uint8_t     addrv6[16];
        }both;
    }addr;
}__attribute__((packed));

/*Bearer QoS    See 3GPP TS 29.274, clause 8.15*/
struct qos_t{
    uint8_t     pvi     :1; /*  .......0    PVI (Pre-emption Vulnerability): See 3GPP TS 29.212, clause 5.3.47 Pre-emption-Vulnerability AVP*/
    uint8_t     spare1  :1; /*  ......0.    SPARE   */
    uint8_t     pl      :4; /*  ..0000..    PL (Priority Level): See 3GPP TS 29.212, clause 5.3.45 ARP-Value AVP. PL encodes each priority level defined for the ARP-Value AVP as the binary value of the priority level.*/
    uint8_t     pci     :1; /*  .0......    PCI (Pre-emption Capability): See 3GPP TS 29.212, clause 5.3.46 Pre-emption-Capability AVP.*/
    uint8_t     spare2  :1; /*  0.......    SPARE   */
    uint8_t     qci;        /*   "QCI" value, as specified in 3GPP TS 23.203*/
    /*The UL/DL MBR and GBR fields are encoded as kilobits per second (1 kbps = 1000 bps) in binary value.
     *  For non-GBR bearers, both the UL/DL MBR and GBR should be set to zero.*/
    uint64_t    mbr_ul  :40;/*  Maximum bit rate for uplink */
    uint64_t    mbr_dl  :40;/*  Maximum bit rate for downlink */
    uint64_t    gbr_ul  :40;/*  Guaranteed bit rate for uplink */
    uint64_t    gbr_dl  :40;/*  Guaranteed bit rate for downlink*/
}__attribute__((packed));


/* PDN Address Allocation See 3GPP TS 29.274, clause 8.14*/
#define PAA_IP4  (1)
#define PAA_IP6  (2)
#define PAA_IP46 (3)

struct PAA_t{
    uint8_t type:3;
    uint8_t spare:5;
    union{
        uint32_t ipv4;
        uint8_t  ipv6[16];
        struct{
            uint8_t		ipv6[16];
            uint32_t	ipv4;
        }both;
    }addr;
}__attribute__((packed));
/*
  cause
  imsi
  rai
  tlli
  p_tmsi
  qos_profile0
  reorder
  auth
  map_cause
  p_tmsi_s
  ms_validated
  recovery
  selection_mode
  tei_di
  tei_c
  tei_dii
  teardown
  nsapi
  ranap_cause
  rab_context
  rp_sms
  rp
  pfi
  charging_c
  trace_ref
  trace_type
  ms_not_reach
  charging_id
  eua
  mm_context
  pdp_context
  apn
  pco
  gsn_addr
  msisdn
  qos_profile
  auth
  tft
  target_inf
  utran_trans
  rab_setup
  ext_header_t
  trigger_id
  omc_id
  charging_addr
  private
*/

struct tlv1 {
    uint8_t  type;
    uint8_t  length;
}__attribute__((packed));

struct tlv2 {
    uint8_t  type;
    uint16_t length;
}__attribute__((packed));

extern int gtp2ie_tliv(void *p, unsigned int *length, unsigned int size,  uint8_t type, uint8_t instance,  uint8_t value[], uint16_t vsize);

extern int gtp2ie_gettliv(union gtpie_member* ie[], int type, int instance, uint8_t *dst, uint16_t *iesize);

extern int gtp2ie_decap(union gtpie_member* ie[], void *pack, unsigned len);
extern int gtp2ie_encaps(union gtpie_member ie[], unsigned int size, void *pack, unsigned *len);

extern int gtp2ie_decaps_group(union gtpie_member *ie[], unsigned int *size, void *from,  unsigned int len);
extern int gtp2ie_encaps_group(int type, int instance, void* to, union gtpie_member ie[], unsigned int size);

/**@brief Converts TBCD network field to hardware decimal.
 *
 * Used on some IE value fields, i.e IMSI
 * @param [out] dec destination of the decoded value on decimal base
 * @param [in]  tbcd pointer to the tbcd field
 * @param [in]  length tbcd number of bytes
 * */
extern void tbcd2dec(uint64_t *dec, const uint8_t *tbcd, const uint32_t length);

/**@brief Converts hardware decimal to TBCD network field.
 *
 * Used on some IE value fields, i.e IMSI. The memory allocation of the tbcd buffer must be done before calling this function.
 * The maximum length is 8 bytes, 16 BSD digits.
 * @param [out]  tbcd pointer to the tbcd field
 * @param [out]  length tbcd number of bytes
 * @param [oin]  dec decimal value to be encoded
 * */
extern void dec2tbcd(uint8_t *tbcd, uint32_t *length, const uint64_t dec);
