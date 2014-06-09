/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S1AP_IE.h
 * @Author Vicent Ferrer
 * @date   April, 2013
 * @brief  S1AP IE definitions
 *
 * S1AP protocol Information Element (IE) types definition.
 */

#ifndef S1AP_IE_H_
#define S1AP_IE_H_

#include <stdint.h>

#include "Containers.h"

/** Cause to string conversion arrays*/
extern const char *CauseRadioNetworkName[];
extern const char *CauseRadioNetwork_extName[];

extern const char *CauseTransportName[];

extern const char *CauseNASName[];
extern const char *CauseNAS_extName[];

extern const char *CauseProtocolName[];

extern const char *CauseMiscName[];

typedef void *(*iEconstructor)();

extern iEconstructor getconstructor[];
/*IE and its values*/

typedef struct GenericVal_c{
    void        (*freeIE)(void*);
    void        (*showIE)(void*);

}GenericVal_t;

/**@brief TBCD String
 *
 * ASN.1
 * TypeOfError ::= ENUMERATED {
 *     not-understood,
 *     missing,
 *     ...
 * }
 */
typedef enum TypeOfError_c{
    not_understood,
    missing,
}TypeOfError_e;

extern const char *TypeOfErrorName[];

/**@brief TBCD String
 *
 * Digits 0 to 9, encoded 0000 to 1001, 1111 used as filler digit,
 * two digits per octet,
 * - bits 4 to 1 of octet n encoding digit 2n-1
 * - bits 8 to 5 of octet n encoding digit 2n
 *
 * ASN.1
 * TBCD-STRING ::= OCTET STRING (SIZE (3))
 */
typedef struct TBCD_STRING_c {
    uint8_t s[3];
}TBCD_STRING_t;


/**@brief TAC
 *
 * This information element is used to uniquely identify a Tracking Area Code. (TAC) (clause 9.2.3.7 )
 * ASN.1
 * TAC ::= OCTET STRING (SIZE (2))
 */
typedef struct TAC_c {
    uint8_t s[2];
}TAC_t;


/** @brief PagingDRX
 *
 * This IE indicates the Paging DRX as defined in TS 36.304
 *
 * ASN.1
 * PagingDRX ::= ENUMERATED { v32,v64,v128,v256,  ...}
 */
typedef struct PagingDRX_e{
    void    (*freeIE)(void*);
    void    (*showIE)(void*);
    enum{
        v32,
        v64,
        v128,
        v256
    }pagingDRX;
}PagingDRX_t;

extern const char *PagingDRXName[];

/** @brief Constructor of PLMNidentity type
 *  @return PLMNidentity_t allocated  and initialized structure
 * */
PagingDRX_t* new_PagingDRX();


/**@brief PLMN identity structure
 *
 * Clause 9.2.3.8
 *
 * The PLMN identity consists of 3 digits from MCC followed by either
 * a filler digit plus 2 digits from MNC (in case of 2 digit MNC) or
 * 3 digits from MNC (in case of a 3 digit MNC).
 *
 * ASN.1
 * PLMNidentity                ::= TBCD-STRING
 * */
typedef struct PLMNidentity_c{
    void    (*freeIE)(void*);
    void    (*showIE)(void*);
    TBCD_STRING_t   tbc;
    uint16_t         MCC;
    uint16_t         MNC;
}PLMNidentity_t;

/** @brief Constructor of PLMNidentity type
 *  @return PLMNidentity_t allocated  and initialized structure
 * */
PLMNidentity_t* new_PLMNidentity();

/**@brief convert tbc field to MCC and MNC values
 *
 * The results are stored on the corresponding structure fields.
 * */
extern void plmnId_tbcd2MccMnc(PLMNidentity_t* self);

/**@brief convert MCC and MNC values to tbc field
 *
 * The results are stored on the corresponding structure fields.
 * */
extern void plmnId_MccMnc2tbcd(PLMNidentity_t* self);


/**@brief ENB-ID
 *
 * ASN.1:
 * ENB-ID ::= CHOICE {
 *      macroENB-ID         BIT STRING (SIZE(20)),
 *      homeENB-ID          BIT STRING (SIZE(28)),
 *      ...
 * }
 * */
typedef struct ENB_ID_c{
    void                (*freeIE)(void*);
    void                (*showIE)(void*);
    uint8_t             ext;     /*< Extension flag*/
    uint8_t             choice;     /*< Extension flag*/
    union{
        uint32_t macroENB_ID;    /*< Equal to the 20 leftmost bits of the Cell Identity IE contained in the E-UTRAN CGI IE (see 3GPP 36.413 subclause 9.2.1.38) of each cell served by the eNB. */
        uint32_t homeENB_ID;    /*< Equal to the Cell Identity IE contained in the E-UTRAN CGI IE (see 3GPP 36.413 subclause 9.2.1.38) of the cell served by the eNB. */
    }id;
    /*struct ENB_ID_c *   (*cloneIE)(struct ENB_ID_c *);*/
}ENB_ID_t;

/** @brief Constructor of ENB_ID type
 *  @return ENB_ID_t allocated  and initialized structure
 * */
ENB_ID_t *new_ENB_ID();


/**@brief Global ENB-ID
 *
 * ASN.1
 * Global-ENB-ID ::= SEQUENCE {
 *      pLMNidentity          PLMNidentity,
 *      eNB-ID                ENB-ID,
 *      iE-Extensions         ProtocolExtensionContainer { {GlobalENB-ID-ExtIEs} } OPTIONAL,
 *      ...
 * }
 * */
typedef struct Global_ENB_ID_c{
    void                            (*freeIE)(void *);
    void                            (*showIE)(void *);
    uint8_t                         ext;     /*< Extension flag*/
    uint8_t                         opt;     /*< Optional flag, is Extension Container present?*/
    PLMNidentity_t                  *pLMNidentity;
    ENB_ID_t                        *eNBid;
    ProtocolExtensionContainer_t    *iEext;
    /*struct Global_ENB_ID_c *        (*cloneIE)(struct Global_ENB_ID_c *);*/
}Global_ENB_ID_t;

/** @brief Constructor of Global_ENB_ID type
 *  @return Global_ENB_ID_t allocated  and initialized structure
 * */
Global_ENB_ID_t *new_Global_ENB_ID();


/**@brief ENBname
 *
 * ASN.1
 * ENBname ::= PrintableString (SIZE (1..150,...))
 * */
typedef struct ENBname_c{
    void        (*freeIE)(void *);
    void        (*showIE)(void *);
    uint8_t     ext;     /*< Extension flag*/
    uint8_t     name[150];
    uint8_t     *extension;
}ENBname_t;

/** @brief Constructor of ENBname_ID type
 *  @return ENBname_t allocated  and initialized structure
 * */
ENBname_t *new_ENBname();


/**@brief MMEname
 *
 * ASN.1
 * MMEname ::= PrintableString (SIZE (1..150,...))
 * */
typedef struct MMEname_c{
    void        (*freeIE)(void *);
    void        (*showIE)(void *);
    uint8_t     ext;     /*< Extension flag*/
    uint8_t     name[150];
    uint8_t     *extension;
}MMEname_t;

/** @brief Constructor of ENBname_ID type
 *  @return ENBname_t allocated  and initialized structure
 * */
MMEname_t *new_MMEname();


/**@brief BPLMNs
 *
 * ASN.1
 * BPLMNs ::= SEQUENCE (SIZE(1.. maxnoofBPLMNs)) OF PLMNidentity
 *
 * */
typedef struct BPLMNs_c{
    void                    (*freeIE)(void *);
    void                    (*showIE)(void *);
    void                    (*addPLMNid)(struct BPLMNs_c*, PLMNidentity_t* ie);
    uint8_t                 size;
    uint8_t                 n;
    PLMNidentity_t          **pLMNidentity;

}BPLMNs_t;

/** @brief Constructor of BPLMNs type
 *  @return BPLMNs_t allocated  and initialized structure
 * */
BPLMNs_t *new_BPLMNs();


/**@brief SupportedTAs-item
 *
 * ASN.1
 * SupportedTAs-Item ::=   SEQUENCE  {
 *     tAC                 TAC,
 *     broadcastPLMNs      BPLMNs,
 *     iE-Extensions       ProtocolExtensionContainer { {SupportedTAs-Item-ExtIEs} } OPTIONAL,
 *     ...
 * }
 *
 * SupportedTAs-Item-ExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     ...
 * }
 */
typedef struct SupportedTAs_Item_c{
    void                            (*freeItem)(void *);
    void                            (*showItem)(void *);
    uint8_t                         ext;
    uint8_t                         opt;
    TAC_t                           *tAC;
    BPLMNs_t                        *broadcastPLMNs;
    ProtocolExtensionContainer_t    *iEext;
}SupportedTAs_Item_t;

/** @brief Constructor of SupportedTAs-item type
 *  @return SupportedTAs_item_t allocated  and initialized structure
 * */
SupportedTAs_Item_t *new_SupportedTAs_Item();


/**@brief SupportedTAs
 *
 * ASN.1
 * SupportedTAs ::= SEQUENCE (SIZE(1.. maxnoofTACs)) OF SupportedTAs-Item
 *
 * */
typedef struct SupportedTAs_c{
    void                    (*freeIE)(void *);
    void                    (*showIE)(void *);
    void                    (*additem)(struct SupportedTAs_c*, SupportedTAs_Item_t* ie);
    uint8_t                 size;               /*<Number of items*/
    SupportedTAs_Item_t     **item;             /*<Item Array */
}SupportedTAs_t;

/** @brief Constructor of SupportedTAs type
 *  @return SupportedTAs_t allocated  and initialized structure
 * */
SupportedTAs_t *new_SupportedTAs();

/**@brief CSG-Id
 * 3GPP 36.413 clause 9.2.1.62
 * ASN.1
 * CSG-Id      ::= BIT STRING (SIZE (27))
 */
typedef struct cSG_id_c{
    void                            (*freeIE)(void *);
    void                            (*showIE)(void *);
    uint32_t                        id : 27;
}cSG_id_t;

/** @brief Constructor of CSG-Id type
 *  @return cSG_id_t allocated  and initialized structure
 * */
cSG_id_t *new_CSG_id();

/**@brief CSG-IdList-Item
 * 3GPP 36.413 clause 9.2.1.62
 * ASN.1
 * CSG-Id      ::= BIT STRING (SIZE (27))
 *
 * CSG-IdList-Item ::= SEQUENCE {
 *     cSG-Id          CSG-Id,
 *     iE-Extensions   ProtocolExtensionContainer { {CSG-IdList-Item-ExtIEs} } OPTIONAL,
 *     ...
 * }
 * CSG-IdList-Item-ExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     ...
 * }
 */
typedef struct CSG_IdList_Item_c{
    void                            (*freeItem)(void *);
    void                            (*showItem)(void *);
    uint8_t                         ext;
    uint8_t                         opt;
    cSG_id_t                        *cSG_id;
    ProtocolExtensionContainer_t    *iEext;
}CSG_IdList_Item_t;

/** @brief Constructor of CSG-IdList-Item type
 *  @return CSG_IdList_Item_t allocated  and initialized structure
 * */
CSG_IdList_Item_t *new_CSG_IdList_Item();


/**@brief CSG-IdList
 *
 * ASN.1
 * CSG-IdList ::= SEQUENCE (SIZE (1..maxNrOfCSGs)) OF CSG-IdList-Item
 * */
typedef struct CSG_IdList_c{
    void                    (*freeIE)(void *);
    void                    (*showIE)(void *);
    void                    (*additem)(struct CSG_IdList_c*, CSG_IdList_Item_t* ie);
    uint8_t                 size;               /*<Number of items*/
    CSG_IdList_Item_t       **item;             /*<Item Array */

}CSG_IdList_t;

/** @brief Constructor of CSG-IdList type
 *  @return CSG_IdList_t allocated  and initialized structure
 * */
CSG_IdList_t *new_CSG_IdList();


/**@brief MME-Group-ID
 *
 * ASN.1
 * MME-Group-ID    ::= OCTET STRING (SIZE (2))
 */
typedef struct MME_Group_ID_c {
    void    (*freeIE)(void *);
    void    (*showIE)(void *);
    uint8_t s[2];
}MME_Group_ID_t;

/** @brief Constructor of MME-Group-ID type
 *  @return MME_Group_ID_t allocated  and initialized structure
 * */
MME_Group_ID_t *new_MME_Group_ID();


/**@brief MME-Code
 * clause 9.2.3.12
 * ASN.1
 * MME-Code        ::= OCTET STRING (SIZE (1))
 */
typedef struct MME_Code_c {
    void    (*freeIE)(void *);
    void    (*showIE)(void *);
    uint8_t s[1];
}MME_Code_t;

/** @brief Constructor of MME-Code type
 *  @return MME_Code_t allocated  and initialized structure
 * */
MME_Code_t *new_MME_Code();


/**@brief ServedGroupIDs
 *
 * ASN.1
 * ServedGroupIDs ::= SEQUENCE (SIZE(1.. maxnoofGroupIDs)) OF MME-Group-ID
 * */
typedef struct ServedGroupIDs_c{
    void                    (*freeIE)(void *);
    void                    (*showIE)(void *);
    void                    (*additem)(struct ServedGroupIDs_c*, MME_Group_ID_t* ie);
    uint16_t                size;               /*<Number of items*/
    MME_Group_ID_t          **item;             /*<Item Array */
}ServedGroupIDs_t;

/** @brief Constructor of ServedGroupIDs type
 *  @return ServedGroupIDs_t allocated  and initialized structure
 * */
ServedGroupIDs_t *new_ServedGroupIDs();


/**@brief ServedMMECs
 *
 * ASN.1
 * ServedMMECs ::= SEQUENCE (SIZE(1.. maxnoofMMECs)) OF MME-Code
 * */
typedef struct ServedMMECs_c{
    void                    (*freeIE)(void *);
    void                    (*showIE)(void *);
    void                    (*additem)(struct ServedMMECs_c*, MME_Code_t* ie);
    uint8_t                 size;               /*<Number of items*/
    MME_Code_t              **item;             /*<Item Array */
}ServedMMECs_t;

/** @brief Constructor of ServedMMECs type
 *  @return ServedMMECs_t allocated  and initialized structure
 * */
ServedMMECs_t *new_ServedMMECs();


/**@brief ServedPLMNs
 *
 * ASN.1
 * ServedPLMNs ::= SEQUENCE (SIZE(1.. maxnoofPLMNsPerMME)) OF PLMNidentity
 * */
typedef struct ServedPLMNs_c{
    void                    (*freeIE)(void *);
    void                    (*showIE)(void *);
    void                    (*additem)(struct ServedPLMNs_c*, PLMNidentity_t* ie);
    uint8_t                 size;               /*<Number of items*/
    PLMNidentity_t          **item;             /*<Item Array */
}ServedPLMNs_t;

/** @brief Constructor of ServedPLMNs type
 *  @return ServedPLMNs_t allocated  and initialized structure
 * */
ServedPLMNs_t *new_ServedPLMNs();


/**@brief ServedGUMMEIsItem
 *
 * ASN.1
 * ServedGUMMEIsItem ::= SEQUENCE {
 *     servedPLMNs             ServedPLMNs,
 *     servedGroupIDs          ServedGroupIDs,
 *     servedMMECs             ServedMMECs,
 *     iE-Extensions           ProtocolExtensionContainer { {ServedGUMMEIsItem-ExtIEs} }   OPTIONAL,
 *     ...
 * }
 * ServedGUMMEIsItem-ExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     ...
 * }
 * */
typedef struct ServedGUMMEIsItem_c{
    void                            (*freeIE)(void *);
    void                            (*showIE)(void *);
    uint8_t                         ext;     /*< Extension flag*/
    uint8_t                         opt;     /*< Optional flag, is Extension Container present?*/
    ServedPLMNs_t                   *servedPLMNs;
    ServedGroupIDs_t                *servedGroupIDs;
    ServedMMECs_t                   *servedMMECs;
    ProtocolExtensionContainer_t    *iEext;
}ServedGUMMEIsItem_t;

/** @brief Constructor of ServedGUMMEIsItem type
 *  @return ServedGUMMEIsItem_t allocated  and initialized structure
 * */
ServedGUMMEIsItem_t *new_ServedGUMMEIsItem();


/**@brief ServedGUMMEIs
 *
 * ASN.1
 * ServedGUMMEIs ::= SEQUENCE (SIZE (1.. maxnoofRATs)) OF ServedGUMMEIsItem
 * */
typedef struct ServedGUMMEIs_c{
    void                    (*freeIE)(void *);
    void                    (*showIE)(void *);
    uint8_t                 size;               /*<Number of items*/
    ServedGUMMEIsItem_t     **item;             /*<Item Array */
    void                    (*additem)(struct ServedGUMMEIs_c*, ServedGUMMEIsItem_t* ie);
}ServedGUMMEIs_t;

/** @brief Constructor of ServedGUMMEIs type
 *  @return ServedGUMMEIs_t allocated  and initialized structure
 * */
ServedGUMMEIs_t *new_ServedGUMMEIs();


/**@brief RelativeMMECapacity
 *
 * ASN.1
 * RelativeMMECapacity             ::= INTEGER (0..255)
 * */
typedef struct RelativeMMECapacity_c{
    void    (*freeIE)(void *);
    void    (*showIE)(void *);
    uint8_t cap;
}RelativeMMECapacity_t;

/** @brief Constructor of RelativeMMECapacity type
 *  @return RelativeMMECapacity_t allocated  and initialized structure
 * */
RelativeMMECapacity_t *new_RelativeMMECapacity();


/**@brief MMERelaySupportIndicator
 *
 * ASN.1
 * MMERelaySupportIndicator ::= ENUMERATED {true, ...}
 * */
typedef struct MMERelaySupportIndicator_c{
    void    (*freeIE)(void *);
    void    (*showIE)(void *);
    uint8_t ext;
    enum{
        MMERelaySupportIndicator_true,
    }ind;
}MMERelaySupportIndicator_t;

/** @brief Constructor of MMERelaySupportIndicator type
 *  @return MMERelaySupportIndicator_t allocated  and initialized structure
 * */
MMERelaySupportIndicator_t *new_MMERelaySupportIndicator();


/**@brief CriticalityDiagnostics-IE-Item
 *
 * ASN.1
 * CriticalityDiagnostics-IE-Item ::=  SEQUENCE {
 *     iECriticality           Criticality,
 *     iE-ID                   ProtocolIE-ID,
 *     typeOfError             TypeOfError,
 *     iE-Extensions           ProtocolExtensionContainer {{CriticalityDiagnostics-IE-Item-ExtIEs}} OPTIONAL,
 *     ...
 * }
 *
 * CriticalityDiagnostics-IE-Item-ExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     ...
 * }
 * */
typedef struct CriticalityDiagnostics_IE_Item_c{
    void                            (*freeIE)(void *);
    void                            (*showIE)(void *);
    uint8_t                         ext;     /*< Extension flag*/
    uint8_t                         opt;     /*< Optional flags, is Extension Container present?*/
    Criticality_e                   iECriticality;
    ProtocolIE_ID_t                 iE_ID;
    TypeOfError_e                   typeOfError;
    ProtocolExtensionContainer_t    *iEext;
}CriticalityDiagnostics_IE_Item_t;

/** @brief Constructor of CriticalityDiagnostics-IE-Item type
 *  @return CriticalityDiagnostics_IE_Item_t allocated  and initialized structure
 * */
CriticalityDiagnostics_IE_Item_t *new_CriticalityDiagnostics_IE_Item();


/**@brief CriticalityDiagnostics-IE-List
 *
 * ASN.1
 * CriticalityDiagnostics-IE-List ::= SEQUENCE (SIZE (1..maxNrOfErrors)) OF CriticalityDiagnostics-IE-Item
 * */
typedef struct CriticalityDiagnostics_IE_List_c{
    void                                (*freeIE)(void *);
    void                                (*showIE)(void *);
    void                                (*additem)(struct CriticalityDiagnostics_IE_List_c*, CriticalityDiagnostics_IE_Item_t* ie);
    uint8_t                             size;               /*<Number of items*/
    CriticalityDiagnostics_IE_Item_t    **item;             /*<Item Array */
}CriticalityDiagnostics_IE_List_t;

/** @brief Constructor of CriticalityDiagnostics-IE-List type
 *  @return CriticalityDiagnostics_IE_List_t allocated  and initialized structure
 * */
CriticalityDiagnostics_IE_List_t *new_CriticalityDiagnostics_IE_List();


/**@brief Criticality Diagnostics
 *
 * ASN.1
 * CriticalityDiagnostics ::= SEQUENCE {
 *      procedureCode                   ProcedureCode                   OPTIONAL,
 *      triggeringMessage               TriggeringMessage               OPTIONAL,
 *      procedureCriticality            Criticality                     OPTIONAL,
 *      iEsCriticalityDiagnostics       CriticalityDiagnostics-IE-List  OPTIONAL,
 *      iE-Extensions                   ProtocolExtensionContainer {{CriticalityDiagnostics-ExtIEs}} OPTIONAL,
 *      ...
 * }
 *
 * CriticalityDiagnostics-ExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     ...
 * }
 * */
typedef struct CriticalityDiagnostics_c{
    void                                (*freeIE)(void *);
    void                                (*showIE)(void *);
    uint8_t                             ext;     /*< Extension flag*/
    uint8_t                             opt;     /*< Optional flags, is Extension Container present?*/
    ProcedureCode_t                     procedureCode;
    TriggeringMessage_e                 triggeringMessage;
    Criticality_e                       procedureCriticality;
    CriticalityDiagnostics_IE_List_t    *iEsCriticalityDiagnostics;
    ProtocolExtensionContainer_t        *iEext;
}CriticalityDiagnostics_t;

/** @brief Constructor of CriticalityDiagnostics type
 *  @return CriticalityDiagnostics_t allocated  and initialized structure
 * */
CriticalityDiagnostics_t *new_CriticalityDiagnostics();


/**@brief CauseRadioNetwork
 *
 * ASN.1
 * CauseRadioNetwork ::= ENUMERATED {
 *     unspecified,
 *     tx2relocoverall-expiry,
 *     successful-handover,
 *     release-due-to-eutran-generated-reason,
 *     handover-cancelled,
 *     partial-handover,
 *     ho-failure-in-target-EPC-eNB-or-target-system,
 *     ho-target-not-allowed,
 *     tS1relocoverall-expiry,
 *     tS1relocprep-expiry,
 *     cell-not-available,
 *     unknown-targetID,
 *     no-radio-resources-available-in-target-cell,
 *     unknown-mme-ue-s1ap-id,
 *     unknown-enb-ue-s1ap-id,
 *     unknown-pair-ue-s1ap-id,
 *     handover-desirable-for-radio-reason,
 *     time-critical-handover,
 *     resource-optimisation-handover,
 *     reduce-load-in-serving-cell,
 *     user-inactivity,
 *     radio-connection-with-ue-lost,
 *     load-balancing-tau-required,
 *     cs-fallback-triggered,
 *     ue-not-available-for-ps-service,
 *     radio-resources-not-available,
 *     failure-in-radio-interface-procedure,
 *     invalid-qos-combination,
 *     interrat-redirection,
 *     interaction-with-other-procedure,
 *     unknown-E-RAB-ID,
 *     multiple-E-RAB-ID-instances,
 *     encryption-and-or-integrity-protection-algorithms-not-supported,
 *     s1-intra-system-handover-triggered,
 *     s1-inter-system-handover-triggered,
 *     x2-handover-triggered,
 *     ...,
 *     redirection-towards-1xRTT,
 *     not-supported-QCI-value,
 *     invalid-CSG-Id
 * }
 */
typedef struct CauseRadioNetwork_c{
    uint8_t ext;
    union{
        enum{
            CauseRadioNetwork_unspecified,
            CauseRadioNetwork_tx2relocoverall_expiry,
            CauseRadioNetwork_successful_handover,
            CauseRadioNetwork_release_due_to_eutran_generated_reason,
            CauseRadioNetwork_handover_cancelled,
            CauseRadioNetwork_partial_handover,
            CauseRadioNetwork_ho_failure_in_target_EPC_eNB_or_target_system,
            CauseRadioNetwork_ho_target_not_allowed,
            CauseRadioNetwork_tS1relocoverall_expiry,
            CauseRadioNetwork_tS1relocprep_expiry,
            CauseRadioNetwork_cell_not_available,
            CauseRadioNetwork_unknown_targetID,
            CauseRadioNetwork_no_radio_resources_available_in_target_cell,
            CauseRadioNetwork_unknown_mme_ue_s1ap_id,
            CauseRadioNetwork_unknown_enb_ue_s1ap_id,
            CauseRadioNetwork_unknown_pair_ue_s1ap_id,
            CauseRadioNetwork_handover_desirable_for_radio_reason,
            CauseRadioNetwork_time_critical_handover,
            CauseRadioNetwork_resource_optimisation_handover,
            CauseRadioNetwork_reduce_load_in_serving_cell,
            CauseRadioNetwork_user_inactivity,
            CauseRadioNetwork_radio_connection_with_ue_lost,
            CauseRadioNetwork_load_balancing_tau_required,
            CauseRadioNetwork_cs_fallback_triggered,
            CauseRadioNetwork_ue_not_available_for_ps_service,
            CauseRadioNetwork_radio_resources_not_available,
            CauseRadioNetwork_failure_in_radio_interface_procedure,
            CauseRadioNetwork_invalid_qos_combination,
            CauseRadioNetwork_interrat_redirection,
            CauseRadioNetwork_interaction_with_other_procedure,
            CauseRadioNetwork_unknown_E_RAB_ID,
            CauseRadioNetwork_multiple_E_RAB_ID_instances,
            CauseRadioNetwork_encryption_and_or_integrity_protection_algorithms_not_supported,
            CauseRadioNetwork_s1_intra_system_handover_triggered,
            CauseRadioNetwork_s1_inter_system_handover_triggered,
            CauseRadioNetwork_x2_handover_triggered,
        }noext;
        enum{
            CauseRadioNetwork_redirection_towards_1xRTT,
            CauseRadioNetwork_not_supported_QCI_value,
            CauseRadioNetwork_invalid_CSG_Id
        }ext;
    }cause;
}CauseRadioNetwork_e;

/**@brief CauseTransport
 *
 * ASN.1
 * CauseTransport ::= ENUMERATED {
 *     transport-resource-unavailable,
 *     unspecified,
 *     ...
 * }
 */
typedef struct CauseTransport_c{
    uint8_t ext;
    union{
        enum{
            CauseTransport_transport_resource_unavailable,
            CauseTransport_unspecified,
        }noext;
        enum{
            CauseTransportnoext,
        }ext;
    }cause;
}CauseTransport_e;

/**@brief CauseNAS
 *
 * ASN.1
 * CauseNas ::= ENUMERATED {
 *     normal-release,
 *     authentication-failure,
 *     detach,
 *     unspecified,
 *     ...,
 *     csg-subscription-expiry
 * }
 */
typedef struct CauseNas_c{
    uint8_t ext;
    union{
        enum{
            CauseNas_normal_release,
            CauseNas_authentication_failure,
            CauseNas_detach,
            CauseNas_unspecified,
        }noext;
        enum{
            CauseNas_csg_subscription_expiry
        }ext;
    }cause;
}CauseNas_e;

/**@brief CauseProtocol
 *
 * ASN.1
 * CauseProtocol ::= ENUMERATED {
 *     transfer-syntax-error,
 *     abstract-syntax-error-reject,
 *     abstract-syntax-error-ignore-and-notify,
 *     message-not-compatible-with-receiver-state,
 *     semantic-error,
 *     abstract-syntax-error-falsely-constructed-message,
 *     unspecified,
 *     ...
 * }
 */
typedef struct CauseProtocol_c{
    uint8_t ext;
    union{
        enum{
            CauseProtocol_transfer_syntax_error,
            CauseProtocol_abstract_syntax_error_reject,
            CauseProtocol_abstract_syntax_error_ignore_and_notify,
            CauseProtocol_message_not_compatible_with_receiver_state,
            CauseProtocol_semantic_error,
            CauseProtocol_abstract_syntax_error_falsely_constructed_message,
            CauseProtocol_unspecified,
        }noext;
        enum{
            noext,
        }ext;
    }cause;
}CauseProtocol_e;

/**@brief CauseMisc
 *
 * ASN.1
 * CauseMisc ::= ENUMERATED {
 *     control-processing-overload,
 *     not-enough-user-plane-processing-resources,
 *     hardware-failure,
 *     om-intervention,
 *     unspecified,
 *     unknown-PLMN,
 *     ...
 * }
 */
typedef struct CauseMisc_c{
    uint8_t ext;
    union{
        enum{
            CauseMisc_control_processing_overload,
            CauseMisc_not_enough_user_plane_processing_resources,
            CauseMisc_hardware_failure,
            CauseMisc_om_intervention,
            CauseMisc_unspecified,
            CauseMisc_unknown_PLMN,
        }noext;
        enum{
            misc_noext
        }ext;
    }cause;
}CauseMisc_e;

typedef enum cause_choice_c{
    CauseRadioNetwork,
    CauseTransport,
    CauseNas,
    CauseProtocol,
    CauseMisc
}cause_choice_t;

/**@brief Cause
 *
 * ASN.1
 * Cause ::= CHOICE {
 *     radioNetwork        CauseRadioNetwork,
 *     transport           CauseTransport,
 *     nas                 CauseNas,
 *     protocol            CauseProtocol,
 *     misc                CauseMisc,
 *     ...
 * }
 * */
typedef struct Cause_c{
    void                                (*freeIE)(void *);
    void                                (*showIE)(void *);
    uint8_t                             ext;    /*< Extension flag*/
    uint8_t                             choice; /*< Optional flags, is Extension Container present?*/
    union{
        CauseRadioNetwork_e radioNetwork;
        CauseTransport_e    transport;
        CauseNas_e          nas;
        CauseProtocol_e     protocol;
        CauseMisc_e         misc;
    }cause;
}Cause_t;

/** @brief Constructor of Cause type
 *  @return Cause_t allocated  and initialized structure
 * */
Cause_t *new_Cause();


/**@brief TimeToWait
 *
 * ASN.1
 * TimeToWait ::= ENUMERATED {v1s, v2s, v5s, v10s, v20s, v60s, ...}
 * */
typedef struct TimeToWait_c{
    void            (*freeIE)(void *);
    void            (*showIE)(void *);
    uint8_t         ext;    /*< Extension flag*/
    union{
        enum{
            v1s,
            v2s,
            v5s,
            v10s,
            v20s,
            v60s
        }noext;
        enum{
            time_noext
        }ext;
    }time;
}TimeToWait_t;

/** @brief Constructor of Cause type
 *  @return Cause_t allocated  and initialized structure
 * */
TimeToWait_t *new_TimeToWait();

/**@brief ENB-UE-S1AP-ID
 *
 * range (0 .. 2^24-1) -> 24 bits
 * ASN.1:
 * ENB-UE-S1AP-ID              ::= INTEGER (0..16777215)
 * */
typedef struct ENB_UE_S1AP_ID_c{
    void        (*freeIE)(void*);
    void        (*showIE)(void*);
    uint32_t    eNB_id   :   24;
}ENB_UE_S1AP_ID_t;

/** @brief Constructor of ENB_UE_S1AP_ID type
 *  @return ENB_UE_S1AP_ID_t allocated  and initialized structure
 * */
ENB_UE_S1AP_ID_t *new_ENB_UE_S1AP_ID();


/**@brief MME-UE-S1AP-ID
 *
 * range (0 .. 2^32-1) -> 32 bits
 * ASN.1:
 * MME-UE-S1AP-ID  ::= INTEGER (0..4294967295)
 * */
typedef struct MME_UE_S1AP_ID_c{
    void        (*freeIE)(void*);
    void        (*showIE)(void*);
    uint32_t    mme_id;
}MME_UE_S1AP_ID_t;

/** @brief Constructor of MME_UE_S1AP_ID type
 *  @return MME_UE_S1AP_ID_t allocated  and initialized structure
 * */
MME_UE_S1AP_ID_t *new_MME_UE_S1AP_ID();


/**@brief Unconstrained_Octed_String
 * This type is used as unconstrained octet string container for this IE:
 *
 *  - NAS-PDU
 *  - Source-ToTarget-TransparentContainer
 *      This IE includes a transparent container from the source RAN node to the target RAN node.
 *      The octets of the OCTET STRING are encoded according to the specifications of the target system.
 *  - Target-ToSource-TransparentContainer
 *      This IE includes a transparent container from the target RAN node to the source RAN node.
 *      The octets of the OCTET STRING are coded according to the specifications of the target system.
 *  - NASSecurityParametersfromE-UTRAN
 *
 * ASN.1:
 * <type> ::= OCTET STRING
 * */
typedef struct Unconstrained_Octed_String_c{
    void        (*freeIE)(void*);
    void        (*showIE)(void*);
    uint32_t    len;
    uint8_t     *str;
}Unconstrained_Octed_String_t;

/** @brief Constructor of Unconstrained_Octed_String type
 *  @return Unconstrained_Octed_String_t allocated  and initialized structure
 * */
Unconstrained_Octed_String_t *new_Unconstrained_Octed_String();


/**@brief NAS-PDU
 *
 * ASN.1:
 * NAS-PDU ::=  OCTET STRING
 * *//*
typedef struct NAS_PDU_c{
    uint32_t    len;
    uint8_t     *str;
    void        (*freeIE)(void*);
    void        (*showIE)(void*);
}NAS_PDU_t;*/

/** @brief Constructor of NAS_PDU type
 *  @return NAS_PDU_t allocated  and initialized structure
 * *//*
NAS_PDU_t *new_NAS_PDU();*/


/**@brief TAI
 *
 * ASN.1:
 * TAI ::= SEQUENCE {
 *     pLMNidentity            PLMNidentity,
 *     tAC                     TAC,
 *     iE-Extensions           ProtocolExtensionContainer { {TAI-ExtIEs} } OPTIONAL,
 *     ...
 * }
 *
 * TAI-ExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     ...
 * }
 * */
typedef struct TAI_c{
    void                            (*freeIE)(void*);
    void                            (*showIE)(void*);
    uint8_t                         ext;
    uint8_t                         opt;
    PLMNidentity_t                  *pLMNidentity;
    TAC_t                           *tAC;
    ProtocolExtensionContainer_t    *iEext;
}TAI_t;

/** @brief Constructor of NAS_PDU type
 *  @return TAI_t allocated  and initialized structure
 * */
TAI_t *new_TAI();

/**@brief CellIdentity
 * ASN.1
 * CellIdentity            ::= BIT STRING (SIZE (28))
 */
typedef struct CellIdentity_c{
    uint32_t id:28;
}CellIdentity_t;

/**@brief EUTRAN_CGI
 *
 * ASN.1:
 * EUTRAN-CGI ::= SEQUENCE {
 *     pLMNidentity            PLMNidentity,
 *     cell-ID                 CellIdentity,
 *     iE-Extensions           ProtocolExtensionContainer { {EUTRAN-CGI-ExtIEs} } OPTIONAL,
 *     ...
 * }
 * EUTRAN-CGI-ExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     ...
 * }
 * */
typedef struct EUTRAN_CGI_c{
    void                            (*freeIE)(void*);
    void                            (*showIE)(void*);
    uint8_t                         ext;
    uint8_t                         opt;
    PLMNidentity_t                  *pLMNidentity;
    CellIdentity_t                  cell_ID;
    ProtocolExtensionContainer_t    *iEext;
}EUTRAN_CGI_t;

/** @brief Constructor of EUTRAN_CGI type
 *  @return EUTRAN_CGI_t allocated  and initialized structure
 * */
EUTRAN_CGI_t *new_EUTRAN_CGI();

/**@brief RRC_Establishment_Cause
 *
 *establishmentCause:The main cause values and the corresponding NAS procedure whichtriggers the RRC connection establishment are presented below:
 * emergency: This corresponds to NAS Procedure “MO-CS fallback Emergency call”
 * mt-Access: Corresponding NAS procedures are“Service Request” (paging response for PS domain) or “Extended Service Request”(MT-CS fallback)
 * mo-Signalling: Corresponding NAS procedures are Attach, Detach, and TAU
 * mo-Data: Corresponding NAS Procedures are“Service Request” and “Extended Service Request”
 *
 * ASN.1
 * RRC-Establishment-Cause ::= ENUMERATED {
 *     emergency,
 *     highPriorityAccess,
 *     mt-Access,
 *     mo-Signalling,
 *     mo-Data,
 *     ...,
 *     delay-TolerantAccess
 * }
 * */
typedef struct RRC_Establishment_Cause_c{
    void                                (*freeIE)(void *);
    void                                (*showIE)(void *);
    uint8_t                             ext;    /*< Extension flag*/
    union{
        enum{
            RRC_emergency,
            RRC_highPriorityAccess,
            RRC_mt_Access,
            RRC_mo_Signalling,
            RRC_mo_Data
        }noext;
        enum{
            RRC_delay_TolerantAccess,
        }ext;
    }cause;
}RRC_Establishment_Cause_t;

/** @brief Constructor of Cause type
 *  @return Cause_t allocated  and initialized structure
 * */
RRC_Establishment_Cause_t *new_RRC_Establishment_Cause();

extern const char *RRC_Establishment_Cause_Name[];

/**@brief BitRate
 * ASN.1
 * BitRate ::= INTEGER (0..10000000000)
 */
typedef struct BitRate_c{
    uint64_t rate;
}BitRate_t;


/**@brief UEAggregateMaximumBitrate
 *
 * ASN.1
 * UEAggregateMaximumBitrate ::= SEQUENCE {
 *     uEaggregateMaximumBitRateDL             BitRate,
 *     uEaggregateMaximumBitRateUL             BitRate,
 *     iE-Extensions                           ProtocolExtensionContainer { {UEAggregate-MaximumBitrates-ExtIEs} } OPTIONAL,
 *    ...
 * }
 *
 * UEAggregate-MaximumBitrates-ExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *    ...
 * }
 * */
typedef struct UEAggregateMaximumBitrate_c{
    void                            (*freeIE)(void *);
    void                            (*showIE)(void *);
    uint8_t                         ext;    /*< Extension flag*/
    uint8_t                         opt;
    BitRate_t                       uEaggregateMaximumBitRateDL;
    BitRate_t                       uEaggregateMaximumBitRateUL;
    ProtocolExtensionContainer_t    *iEext;
}UEAggregateMaximumBitrate_t;

/** @brief Constructor of Cause type
 *  @return UEAggregateMaximumBitrate_t allocated  and initialized structure
 * */
UEAggregateMaximumBitrate_t *new_UEAggregateMaximumBitrate();


/* **************** Work in Progress ********************* */

/**@brief UEAggregateMaximumBitrate
 * ASN.1
 * E-RAB-ID        ::= INTEGER (0..15, ...)
 * */
typedef struct E_RAB_ID_c{
    uint8_t     ext;    /*< Extension flag*/
    uint32_t    id;
}E_RAB_ID_t;


/**@brief QCI
 * ASN.1
 * QCI                     ::= INTEGER (0..255)
 * */
typedef uint8_t QCI_t;


/**@brief PriorityLevel
 * ASN.1
 * PriorityLevel               ::= INTEGER { spare (0), highest (1), lowest (14), no-priority (15) } (0..15)
 */
typedef uint8_t PriorityLevel_t;


/**@brief Pre-emptionCapability
 * ASN.1
 * Pre-emptionCapability ::= ENUMERATED {
 *     shall-not-trigger-pre-emption,
 *     may-trigger-pre-emption
 * }
 */
typedef enum Pre_emptionCapability_c{
    shall_not_trigger_pre_emption,
    may_trigger_pre_emption
}Pre_emptionCapability_e;

extern const char *Pre_emptionCapabilityName[];


/**@brief Pre-emptionVulnerability
 * ASN.1
 * Pre-emptionVulnerability ::= ENUMERATED {
 *     not-pre-emptable,
 *     pre-emptable
 * }
 */
typedef enum Pre_emptionVulnerability_c{
    not_pre_emptable,
    pre_emptable
}Pre_emptionVulnerability_e;

extern const char *Pre_emptionVulnerabilityName[];


/**@brief AllocationAndRetentionPriority
 *
 * ASN.1
 * AllocationAndRetentionPriority ::= SEQUENCE {
 *     priorityLevel               PriorityLevel,
 *     pre-emptionCapability       Pre-emptionCapability,
 *     pre-emptionVulnerability    Pre-emptionVulnerability,
 *     iE-Extensions               ProtocolExtensionContainer { {AllocationAndRetentionPriority-ExtIEs} } OPTIONAL,
 *     ...
 * }
 *
 * AllocationAndRetentionPriority-ExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     ...
 * }
 * */
typedef struct AllocationAndRetentionPriority_c{
    void                            (*freeIE)(void *);
    void                            (*showIE)(void *);
    uint8_t                         ext;    /*< Extension flag*/
    uint8_t                         opt;
    PriorityLevel_t                 priorityLevel;
    Pre_emptionCapability_e         pre_emptionCapability;
    Pre_emptionVulnerability_e      pre_emptionVulnerability;
    ProtocolExtensionContainer_t    *iEext;
}AllocationAndRetentionPriority_t;

/** @brief Constructor of AllocationAndRetentionPriority type
 *  @return AllocationAndRetentionPriority_t allocated  and initialized structure
 * */
AllocationAndRetentionPriority_t *new_AllocationAndRetentionPriority();


/**@brief GBR-QosInformation
 *
 * ASN.1
 * GBR-QosInformation ::= SEQUENCE {
 *     e-RAB-MaximumBitrateDL          BitRate,
 *     e-RAB-MaximumBitrateUL          BitRate,
 *     e-RAB-GuaranteedBitrateDL       BitRate,
 *     e-RAB-GuaranteedBitrateUL       BitRate,
 *     iE-Extensions                   ProtocolExtensionContainer { { GBR-QosInformation-ExtIEs} } OPTIONAL,
 *     ...
 * }
 *
 * GBR-QosInformation-ExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     ...
 * }
 * */
typedef struct GBR_QosInformation_c{
    void                            (*freeIE)(void *);
    void                            (*showIE)(void *);
    uint8_t                         ext;    /*< Extension flag*/
    uint8_t                         opt;
    BitRate_t                       eRAB_MaximumBitrateDL;
    BitRate_t                       eRAB_MaximumBitrateUL;
    BitRate_t                       eRAB_GuaranteedBitrateDL;
    BitRate_t                       eRAB_GuaranteedBitrateUL;
    ProtocolExtensionContainer_t    *iEext;
}GBR_QosInformation_t;

/** @brief Constructor of GBR-QosInformation type
 *  @return GBR_QosInformation_t allocated  and initialized structure
 * */
GBR_QosInformation_t *new_GBR_QosInformation();


/**@brief E-RABLevelQoSParameters
 *
 * ASN.1
 * E-RABLevelQoSParameters ::= SEQUENCE {
 *     qCI         QCI,
 *     allocationRetentionPriority         AllocationAndRetentionPriority,
 *     gbrQosInformation                   GBR-QosInformation          OPTIONAL,
 *     iE-Extensions                       ProtocolExtensionContainer { {E-RABQoSParameters-ExtIEs} } OPTIONAL,
 *     ...
 * }
 *
 * E-RABQoSParameters-ExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     ...
 * }
 * */
typedef struct E_RABLevelQoSParameters_c{
    void                                (*freeIE)(void *);
    void                                (*showIE)(void *);
    uint8_t                             ext;    /*< Extension flag*/
    uint8_t                             opt;
    QCI_t                               qCI;
    AllocationAndRetentionPriority_t    *allocationRetentionPriority;
    GBR_QosInformation_t                *gbrQosInformation;
    ProtocolExtensionContainer_t        *iEext;
}E_RABLevelQoSParameters_t;

/** @brief Constructor of E_RABLevelQoSParameters type
 *  @return E_RABLevelQoSParameters_t allocated  and initialized structure
 * */
E_RABLevelQoSParameters_t *new_E_RABLevelQoSParameters();


/**@brief GTP-TEID
 *
 * ASN.1
 * GTP-TEID                    ::= OCTET STRING (SIZE (4))
 * */
typedef struct GTP_TEID_c {
    uint8_t teid[4];
}GTP_TEID_t;

/**@brief TransportLayerAddress
 * ASN.1
 * TransportLayerAddress       ::= BIT STRING (SIZE(1..160, ...))
 */
typedef struct TransportLayerAddress_c {
    void        (*freeIE)(void *);
    void        (*showIE)(void *);
    uint8_t     ext;
    uint16_t    len;
    uint8_t     addr[20];
    uint8_t     *extension;
}TransportLayerAddress_t;

/** @brief Constructor of TransportLayerAddress type
 *  @return TransportLayerAddress_t allocated  and initialized structure
 * */
TransportLayerAddress_t *new_TransportLayerAddress();

/**@brief E-RABToBeSetupItemCtxtSUReq
 *
 * ASN.1
 * E-RABToBeSetupItemCtxtSUReq ::= SEQUENCE {
 *     e-RAB-ID                    E-RAB-ID,
 *     e-RABlevelQoSParameters                 E-RABLevelQoSParameters,
 *     transportLayerAddress           TransportLayerAddress,
 *     gTP-TEID            GTP-TEID,
 *     nAS-PDU             NAS-PDU     OPTIONAL,
 *    iE-Extensions                   ProtocolExtensionContainer { {E-RABToBeSetupItemCtxtSUReqExtIEs} } OPTIONAL,
 *    ...
 * }
 * E-RABToBeSetupItemCtxtSUReqExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     { ID id-Correlation-ID      CRITICALITY ignore  EXTENSION Correlation-ID        PRESENCE optional},
 *     ...
 * }
 * */
typedef struct E_RABToBeSetupItemCtxtSUReq_c{
    void                            (*freeIE)(void *);
    void                            (*showIE)(void *);
    uint8_t                         ext;    /*< Extension flag*/
    uint8_t                         opt;
    E_RAB_ID_t                      eRAB_ID;
    E_RABLevelQoSParameters_t       *eRABlevelQoSParameters;
    TransportLayerAddress_t         *transportLayerAddress;
    GTP_TEID_t                      gTP_TEID;
    Unconstrained_Octed_String_t    *nAS_PDU;
    ProtocolExtensionContainer_t    *iEext;
}E_RABToBeSetupItemCtxtSUReq_t;

/** @brief Constructor of E-RABToBeSetupItemCtxtSUReq type
 *  @return E_RABToBeSetupItemCtxtSUReq_t allocated  and initialized structure
 * */
E_RABToBeSetupItemCtxtSUReq_t *new_E_RABToBeSetupItemCtxtSUReq();


/**@brief E-RABToBeSetupListCtxtSUReq
 *
 * ASN.1
 * E-RABToBeSetupListCtxtSUReq ::= SEQUENCE (SIZE(1.. maxNrOfE-RABs)) OF ProtocolIE-SingleContainer { {E-RABToBeSetupItemCtxtSUReqIEs} }
 *
 * E-RABToBeSetupItemCtxtSUReqIEs  S1AP-PROTOCOL-IES ::= {
 *     { ID id-E-RABToBeSetupItemCtxtSUReq  CRITICALITY reject     TYPE E-RABToBeSetupItemCtxtSUReq  PRESENCE mandatory },
 *     ...
 * }
 * */
typedef struct E_RABToBeSetupListCtxtSUReq_c{
    void                                (*freeIE)(void *);
    void                                (*showIE)(void *);
    void                                (*additem)(struct E_RABToBeSetupListCtxtSUReq_c*, ProtocolIE_SingleContainer_t* ie);
    uint8_t                             size;               /*<Number of items*/
    ProtocolIE_SingleContainer_t        **item;             /*<Item Array */
}E_RABToBeSetupListCtxtSUReq_t;

/** @brief Constructor of E-RABToBeSetupListCtxtSUReq type
 *  @return E_RABToBeSetupListCtxtSUReq_t allocated  and initialized structure
 * */
E_RABToBeSetupListCtxtSUReq_t *new_E_RABToBeSetupListCtxtSUReq();


/**@brief SecurityKey
 * ASN.1
 * SecurityKey ::= BIT STRING (SIZE(256))
 */
typedef struct SecurityKey_c {
    void        (*freeIE)(void *);
    void        (*showIE)(void *);
    uint8_t     key[32];
}SecurityKey_t;

/** @brief Constructor of SecurityKey type
 *  @return SecurityKey_t allocated  and initialized structure
 * */
SecurityKey_t *new_SecurityKey();


/**@brief SubscriberProfileIDforRFP
 * ASN.1
 * SubscriberProfileIDforRFP ::= INTEGER (1..256)
 */
typedef struct SubscriberProfileIDforRFP_c {
    void        (*freeIE)(void *);
    void        (*showIE)(void *);
    uint16_t    subscriberProfileIDforRFP;
}SubscriberProfileIDforRFP_t;

/** @brief Constructor of SubscriberProfileIDforRFP type
 *  @return SubscriberProfileIDforRFP_t allocated  and initialized structure
 * */
SubscriberProfileIDforRFP_t *new_SubscriberProfileIDforRFP();


/**@brief IntegrityProtectionAlgorithms
 * ASN.1
 * IntegrityProtectionAlgorithms ::= BIT STRING (SIZE (16,...))
 */
typedef struct IntegrityProtectionAlgorithms_c {
    uint8_t     ext;
    uint16_t    v;
    uint8_t     *extension;
}IntegrityProtectionAlgorithms_t;


/**@brief EncryptionAlgorithms
 * ASN.1
 * EncryptionAlgorithms ::= BIT STRING (SIZE (16,...))
 */
typedef struct EncryptionAlgorithms_c {
    uint8_t     ext;
    uint16_t    v;
    uint8_t     *extension;
}EncryptionAlgorithms_t;


/**@brief UESecuityCapabilities
 * ASN.1
 * UESecuityCapabilities ::= SEQUENCE {
 *     encryptionAlgorithms            EncryptionAlgorithms,
 *     integrityProtectionAlgorithms   IntegrityProtectionAlgorithms,
 *     iE-Extensions                   ProtocolExtensionContainer { { UESecurityCapabilities-ExtIEs} }             OPTIONAL,
 * ...
 * }
 *
 * UESecurityCapabilities-ExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     ...
 * }
 */
typedef struct UESecuityCapabilities_c {
    void                            (*freeIE)(void *);
    void                            (*showIE)(void *);
    uint8_t                         ext;
    uint8_t                         opt;
    EncryptionAlgorithms_t          encryptionAlgorithms;
    IntegrityProtectionAlgorithms_t integrityProtectionAlgorithms;
    ProtocolExtensionContainer_t    *iEext;
}UESecurityCapabilities_t;

/** @brief Constructor of UESecurityCapabilities type
 *  @return UESecurityCapabilities_t allocated  and initialized structure
 * */
UESecurityCapabilities_t *new_UESecurityCapabilities();


/**@brief UERadioCapability
 *
 * ASN.1:
 * UERadioCapability ::=  OCTET STRING
 * */
typedef struct UERadioCapability_c{
    void        (*freeIE)(void*);
    void        (*showIE)(void*);
    uint32_t    len;
    uint8_t     *str;
}UERadioCapability_t;

/** @brief Constructor of UERadioCapability type
 *  @return UERadioCapability_t allocated  and initialized structure
 * */
UERadioCapability_t *new_UERadioCapability();

/**@brief UE-S1AP-ID-pair
 * ASN.1
 * UE-S1AP-ID-pair ::= SEQUENCE{
 *     mME-UE-S1AP-ID      MME-UE-S1AP-ID,
 *     eNB-UE-S1AP-ID      ENB-UE-S1AP-ID,
 *     iE-Extensions       ProtocolExtensionContainer { {UE-S1AP-ID-pair-ExtIEs} } OPTIONAL,
 *     ...
 * }
 * UE-S1AP-ID-pair-ExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     ...
 * }
 */
typedef struct UE_S1AP_ID_pair_c {
    void                            (*freeIE)(void *);
    void                            (*showIE)(void *);
    uint8_t                         ext;
    uint8_t                         opt;
    MME_UE_S1AP_ID_t                *mME_UE_S1AP_ID;
    ENB_UE_S1AP_ID_t                *eNB_UE_S1AP_ID;
    ProtocolExtensionContainer_t    *iEext;
}UE_S1AP_ID_pair_t;

/** @brief Constructor of UE_S1AP_ID_pair type
 *  @return UE_S1AP_ID_pair_t allocated  and initialized structure
 * */
UE_S1AP_ID_pair_t *new_UE_S1AP_ID_pair();


/** UE-S1AP-IDs
 * ASN.1 UE-S1AP-IDs
 * UE-S1AP-IDs ::= CHOICE{
 *     uE-S1AP-ID-pair     UE-S1AP-ID-pair,
 *     mME-UE-S1AP-ID      MME-UE-S1AP-ID,
 *     ...
 * }
 */
typedef struct UE_S1AP_IDs_c{
    void                (*freeIE)(void*);
    void                (*showIE)(void*);
    uint8_t             ext;     /*< Extension flag*/
    uint8_t             choice;     /*< choice flag*/
    union{
        UE_S1AP_ID_pair_t   *uE_S1AP_ID_pair;
        MME_UE_S1AP_ID_t    *mME_UE_S1AP_ID;
    }uE_S1AP_ID;
}UE_S1AP_IDs_t;

/** @brief Constructor of UE_S1AP_IDs type
 *  @return UE_S1AP_IDs_t allocated  and initialized structure
 * */
UE_S1AP_IDs_t *new_UE_S1AP_IDs();

/**
 * ASN.1
 * PDCP-SN ::= INTEGER (0..4095)
 */
typedef struct PDCP_SN_c{
    uint16_t pDCP_SN:12;
}PDCP_SN_t;


/** Hyper frame number
 * ASN.1
 * HFN ::= INTEGER (0..1048575)
 */
typedef struct HFN_c{
    uint32_t hFN:20;
}HFN_t;


/**@brief COUNTvalue
 * ASN.1
 * COUNTvalue ::= SEQUENCE {
 *     pDCP-SN         PDCP-SN,
 *     hFN             HFN,
 *     iE-Extensions   ProtocolExtensionContainer { {COUNTvalue-ExtIEs} } OPTIONAL,
 *     ...
 * }
 * COUNTvalue-ExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     ...
 * }
 */
typedef struct COUNTvalue_c {
    void                            (*freeIE)(void *);
    void                            (*showIE)(void *);
    uint8_t                         ext;
    uint8_t                         opt;
    PDCP_SN_t                       pDCP_SN ;
    HFN_t                           hFN;
    ProtocolExtensionContainer_t    *iEext;
}COUNTvalue_t;

/** @brief Constructor of COUNTvalue type
 *  @return COUNTvalue_t allocated  and initialized structure
 * */
COUNTvalue_t *new_COUNTvalue();

/**@brief ReceiveStatusofULPDCPSDUs (NOT IMPLEMENTED)
 * ASN.1
 * ReceiveStatusofULPDCPSDUs ::= BIT STRING (SIZE(4096))
 */
typedef uint8_t ReceiveStatusofULPDCPSDUs_t[512];

/**@briefBearers-SubjectToStatusTransfer-Item
 * ASN.1
 * Bearers-SubjectToStatusTransfer-Item ::= SEQUENCE {
 *     e-RAB-ID                                E-RAB-ID,
 *     uL-COUNTvalue                           COUNTvalue,
 *     dL-COUNTvalue                           COUNTvalue,
 *     receiveStatusofULPDCPSDUs               ReceiveStatusofULPDCPSDUs           OPTIONAL,
 *     iE-Extensions                           ProtocolExtensionContainer { {Bearers-SubjectToStatusTransfer-ItemExtIEs} } OPTIONAL,
 *     ...
 * }
 */
typedef struct Bearers_SubjectToStatusTransfer_Item_c {
    void                            (*freeIE)(void *);
    void                            (*showIE)(void *);
    uint8_t                         ext;
    uint8_t                         opt;
    E_RAB_ID_t                      eRAB_ID;
    COUNTvalue_t                    *uL_COUNTvalue;
    COUNTvalue_t                    *dL_COUNTvalue;
    ReceiveStatusofULPDCPSDUs_t     receiveStatusofULPDCPSDUs;
    ProtocolExtensionContainer_t    *iEext;
}Bearers_SubjectToStatusTransfer_Item_t;

/** @brief Constructor of Bearers_SubjectToStatusTransfer_Item type
 *  @return Bearers_SubjectToStatusTransfer_Item_t allocated  and initialized structure
 * */
Bearers_SubjectToStatusTransfer_Item_t *new_Bearers_SubjectToStatusTransfer_Item();


/**@brief Bearers_SubjectToStatusTransferList
 *
 * ASN.1
 * Bearers-SubjectToStatusTransferList ::= SEQUENCE (SIZE(1.. maxNrOfE-RABs)) OF ProtocolIE-SingleContainer { { Bearers-SubjectToStatusTransfer-ItemIEs } }
 * */
typedef struct Bearers_SubjectToStatusTransferList_c{
    void                                (*freeIE)(void *);
    void                                (*showIE)(void *);
    void                                (*additem)(struct Bearers_SubjectToStatusTransferList_c*, ProtocolIE_SingleContainer_t* ie);
    uint8_t                             size;               /*<Number of items*/
    ProtocolIE_SingleContainer_t        **item;             /*<Item Array */
}Bearers_SubjectToStatusTransferList_t;

/** @brief Constructor of Bearers_SubjectToStatusTransferList type
 *  @return Bearers_SubjectToStatusTransferList_t allocated  and initialized structure
 * */
Bearers_SubjectToStatusTransferList_t *new_Bearers_SubjectToStatusTransferList();


/**@brief ENB-StatusTransfer-TransparentContainer
 * ASN.1
 * ENB-StatusTransfer-TransparentContainer     ::= SEQUENCE {
 *     bearers-SubjectToStatusTransferList     Bearers-SubjectToStatusTransferList,
 *     iE-Extensions           ProtocolExtensionContainer { {ENB-StatusTransfer-TransparentContainer-ExtIEs} } OPTIONAL,
 *     ...
 * }
 */
typedef struct ENB_StatusTransfer_TransparentContainer_c {
    void                                    (*freeIE)(void *);
    void                                    (*showIE)(void *);
    uint8_t                                 ext;
    uint8_t                                 opt;
    Bearers_SubjectToStatusTransferList_t   *bearers_SubjectToStatusTransferList;
    ProtocolExtensionContainer_t            *iEext;
}ENB_StatusTransfer_TransparentContainer_t;

/** @brief Constructor of ENB_StatusTransfer_TransparentContainer type
 *  @return ENB_StatusTransfer_TransparentContainer_t allocated  and initialized structure
 * */
ENB_StatusTransfer_TransparentContainer_t *new_ENB_StatusTransfer_TransparentContainer();


/**@brief E-RABItem
 * ASN.1
 * E-RABItem ::= SEQUENCE {
 *     e-RAB-ID                    E-RAB-ID,
 *     cause                       Cause,
 *     iE-Extensions               ProtocolExtensionContainer { {E-RABItem-ExtIEs} } OPTIONAL,
 *     ...
 * }
 */
typedef struct E_RABItem_c {
    void                                    (*freeIE)(void *);
    void                                    (*showIE)(void *);
    uint8_t                                 ext;
    uint8_t                                 opt;
    E_RAB_ID_t                              eRAB_ID;
    Cause_t                                 *cause;
    ProtocolExtensionContainer_t            *iEext;
}E_RABItem_t;

/** @brief Constructor of E_RABItem type
 *  @return E_RABItem_t allocated and initialized structure
 * */
E_RABItem_t *new_E_RABItem();


/**@brief E-RABList
 * ASN.1
 * E-RABList ::= SEQUENCE (SIZE(1.. maxNrOfE-RABs)) OF ProtocolIE-SingleContainer { {E-RABItemIEs} }
 *
 * E-RABItemIEs    S1AP-PROTOCOL-IES ::= {
 *     { ID id-E-RABItem    CRITICALITY ignore     TYPE E-RABItem  PRESENCE mandatory },
 *     ...
 * }
 * */
typedef struct E_RABList_c{
    void                                (*freeIE)(void *);
    void                                (*showIE)(void *);
    void                                (*additem)(struct E_RABList_c*, ProtocolIE_SingleContainer_t* ie);
    uint8_t                             size;               /*<Number of items*/
    ProtocolIE_SingleContainer_t        **item;             /*<Item Array */
}E_RABList_t;

/** @brief Constructor of E_RABList type
 *  @return E_RABList_t allocated  and initialized structure
 * */
E_RABList_t *new_E_RABList();

/**@brief E-RABToBeModifiedItemBearerModReq
 * ASN.1
 * E-RABToBeModifiedItemBearerModReq ::= SEQUENCE {
 *     e-RAB-ID                        E-RAB-ID,
 *     e-RABLevelQoSParameters         E-RABLevelQoSParameters,
 *     nAS-PDU                         NAS-PDU,
 *     iE-Extensions                   ProtocolExtensionContainer { {E-RABToBeModifyItemBearerModReqExtIEs} } OPTIONAL,
 *     ...
 * }
 *
 * E-RABToBeModifyItemBearerModReqExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     ...
 * }
 */
typedef struct E_RABToBeModifiedItemBearerModReq_c {
    void                                    (*freeIE)(void *);
    void                                    (*showIE)(void *);
    uint8_t                                 ext;
    uint8_t                                 opt;
    E_RAB_ID_t                              eRAB_ID;
    E_RABLevelQoSParameters_t               *eRABLevelQoSParameters ;
    Unconstrained_Octed_String_t            *nAS_PDU;
    ProtocolExtensionContainer_t            *iEext;
}E_RABToBeModifiedItemBearerModReq_t;

/** @brief Constructor of E_RABToBeModifiedItemBearerModReq type
 *  @return E_RABToBeModifiedItemBearerModReq_t allocated and initialized structure
 * */
E_RABToBeModifiedItemBearerModReq_t *new_E_RABToBeModifiedItemBearerModReq();


/**@brief E-RABToBeModifiedListBearerModReq
 * E-RABToBeModifiedListBearerModReq ::= SEQUENCE (SIZE(1.. maxNrOfE-RABs)) OF ProtocolIE-SingleContainer { {E-RABToBeModifiedItemBearerModReqIEs} }
 *
 * E-RABToBeModifiedItemBearerModReqIEs    S1AP-PROTOCOL-IES ::= {
 *     { ID id-E-RABToBeModifiedItemBearerModReq    CRITICALITY reject     TYPE E-RABToBeModifiedItemBearerModReq   PRESENCE mandatory },
 *     ...
 * }
*/
typedef struct E_RABToBeModifiedListBearerModReq_c{
    void                                (*freeIE)(void *);
    void                                (*showIE)(void *);
    void                                (*additem)(struct E_RABToBeModifiedListBearerModReq_c*, ProtocolIE_SingleContainer_t* ie);
    uint8_t                             size;               /*<Number of items*/
    ProtocolIE_SingleContainer_t        **item;             /*<Item Array */
}E_RABToBeModifiedListBearerModReq_t;

/** @brief Constructor of E-RABToBeModifiedListBearerModReq type
 *  @return E_RABToBeModifiedListBearerModReq_t allocated  and initialized structure
 * */
E_RABToBeModifiedListBearerModReq_t *new_E_RABToBeModifiedListBearerModReq();


/**@brief E-RABToBeSetupItemBearerSUReq
 * ASN.1
 * E-RABToBeSetupItemBearerSUReq ::= SEQUENCE {
 *     e-RAB-ID                        E-RAB-ID,
 *     e-RABlevelQoSParameters         E-RABLevelQoSParameters,
 *     transportLayerAddress           TransportLayerAddress,
 *     gTP-TEID                        GTP-TEID,
 *     nAS-PDU                         NAS-PDU,
 *     iE-Extensions                   ProtocolExtensionContainer { {E-RABToBeSetupItemBearerSUReqExtIEs} } OPTIONAL,
 *     ...
 * }
 * E-RABToBeSetupItemBearerSUReqExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     { ID id-Correlation-ID      CRITICALITY ignore  EXTENSION Correlation-ID        PRESENCE optional},
 *     ...
 * }
 */
typedef struct E_RABToBeSetupItemBearerSUReq_c {
    void                                    (*freeIE)(void *);
    void                                    (*showIE)(void *);
    uint8_t                                 ext;
    uint8_t                                 opt;
    E_RAB_ID_t                              eRAB_ID;
    E_RABLevelQoSParameters_t               *eRABLevelQoSParameters ;
    TransportLayerAddress_t                 *transportLayerAddress;
    GTP_TEID_t                              gTP_TEID;
    Unconstrained_Octed_String_t                               *nAS_PDU;
    ProtocolExtensionContainer_t            *iEext;
}E_RABToBeSetupItemBearerSUReq_t;

/** @brief Constructor of E_RABToBeSetupItemBearerSUReq type
 *  @return E_RABToBeSetupItemBearerSUReq_t allocated and initialized structure
 * */
E_RABToBeSetupItemBearerSUReq_t *new_E_RABToBeSetupItemBearerSUReq();


/**@brief E-RABToBeSetupListBearerSUReq
 * E-RABToBeSetupListBearerSUReq ::= SEQUENCE (SIZE(1.. maxNrOfE-RABs)) OF ProtocolIE-SingleContainer { {E-RABToBeSetupItemBearerSUReqIEs} }
 *
 * E-RABToBeSetupItemBearerSUReqIEs    S1AP-PROTOCOL-IES ::= {
 *     { ID id-E-RABToBeSetupItemBearerSUReq    CRITICALITY reject     TYPE E-RABToBeSetupItemBearerSUReq   PRESENCE mandatory },
 *     ...
 * }
 */
typedef struct E_RABToBeSetupListBearerSUReq_c{
    void                                (*freeIE)(void *);
    void                                (*showIE)(void *);
    void                                (*additem)(struct E_RABToBeSetupListBearerSUReq_c*, ProtocolIE_SingleContainer_t* ie);
    uint8_t                             size;               /*<Number of items*/
    ProtocolIE_SingleContainer_t        **item;             /*<Item Array */
}E_RABToBeSetupListBearerSUReq_t;

/** @brief Constructor of E-RABToBeModifiedListBearerModReq type
 *  @return E_RABToBeModifiedListBearerModReq_t allocated  and initialized structure
 * */
E_RABToBeSetupListBearerSUReq_t *new_E_RABToBeSetupListBearerSUReq();



/**@brief E-RABSetupItemBearerSURes
 * ASN.1
 * E-RABSetupItemBearerSURes ::= SEQUENCE {
 *     e-RAB-ID                    E-RAB-ID,
 *     transportLayerAddress       TransportLayerAddress,
 *     gTP-TEID                    GTP-TEID,
 *     iE-Extensions               ProtocolExtensionContainer { {E-RABSetupItemBearerSUResExtIEs} } OPTIONAL,
 *     ...
 * }
 *
 * E-RABSetupItemBearerSUResExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     ...
 * }
 */
typedef struct E_RABSetupItemBearerSURes_c {
    void                                    (*freeIE)(void *);
    void                                    (*showIE)(void *);
    uint8_t                                 ext;
    uint8_t                                 opt;
    E_RAB_ID_t                              eRAB_ID;
    TransportLayerAddress_t                 *transportLayerAddress;
    GTP_TEID_t                              gTP_TEID;
    ProtocolExtensionContainer_t            *iEext;
}E_RABSetupItemBearerSURes_t;

/** @brief Constructor of E_RABSetupItemBearerSURea type
 *  @return E_RABSetupItemBearerSUReq_t allocated and initialized structure
 * */
E_RABSetupItemBearerSURes_t *new_E_RABSetupItemBearerSURes();


/**@brief E-RABSetupListBearerSURes
 * E-RABSetupListBearerSURes ::= SEQUENCE (SIZE(1.. maxNrOfE-RABs)) OF ProtocolIE-SingleContainer { {E-RABSetupItemBearerSUResIEs} }
 *
 * E-RABSetupItemBearerSUResIEs    S1AP-PROTOCOL-IES ::= {
 *     { ID id-E-RABSetupItemBearerSURes    CRITICALITY ignore     TYPE E-RABSetupItemBearerSURes  PRESENCE mandatory },
 *     ...
 * }
 */
typedef struct E_RABSetupListBearerSURes_c{
    void                                (*freeIE)(void *);
    void                                (*showIE)(void *);
    void                                (*additem)(struct E_RABSetupListBearerSURes_c*, ProtocolIE_SingleContainer_t* ie);
    uint8_t                             size;               /*<Number of items*/
    ProtocolIE_SingleContainer_t        **item;             /*<Item Array */
}E_RABSetupListBearerSURes_t;

/** @brief Constructor of E-RABSetupListBearerSURes type
 *  @return E_RABSetupListBearerSURes_t allocated  and initialized structure
 * */
E_RABSetupListBearerSURes_t *new_E_RABSetupListBearerSURes();


typedef E_RABSetupItemBearerSURes_t E_RABSetupItemCtxtSURes_t;
#define new_E_RABSetupItemCtxtSURes new_E_RABSetupItemBearerSURes

typedef E_RABSetupListBearerSURes_t E_RABSetupListCtxtSURes_t;
#define new_E_RABSetupListCtxtSURes new_E_RABSetupListBearerSURes

/**@brief HandoverType
 *
 * ASN.1
 * HandoverType ::= ENUMERATED {
 *     intralte,
 *     ltetoutran,
 *     ltetogeran,
 *     utrantolte,
 *     gerantolte,
 *     ...
 * }
 */
typedef struct HandoverType_c{
    void    (*freeIE)(void *);
    void    (*showIE)(void *);
    uint8_t ext;
    union{
        enum{
            HandoverType_intralte,
            HandoverType_ltetoutran,
            HandoverType_ltetogeran,
            HandoverType_utrantolte,
            HandoverType_gerantolte,
        }noext;
        enum{
            HandoverType_noext,
        }ext;
    }ht;
}HandoverType_t;

/** @brief Constructor of HandoverType type
 *  @return HandoverType_t allocated  and initialized structure
 * */
HandoverType_t *new_HandoverType();

extern const char *HandoverTypeName[];


/**@brief TargeteNB-ID
 * ASN.1
 * TargeteNB-ID ::= SEQUENCE {
 *     global-ENB-ID   Global-ENB-ID,
 *     selected-TAI    TAI,
 *     iE-Extensions   ProtocolExtensionContainer { {TargeteNB-ID-ExtIEs} } OPTIONAL,
 *     ...
 * }
 * TargeteNB-ID-ExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     ...
 * }
 */
typedef struct TargeteNB_ID_c {
    void                                    (*freeIE)(void *);
    void                                    (*showIE)(void *);
    uint8_t                                 ext;
    uint8_t                                 opt;
    Global_ENB_ID_t                         *global_ENB_ID;
    TAI_t                                   *selected_TAI;
    ProtocolExtensionContainer_t            *iEext;
}TargeteNB_ID_t;

/** @brief Constructor of E_TargeteNB type
 *  @return E_TargeteNB_t allocated and initialized structure
 * */
TargeteNB_ID_t *new_TargeteNB_ID();


/**@brief LAC
 *
 * ASN.1
 * LAC ::= OCTET STRING (SIZE (2))
 */
typedef struct LAC_c {
    uint8_t s[2];
}LAC_t;


/*@brief LAI
 * This information element is used to uniquely identify a Location Area (clause 9.2.3.1 )
 * ASN.1
 * LAI ::= SEQUENCE {
 *     pLMNidentity                PLMNidentity,
 *     lAC             LAC,
 *     iE-Extensions           ProtocolExtensionContainer { {LAI-ExtIEs} } OPTIONAL,
 *     ...
 * }
 * LAI-ExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     ...
 * }
 */
typedef struct LAI_c {
    void                                    (*freeIE)(void *);
    void                                    (*showIE)(void *);
    uint8_t                                 ext;
    uint8_t                                 opt;
    PLMNidentity_t                          *pLMNidentity;
    LAC_t                                   lAC;
    ProtocolExtensionContainer_t            *iEext;
}LAI_t;

/** @brief Constructor of LAI type
 *  @return LAI_t allocated and initialized structure
 * */
LAI_t *new_LAI();


/**@brief RNC-ID
 * ASN.1
 * RNC-ID                  ::= INTEGER (0..4095)
 */
typedef struct RNC_ID_c{
    uint16_t pDCP_SN:12;
}RNC_ID_t;


/*@brief TargetRNC-ID
 * ASN.1
 * TargetRNC-ID ::= SEQUENCE {
 *     lAI                 LAI,
 *     rAC                 RAC         OPTIONAL,
 *     rNC-ID              RNC-ID,
 *     extendedRNC-ID      ExtendedRNC-ID      OPTIONAL,
 *     iE-Extensions       ProtocolExtensionContainer { {TargetRNC-ID-ExtIEs} } OPTIONAL,
 *     ...
 * }
 * TargetRNC-ID-ExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     ...
 * }
 */
typedef struct TargetRNC_ID_c {
    void                            (*freeIE)(void *);
    void                            (*showIE)(void *);
    uint8_t                         ext;
    uint8_t                         opt;
    LAI_t                           *lAI;
    /*RAC_t                           rAC;*/
    RNC_ID_t                        rNC_ID;
    ProtocolExtensionContainer_t    *iEext;
}TargetRNC_ID_t;

/** @brief Constructor of TargetRNC_ID type
 *  @return TargetRNC_ID_t allocated and initialized structure
 * */
TargetRNC_ID_t *new_TargetRNC_ID();


/**@brief CI
 *
 * ASN.1
 * CI  ::= OCTET STRING (SIZE (2))
 */
typedef struct CI_c {
    uint8_t s[2];
}CI_t;


/*@brief CGI
 * ASN.1
 * CGI ::= SEQUENCE {
 *     pLMNidentity                PLMNidentity,
 *     lAC             LAC,
 *     cI              CI,
 *     rAC             RAC                     OPTIONAL,
 *     iE-Extensions   ProtocolExtensionContainer { {CGI-ExtIEs} } OPTIONAL,
 *     ...
 * }
 * CGI-ExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     ...
 * }
 */
typedef struct CGI_c {
    void                                    (*freeIE)(void *);
    void                                    (*showIE)(void *);
    uint8_t                                 ext;
    uint8_t                                 opt;
    PLMNidentity_t                          *pLMNidentity;
    LAC_t                                   lAC;
    CI_t                                    cI;
    /*RAC_t                                   rAC;*/
    ProtocolExtensionContainer_t            *iEext;
}CGI_t;

/** @brief Constructor of CGI type
 *  @return CGI_t allocated and initialized structure
 * */
CGI_t *new_CGI();


/** TargetID
 * ASN.1
 * TargetID ::= CHOICE {
 *     targeteNB-ID            TargeteNB-ID,
 *     targetRNC-ID            TargetRNC-ID,
 *     cGI                     CGI,
 *     ...
 * }
 */
typedef struct TargetID_c{
    void                (*freeIE)(void*);
    void                (*showIE)(void*);
    uint8_t             ext;     /*< Extension flag*/
    uint8_t             choice;     /*< choice flag*/
    union{
        TargeteNB_ID_t  *targeteNB_ID;
        TargetRNC_ID_t  *targetRNC_ID;
        CGI_t           *cGI;
    }targetID;
}TargetID_t;

/** @brief Constructor of TargetID type
 *  @return TargetID_t allocated  and initialized structure
 * */
TargetID_t *new_TargetID();


/**@brief Direct-Forwarding-Path-Availability
 *
 * ASN.1
 * Direct-Forwarding-Path-Availability ::= ENUMERATED {
 *     directPathAvailable,
 *     ...
 * }
 * */
typedef struct Direct_Forwarding_Path_Availability_c{
    void    (*freeIE)(void *);
    void    (*showIE)(void *);
    uint8_t ext;
    enum dfpa_c{
        Direct_Forwarding_Path_Availability_directPathAvailable,
    }ind;
}Direct_Forwarding_Path_Availability_t;

/** @brief Constructor of Direct_Forwarding_Path_Availability type
 *  @return Direct_Forwarding_Path_Availability_t allocated  and initialized structure
 * */
Direct_Forwarding_Path_Availability_t *new_Direct_Forwarding_Path_Availability();


/**@brief E-RABToBeSetupListHOReq
 *
 * ASN.1
 * E-RABToBeSetupListHOReq                     ::= E-RAB-IE-ContainerList { {E-RABToBeSetupItemHOReqIEs} }
 *
 * E-RABToBeSetupItemHOReqIEs S1AP-PROTOCOL-IES ::= {
 *     { ID id-E-RABToBeSetupItemHOReq         CRITICALITY reject  TYPE E-RABToBeSetupItemHOReq             PRESENCE mandatory    },
 *     ...
 * }
 * */
typedef E_RABToBeSetupListCtxtSUReq_t E_RABToBeSetupListHOReq_t;

/** @brief Constructor of E-RABToBeSetupListHOReq type
 *  @return E_RABToBeSetupListHOReq_t allocated  and initialized structure
 * */
E_RABToBeSetupListHOReq_t *new_E_RABToBeSetupListHOReq();


/**@brief E-RABToBeSetupItemHOReq
 *
 * ASN.1
 * E-RABToBeSetupItemHOReq ::= SEQUENCE {
 *     e-RAB-ID                            E-RAB-ID,
 *     transportLayerAddress               TransportLayerAddress,
 *     gTP-TEID                            GTP-TEID,
 *     e-RABlevelQosParameters             E-RABLevelQoSParameters,
 *     iE-Extensions                       ProtocolExtensionContainer { {E-RABToBeSetupItemHOReq-ExtIEs} }                OPTIONAL,
 *     ...
 * }
 * E-RABToBeSetupItemHOReq-ExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     {ID id-Data-Forwarding-Not-Possible    CRITICALITY ignore   EXTENSION  Data-Forwarding-Not-Possible   PRESENCE optional},
 *     ...
 * }
 * */
typedef struct E_RABToBeSetupItemHOReq_c{
    void                            (*freeIE)(void *);
    void                            (*showIE)(void *);
    uint8_t                         ext;    /*< Extension flag*/
    uint8_t                         opt;
    E_RAB_ID_t                      eRAB_ID;
    TransportLayerAddress_t         *transportLayerAddress;
    GTP_TEID_t                      gTP_TEID;
    E_RABLevelQoSParameters_t       *eRABLevelQoSParameters;
    ProtocolExtensionContainer_t    *iEext;
}E_RABToBeSetupItemHOReq_t;

/** @brief Constructor of E-RABToBeSetupItemHOReq type
 *  @return E_RABToBeSetupItemHOReq_t allocated  and initialized structure
 * */
E_RABToBeSetupItemHOReq_t *new_E_RABToBeSetupItemHOReq();


/**@brief SecurityContext
 *
 * ASN.1
 * SecurityContext ::= SEQUENCE {
 *     nextHopChainingCount        INTEGER (0..7),
 *     nextHopParameter            SecurityKey,
 *     iE-Extensions               ProtocolExtensionContainer { { SecurityContext-ExtIEs} } OPTIONAL,
 *     ...
 * }
 * SecurityContext-ExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     ...
 * }
 * */
typedef struct SecurityContext_c{
    void                            (*freeIE)(void *);
    void                            (*showIE)(void *);
    uint8_t                         ext;    /*< Extension flag*/
    uint8_t                         opt;
    uint8_t                         nextHopChainingCount : 3;
    SecurityKey_t                   *nextHopParameter;
    ProtocolExtensionContainer_t    *iEext;
}SecurityContext_t;

/** @brief Constructor of SecurityContext type
 *  @return SecurityContext_t allocated  and initialized structure
 * */
SecurityContext_t *new_SecurityContext();

/*
E-RABAdmittedList                   ::= E-RAB-IE-ContainerList { {E-RABAdmittedItemIEs} }

E-RABAdmittedItemIEs S1AP-PROTOCOL-IES ::= {
    { ID id-E-RABAdmittedItem           CRITICALITY ignore  TYPE E-RABAdmittedItem          PRESENCE mandatory  },
    ...
}
 */
typedef E_RABToBeSetupListCtxtSUReq_t E_RABAdmittedList_t;

E_RABAdmittedList_t *new_E_RABAdmittedList();


/**@brief E-RABAdmittedItem
 *
 * ASN.1
 * E-RABAdmittedItem ::= SEQUENCE {
 *     e-RAB-ID                        E-RAB-ID,
 *     transportLayerAddress           TransportLayerAddress,
 *     gTP-TEID                        GTP-TEID,
 *     dL-transportLayerAddress        TransportLayerAddress   OPTIONAL,
 *     dL-gTP-TEID                     GTP-TEID                OPTIONAL,
 *     uL-TransportLayerAddress        TransportLayerAddress   OPTIONAL,
 *     uL-GTP-TEID                     GTP-TEID                OPTIONAL,
 *     iE-Extensions                   ProtocolExtensionContainer { {E-RABAdmittedItem-ExtIEs} }   OPTIONAL,
 *     ...
 * }
 *
 * E-RABAdmittedItem-ExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     ...
 * }
 * */
typedef struct E_RABAdmittedItem_c{
    void                            (*freeIE)(void *);
    void                            (*showIE)(void *);
    uint8_t                         ext;    /*< Extension flag*/
    uint8_t                         opt;
    E_RAB_ID_t                      eRAB_ID;
    TransportLayerAddress_t         *transportLayerAddress;
    GTP_TEID_t                      gTP_TEID;
    TransportLayerAddress_t         *dL_transportLayerAddress;
    GTP_TEID_t                      dL_GTP_TEID;
    TransportLayerAddress_t         *uL_transportLayerAddress;
    GTP_TEID_t                      uL_GTP_TEID;
    ProtocolExtensionContainer_t    *iEext;
}E_RABAdmittedItem_t;

/** @brief Constructor of E-RABAdmittedItem type
 *  @return E_RABAdmittedItem_t allocated and initialized structure
 * */
E_RABAdmittedItem_t *new_E_RABAdmittedItem();


/*
E-RABSubjecttoDataForwardingList ::= E-RAB-IE-ContainerList { {E-RABDataForwardingItemIEs} }

E-RABDataForwardingItemIEs S1AP-PROTOCOL-IES ::= {
    { ID id-E-RABDataForwardingItem         CRITICALITY ignore  TYPE E-RABDataForwardingItem             PRESENCE mandatory    },
    ...
}
 */
typedef E_RABToBeSetupListCtxtSUReq_t E_RABSubjecttoDataForwardingList_t;

E_RABSubjecttoDataForwardingList_t *new_E_RABSubjecttoDataForwardingList();


/**@brief E-RABDataForwardingItem
 *
 * ASN.1
 * E-RABDataForwardingItem ::= SEQUENCE {
 *     e-RAB-ID                            E-RAB-ID,
 *     dL-transportLayerAddress            TransportLayerAddress                                                          OPTIONAL,
 *     dL-gTP-TEID                         GTP-TEID                                                                       OPTIONAL,
 *     uL-TransportLayerAddress            TransportLayerAddress                                                          OPTIONAL,
 *     uL-GTP-TEID                         GTP-TEID                                                                       OPTIONAL,
 *     iE-Extensions                       ProtocolExtensionContainer { { E-RABDataForwardingItem-ExtIEs} } OPTIONAL,
 *     ...
 * }
 *
 * E-RABDataForwardingItem-ExtIEs S1AP-PROTOCOL-EXTENSION ::= {
 *     ...
 * }
 * */
typedef struct E_RABDataForwardingItem_c{
    void                            (*freeIE)(void *);
    void                            (*showIE)(void *);
    uint8_t                         ext;    /*< Extension flag*/
    uint8_t                         opt;
    E_RAB_ID_t                      eRAB_ID;
    TransportLayerAddress_t         *dL_transportLayerAddress;
    GTP_TEID_t                      dL_GTP_TEID;
    TransportLayerAddress_t         *uL_transportLayerAddress;
    GTP_TEID_t                      uL_GTP_TEID;
    ProtocolExtensionContainer_t    *iEext;
}E_RABDataForwardingItem_t;

/** @brief Constructor of E-RABAdmittedItem type
 *  @return E_RABAdmittedItem_t allocated and initialized structure
 * */
E_RABDataForwardingItem_t *new_E_RABDataForwardingItem();

#endif /* S1AP_IE_H_ */
