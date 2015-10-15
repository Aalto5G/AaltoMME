/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S1AP_IEdec.c
 * @Author Vicent Ferrer
 * @date   April, 2013
 * @brief
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>


#include "S1AP_IEenc.h"
#include "S1AP_IE.h"
#include "rt_per_bin.h"
#include "S1APlog.h"

/* Required prototypes*/
void enc_CSGid(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie);
void enc_ServedPLMNs(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie);
void enc_MME_UE_S1AP_ID(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie);
void enc_ENB_UE_S1AP_ID(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie);
void enc_Global_ENB_ID(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie);
void enc_TAI(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie);
void enc_LAI(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie);

/* ******************** IE decoder functions ******************** */

void enc_PLMNidentity(struct BinaryData *bytes, PLMNidentity_t *id){

    /*plmnId_tbcd2MccMnc(id);*/
    if(id->tbc.s[0]== 0 && id->tbc.s[1]== 0 && id->tbc.s[2]== 0 ){
        /*The info is in the MMC MNC fields*/
        plmnId_MccMnc2tbcd(id);
    }
    encode_octet_string(bytes, id->tbc.s, 3);
}

void enc_BPLMNs(struct BinaryData *bytes, BPLMNs_t *v){

    uint32_t i;

    /*Decode length*/
    encode_constrained_number(bytes, v->size, 1, maxnoofBPLMNs);

    /*Decode SupportedTAs_Items*/
    for(i=0 ; i < v->size ; i++){
        enc_PLMNidentity(bytes, v->pLMNidentity[i]);
    }
}

void enc_SupportedTAs_Item(struct BinaryData *bytes, SupportedTAs_Item_t * item){

    /*Set Extension flag*/
    setbits(bytes, 1, item->ext);

    /*Get optionals flag*/
    setbits(bytes, 1, item->opt);

    /*attribute number 1 with type OCTET STRING*/
    encode_octet_string(bytes, item->tAC->s, 2);

    /*attribute number 2 with type BPLMNs*/
    enc_BPLMNs(bytes, item->broadcastPLMNs);

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(item->opt!=0){
        s1ap_msg(ERROR, 0, "Optional iE-Extensions is present, encoding not included in current version.\n");
    }

    /*Extensions*/
    if(item->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding
        getextension(&extensions, bytes, item->ext);
        skipextensions(bytes, 1, &extensions);*/
    }
}

void enc_CSGidList_Item(struct BinaryData *bytes, CSG_IdList_Item_t * item){

    S1AP_PROTOCOL_IES_t fakeIE;
    if(item==NULL){
        return;
    }
    /*Set Extension flag*/
    setbits(bytes, 1, item->ext);

    /*Get optionals flag*/
    setbits(bytes, 1, item->opt);

    /*attribute number 1 with type cSG_id_t*/
    if(item->cSG_id!=0){
        fakeIE.value = item->cSG_id;
        enc_CSGid(bytes, &fakeIE);
    }

    /* attribute number 2 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(item->opt!=0){
        s1ap_msg(ERROR, 0, "Optional iE-Extensions is present, encoding not included in current version.\n");
    }

    /*Extensions*/
    if(item->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding
        getextension(&extensions, bytes, item->ext);
        skipextensions(bytes, 1, &extensions);*/
    }
}

void enc_MME_Group_ID(struct BinaryData *bytes, MME_Group_ID_t * id){
    encode_octet_string(bytes, id->s, 2);
}

void enc_ServedGroupIDs(struct BinaryData *bytes, ServedGroupIDs_t *g){
    uint16_t i;

    /*Encode length*/
    encode_constrained_number(bytes, g->size, 1, maxnoofGroupIDs);

    /*Encode MME_Group_ID_t*/
    for(i=0 ; i < g->size ; i++){
        enc_MME_Group_ID(bytes, g->item[i]);
    }
}

void enc_MME_Code(struct BinaryData *bytes, MME_Code_t * id){
    encode_octet_string(bytes, id->s, 1);
}

void enc_ServedMMECs(struct BinaryData *bytes, ServedMMECs_t *g){
    uint8_t i;

    /*Encode length*/
    encode_constrained_number(bytes, g->size, 1, maxnoofMMECs);

    /*Encode MME_Code_t*/
    for(i=0 ; i < g->size ; i++){
        enc_MME_Code(bytes, g->item[i]);
    }
}

void enc_ServedGUMMEIsItem(struct BinaryData *bytes, ServedGUMMEIsItem_t * item){

    S1AP_PROTOCOL_IES_t fakeIE;
    if(item==NULL){
        return;
    }
    /*Set Extension flag*/
    setbits(bytes, 1, item->ext);

    /*Get optionals flag*/
    setbits(bytes, 1, item->opt);

    /*attribute number 1 with type servedPLMNs_t*/
    if(item->servedPLMNs!=0){
        fakeIE.value=item->servedPLMNs;
        enc_ServedPLMNs(bytes, &fakeIE);
    }

    /*attribute number 2 with type servedGroupIDs_t*/
    if(item->servedGroupIDs!=0){
        enc_ServedGroupIDs(bytes, item->servedGroupIDs);
    }

    /*attribute number 3 with type servedMMECs_t*/
    if(item->servedMMECs!=0){
        enc_ServedMMECs(bytes, item->servedMMECs);
    }

    /* attribute number 4 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(item->opt!=0){
        s1ap_msg(ERROR, 0, "Optional iE-Extensions is present, encoding not included in current version.\n");
    }

    /*Extensions*/
    if(item->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding
        getextension(&extensions, bytes, item->ext);
        skipextensions(bytes, 1, &extensions);*/
    }
}

void enc_CriticalityDiagnostics_IE_Item(struct BinaryData *bytes, CriticalityDiagnostics_IE_Item_t * item){

    /*Set Extension flag*/
    setbits(bytes, 1, item->ext);

    /*Get optionals flag*/
    setbits(bytes, 4, item->opt>>(8-4));

    /* attribute number 1 with type ENUMERATED*/
    if((item->opt&0x8) == 0x8){
        encode_constrained_number(bytes, item->iECriticality, 0, 2);
    }
    /*attribute number 2 with type INTEGER*/
    if((item->opt&0x4) == 0x4){
        encode_constrained_number(bytes, item->iE_ID, 0, 255);
    }
    /* attribute number 3 with type ENUMERATED*/
    if((item->opt&0x2) == 0x2){
        encode_constrained_number(bytes, item->typeOfError, 0, 1);
    }
    /* attribute number 4 with type  ProtocolExtensionContainer*/
    if((item->opt&0x1) == 0x1){
        s1ap_msg(ERROR, 0, "Optional iE-Extensions is present, parsing not included in current version.\n");
    }
    /*Extensions*/
    if(item->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding*/
    }
}

void enc_CriticalityDiagnostics_IE_List(struct BinaryData *bytes, CriticalityDiagnostics_IE_List_t * item){
    uint8_t i;

    /*Encode length*/
    encode_constrained_number(bytes, item->size, 1, maxNrOfErrors);

    /*Encode MME_Code_t*/
    for(i=0 ; i < item->size ; i++){
        enc_CriticalityDiagnostics_IE_Item(bytes, item->item[i]);
    }
}

void enc_AllocationAndRetentionPriority(struct BinaryData *bytes, AllocationAndRetentionPriority_t * v){
    /*Set Extension flag*/
    setbits(bytes, 1, v->ext);

    /*Get optionals flag*/
    setbits(bytes, 1, v->opt);

    /* attribute number 1 with type priorityLevel*/
     encode_constrained_number(bytes, v->priorityLevel, 0, 15);

    /* attribute number 2 with type enum*/
    encode_constrained_number(bytes, v->pre_emptionCapability, 0, 1);
    /* attribute number 3 with type enum*/
    encode_constrained_number(bytes, v->pre_emptionVulnerability, 0, 1);

    /* attribute number 4 with type  ProtocolExtensionContainer*/
    if(v->opt == 1){
        s1ap_msg(ERROR, 0, "Optional iE-Extensions is present, encoding not included in current version.\n");
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding*/
    }
}

void enc_GBR_QosInformation(struct BinaryData *bytes, GBR_QosInformation_t *v){

    /*Set Extension flag*/
    setbits(bytes, 1, v->ext);

    /*Get optionals flag*/
    setbits(bytes, 1, v->opt);

    /*attribute number 1 with type BitRate_t*/
    encode_constrained_number(bytes, v->eRAB_MaximumBitrateDL.rate, 0, 10000000000ULL);

    /*attribute number 2 with type BitRate_t*/
    encode_constrained_number(bytes, v->eRAB_MaximumBitrateUL.rate, 0, 10000000000ULL);

    /*attribute number 3 with type BitRate_t*/
    encode_constrained_number(bytes, v->eRAB_GuaranteedBitrateDL.rate, 0, 10000000000ULL);

    /*attribute number 4 with type BitRate_t*/
    encode_constrained_number(bytes, v->eRAB_GuaranteedBitrateUL.rate, 0, 10000000000ULL);

    /* attribute number 5 with type  ProtocolExtensionContainer*/
    if(v->opt == 0x1){
        s1ap_msg(ERROR, 0, "Optional iE-Extensions is present, parsing not included in current version.\n");
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding*/
    }
}

void enc_E_RABLevelQoSParameters(struct BinaryData *bytes, E_RABLevelQoSParameters_t * v){

    /*Set Extension flag*/
    setbits(bytes, 1, v->ext);

    /*Get optionals flag*/
    setbits(bytes, 5, v->opt>>(8-5));

    /* attribute number 1 with type QCI_t*/
    encode_constrained_number(bytes, v->qCI, 0, 255);

    /* attribute number 2 with type AllocationAndRetentionPriority_t*/
    enc_AllocationAndRetentionPriority(bytes, v->allocationRetentionPriority);

    /* attribute number 3 with type AllocationAndRetentionPriority_t*/
    if((v->opt&0x80) == 0x80){
         enc_GBR_QosInformation(bytes, v->gbrQosInformation);
    }else{
        /*s1ap_msg(WARN, 0, "Optional gbrQosInformation is not present. v->opt = %#x\n", v->opt);*/
    }

    /* attribute number 4 with type  ProtocolExtensionContainer*/
    if((v->opt&0x40) == 0x40){
        s1ap_msg(ERROR, 0, "Optional iE-Extensions is present, encoding not included in current version.\n");
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding*/
    }

}

void enc_TransportLayerAddress(struct BinaryData *bytes, TransportLayerAddress_t * v){
    uint16_t i;
    /*Set Extension flag*/
    setbits(bytes, 1, v->ext);

    /*Decode length*/
    encode_constrained_number(bytes, v->len, 1, 160);

    align_enc(bytes);
    for(i=0; i<v->len/8; i++){
        setbits(bytes, 8, v->addr[i]);
    }
    if(v->len%8!=0){
        setbits(bytes, v->len%8, v->addr[i+1]);
    }
}

void enc_UE_S1AP_ID_pair(struct BinaryData *bytes, UE_S1AP_ID_pair_t * v){
    S1AP_PROTOCOL_IES_t fakeie1, fakeie2;

    /*Get extension flag*/
    setbits(bytes, 1, v->ext);

    /*Set optionals*/
    setbits(bytes, 1, v->opt);

    /* attribute number 1 with type MME_UE_S1AP_ID */
    fakeie1.value = v->mME_UE_S1AP_ID;
    enc_MME_UE_S1AP_ID(bytes, &fakeie1);
    /* attribute number 2 with type ENB_UE_S1AP_ID */
    fakeie2.value = v->eNB_UE_S1AP_ID;
    enc_ENB_UE_S1AP_ID(bytes, &fakeie2);

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==0x01){
        enc_protocolIEs(bytes, (S1AP_PROTOCOL_IES_t*)v->iEext);
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding*/
    }
}

void enc_Bearers_SubjectToStatusTransferList(struct BinaryData *bytes, Bearers_SubjectToStatusTransferList_t *v){
    uint8_t i;

    /*Encode length*/
    encode_constrained_number(bytes, v->size, 1, maxNrOfERABs);

    /*Encode Bearers_SubjectToStatusTransfer_Item_t, tyep Bearers_SubjectToStatusTransfer_Item*/
    for(i=0 ; i < v->size ; i++){
        enc_protocolIEs(bytes, v->item[i]);
    }
}

void enc_COUNTvalue(struct BinaryData *bytes, COUNTvalue_t *v){

    /*Get extension flag*/
    setbits(bytes, 1, v->ext);

    /*Set optionals*/
    setbits(bytes, 1, v->opt);

    /* attribute number 1  with type PDCP-SN ( INTEGER (0..4095) ) */
    encode_constrained_number(bytes, (uint64_t)v->pDCP_SN.pDCP_SN, 0, 4095);

    /* attribute number 2  with type HFN ( INTEGER (0..1048575) ) */
    encode_constrained_number(bytes, (uint64_t)v->hFN.hFN, 0, 1048575);

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==0x01){
        enc_protocolIEs(bytes, (S1AP_PROTOCOL_IES_t*)v->iEext);
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding*/
    }
}

void enc_E_RAB_ID(struct BinaryData *bytes, E_RAB_ID_t *v){
    setbits(bytes, 1, v->ext);
    if(v->ext == 0){
        encode_constrained_number(bytes, v->id, 0, 15);
    }else{
        encode_semi_constrained_number(bytes, 0, v->id);
    }
}

void enc_TargeteNB_ID(struct BinaryData *bytes, TargeteNB_ID_t *v){
    S1AP_PROTOCOL_IES_t fakeie1, fakeie2;
    /*Get extension flag*/
    setbits(bytes, 1, v->ext);

    /*Set optionals*/
    setbits(bytes, 1, v->opt);

    /* attribute number 1  with type Global_ENB_ID */
    fakeie1.value = v->global_ENB_ID;
    enc_Global_ENB_ID(bytes, &fakeie1);

    /* attribute number 2  with type TAI */
    fakeie2.value = v->selected_TAI;
    enc_TAI(bytes, &fakeie2);

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==1){
        enc_protocolIEs(bytes, (S1AP_PROTOCOL_IES_t*)v->iEext);
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding*/
    }
}

void enc_TargetRNC_ID(struct BinaryData *bytes, TargetRNC_ID_t *v){
    S1AP_PROTOCOL_IES_t fakeie1;
    /*Get extension flag*/
    setbits(bytes, 1, v->ext);

    /*Set optionals*/
    setbits(bytes, 3, v->opt>>(8-3));

    /* attribute number 1  with type  LAI*/
    fakeie1.value = v->lAI;
    enc_LAI(bytes, &fakeie1);

    /* attribute number 2  with type  RAC (optional)*/
    if((v->opt&0x80) == 0x80){
        s1ap_msg(WARN, 0, "Optional is RAC present. v->opt = %#x, not implemented yet\n", v->opt);
    }else{
        /*s1ap_msg(WARN, 0, "Optional is not present. v->opt = %#x\n", v->opt);*/
    }

    /* attribute number 3 with type RNC-ID*/
    encode_constrained_number(bytes, v->rNC_ID.pDCP_SN, 0, 4095);;

    /* attribute number 4  with type  ExtendedRNC-ID (optional)*/
    if((v->opt&0x40) == 0x40){
        s1ap_msg(WARN, 0, "Optional ExtendedRNC-ID is present. v->opt = %#x, not implemented yet\n", v->opt);
    }else{
        /*s1ap_msg(WARN, 0, "Optional is not present. v->opt = %#x\n", v->opt);*/
    }

    /* attribute number 5 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if((v->opt&0x20) == 0x20){
        enc_protocolIEs(bytes, (S1AP_PROTOCOL_IES_t*)v->iEext);
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding*/
    }
}

void enc_CGI(struct BinaryData *bytes, CGI_t *v){
    /*Get extension flag*/
    setbits(bytes, 1, v->ext);

    /*Set optionals*/
    setbits(bytes, 2, v->opt>>(8-2));

    /* attribute number 1  with type PLMNidentity */
    enc_PLMNidentity(bytes, v->pLMNidentity);

    /* attribute number 2  with type LAC */
    encode_octet_string(bytes, v->lAC.s, 2);

    /* attribute number 3  with type CI */
    encode_octet_string(bytes, v->cI.s, 2);

    /* attribute number 4 with type RAC (optional)*/
    if((v->opt&0x80) == 0x80){
        s1ap_msg(ERROR, 0, "Optional RAC is present, not implemented yet\n");

    }else{
        /*s1ap_msg(WARN, 0, "Optional is not present. v->opt = %#x\n", v->opt);*/
    }

    /* attribute number 5 with type  ProtocolExtensionContainer*/
    if((v->opt&0x40) == 0x40){
        enc_protocolIEs(bytes, (S1AP_PROTOCOL_IES_t*)v->iEext);
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding*/
    }
}


void enc_UE_associatedLogicalS1_ConnectionListRes(struct BinaryData *bytes,
                                                  UE_associatedLogicalS1_ConnectionListRes_t *v){
    uint8_t i;

    /*Encode length*/
    encode_constrained_number(bytes, v->size, 1, maxNrOfIndividualS1ConnectionsToReset);

    /*Encode Bearers_SubjectToStatusTransfer_Item_t, tyep Bearers_SubjectToStatusTransfer_Item*/
    for(i=0 ; i < v->size ; i++){
        enc_protocolIEs(bytes, v->item[i]);
    }
}


void enc_UE_associatedLogicalS1_ConnectionItem(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    S1AP_PROTOCOL_IES_t fakeie1, fakeie2;

    UE_associatedLogicalS1_ConnectionItem_t  *v;
    v = (UE_associatedLogicalS1_ConnectionItem_t *)ie->value;

    /*Get extension flag*/
    setbits(bytes, 1, v->ext);

    /*Set optionals*/
    setbits(bytes, 3, v->opt);

    /* attribute number 1 with type MME_UE_S1AP_ID */
    if((v->opt&0x80) == 0x80){
        fakeie1.value = v->mME_UE_S1AP_ID;
        enc_MME_UE_S1AP_ID(bytes, &fakeie1);
    }
    /* attribute number 2 with type ENB_UE_S1AP_ID */
    if((v->opt&0x40) == 0x40){
        fakeie2.value = v->eNB_UE_S1AP_ID;
        enc_ENB_UE_S1AP_ID(bytes, &fakeie2);
    }

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if((v->opt&0x20) == 0x20){
        enc_protocolIEs(bytes, (S1AP_PROTOCOL_IES_t*)v->iEext);
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding*/
    }
}


void enc_UE_associatedLogicalS1_ConnectionListResAck(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){

    uint32_t i;
    UE_associatedLogicalS1_ConnectionListRes_t *v = (UE_associatedLogicalS1_ConnectionListRes_t *)ie->value;

    /*Encode length*/
    encode_constrained_number(bytes, v->size, 1, maxNrOfIndividualS1ConnectionsToReset);

    /*Encode SupportedTAs_Items*/
    for(i=0 ; i < v->size ; i++){
        enc_UE_associatedLogicalS1_ConnectionItem(bytes, v->item[i]);
    }
}


void enc_Global_ENB_ID(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    Global_ENB_ID_t *v;

    v = (Global_ENB_ID_t *)ie->value;

    /*Set Extension flag*/
    setbits(bytes, 1, v->ext);

    /*Set Optionals flag*/
    setbits(bytes, 1, v->opt);

    /*attribute number 1 (pLMNidentity) with type OCTET STRING */
    enc_PLMNidentity(bytes, v->pLMNidentity);

    /*attribute number 2 eNB-ID with type ENB-ID*/
    set_choice_ext(bytes, v->eNBid->choice, 2, v->eNBid->ext);

    align_enc(bytes);

    if(v->eNBid->ext!=0 || v->eNBid->choice>1){
        s1ap_msg(ERROR, 0, "Extensions not implemented or choice not correct");
        return;
    }

    align_enc(bytes);
    if(v->eNBid->choice == 0){
        /*macroENB-ID*/
        setbits(bytes, 20, v->eNBid->id.macroENB_ID);
    }else{
        /*homeENB-ID*/
        setbits(bytes, 28, v->eNBid->id.homeENB_ID);
    }

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt!=0){
        s1ap_msg(ERROR, 0, "Optional iE-Extensions is present, encoding not included in current version.\n");
    }

    /*Extensions*/
    /*getextension(&extensions, bytes, ext);
    skipextensions(bytes, 1, &extensions);*/

/*
dec_Global-ENB-ID'(Bytes,_) ->
{Ext,Bytes1} = ?RT_PER:getext(Bytes),
{Opt,Bytes2} = ?RT_PER:getoptionals2(Bytes1,1),
%% attribute number 1 with type OCTET STRING
{Term1,Bytes3} = ?RT_PER:decode_octet_string(Bytes2,[{'SizeConstraint',3}]),

%%  attribute number 2 with type ENB-ID
{Term2,Bytes4} = 'dec_ENB-ID'(Bytes3,telltype),

%% attribute number 3 with type SEQUENCE OF
{Term3,Bytes5} = case Opt band (1 bsl 0) of
  _Opt3 when _Opt3 > 0 ->'dec_Global-ENB-ID_iE-Extensions'(Bytes4, telltype);
0 ->{asn1_NOVALUE,Bytes4}

end,
{Extensions,Bytes6} = ?RT_PER:getextension(Ext,Bytes5),
Bytes7= ?RT_PER:skipextensions(Bytes6,1,Extensions)
,
{{'Global-ENB-ID',Term1,Term2,Term3},Bytes7}.
*/
}

void enc_ENBname(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    ENBname_t *v = ie->value;

    encode_known_multiplier_string_PrintableString_withExt(bytes, 1, 150, v->name, strlen((char*)v->name), v->ext);
}

void enc_MMEname(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    MMEname_t *v = ie->value;

    encode_known_multiplier_string_PrintableString_withExt(bytes, 1, 150, v->name, strlen((char*)v->name), v->ext);
}

void enc_SupportedTAs(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){

    uint32_t i;
    SupportedTAs_t *v = (SupportedTAs_t *)ie->value;

    /*Encode length*/
    encode_constrained_number(bytes, v->size, 1, maxnoofTACs);

    /*Encode SupportedTAs_Items*/
    for(i=0 ; i < v->size ; i++){
        enc_SupportedTAs_Item(bytes, v->item[i]);
    }
}

void enc_PagingDRX(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    PagingDRX_t *v =  (PagingDRX_t *)ie->value;

    /*printf("pagingDRX: %s(%u)\n", PagingDRXName[v->pagingDRX], v->pagingDRX);*/
    encode_constrained_number(bytes, v->pagingDRX, 0, 3);
}

void enc_CSGid(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    cSG_id_t *v = (cSG_id_t*)ie->value;
    setbits(bytes, 27, v->id);
    /*printf("enc_CSGid v->id %#x, pos %u\n", v->id, bytes->pos);*/
}

void enc_CSGidList(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    uint16_t i;
    CSG_IdList_t *v = (CSG_IdList_t *)ie->value;

    /*Encode length*/
    encode_constrained_number(bytes, v->size, 1, maxNrOfCSGs);

    /*Encode SupportedTAs_Items*/
    for(i=0 ; i < v->size ; i++){
        enc_CSGidList_Item(bytes, v->item[i]);
    }
}

void enc_ServedPLMNs(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    uint16_t i;
    ServedPLMNs_t *v = (ServedPLMNs_t *)ie->value;

    /*Encode length*/
    encode_constrained_number(bytes, v->size, 1, maxnoofPLMNsPerMME);

    /*Encode PLMNidentity_t*/
    for(i=0 ; i < v->size ; i++){
        enc_PLMNidentity(bytes, v->item[i]);
    }
}

void enc_ServedGUMMEIs(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    uint16_t i;
    ServedGUMMEIs_t *v = (ServedGUMMEIs_t *)ie->value;

    /*Encode length*/
    encode_constrained_number(bytes, v->size, 1, maxnoofRATs);

    /*Encode ServedGUMMEIsItem_t*/
    for(i=0 ; i < v->size ; i++){
        enc_ServedGUMMEIsItem(bytes, v->item[i]);
    }
}

void enc_RelativeMMECapacity(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    RelativeMMECapacity_t *v = (RelativeMMECapacity_t*)ie->value;
    encode_constrained_number(bytes, v->cap, 0, 255);
}

void enc_MMERelaySupportIndicator(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    MMERelaySupportIndicator_t *v = (MMERelaySupportIndicator_t*)ie->value;

    /*Encode extension*/
    setbits(bytes, 1, v->ext);

    if(v->ind == MMERelaySupportIndicator_true && v->ext == 0){
        return; /*Single option No encoded*/
    }
    else{
        encode_small_number(bytes, v->ind);
    }
}

void enc_CriticalityDiagnostics(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    CriticalityDiagnostics_t *v;

    v = (CriticalityDiagnostics_t *)ie->value;

    /*Set Extension flag*/
    setbits(bytes, 1, v->ext);

    /*Set Optionals flag*/
    setbits(bytes, 5, v->opt);

    /*attribute number 1 with type INTEGER*/
    if((v->opt&0x10) == 0x10){
        encode_constrained_number(bytes, v->procedureCode, 0, 255);
    }
    /* attribute number 2 with type ENUMERATED*/
    if((v->opt&0x8) == 0x8){
        encode_constrained_number(bytes, v->triggeringMessage, 0, 2);
    }
    /* attribute number 3 with type ENUMERATED*/
    if((v->opt&0x4) == 0x4){
        encode_constrained_number(bytes, v->procedureCriticality, 0, 2);
    }
    /* attribute number 4 with type  CriticalityDiagnostics-IE-List*/
    if((v->opt&0x2) == 0x2){
        enc_CriticalityDiagnostics_IE_List(bytes, v->iEsCriticalityDiagnostics);
    }
    /* attribute number 5 with type  CriticalityDiagnostics-IE-List*/
    if((v->opt&0x1) == 0x1){
        s1ap_msg(ERROR, 0,"Optional iE-Extensions is present, parsing not included in current version.\n");
    }

    /*Extensions*/
    /*getextension(&extensions, bytes, ext);
    skipextensions(bytes, 1, &extensions);*/
}

void enc_Cause(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    Cause_t *v;
    v = (Cause_t *)ie->value;

    /*Set extension flag and choice*/
    set_choice_ext(bytes, v->choice, 5, v->ext);


    if(v->ext==0){
        /*Set extension flag & set enum value*/
        switch(v->choice){
        case 0: /*CauseRadioNetwork*/
            setbits(bytes, 1, v->cause.radioNetwork.ext);
            if(v->cause.radioNetwork.ext==0){
                encode_constrained_number(bytes, v->cause.radioNetwork.cause.noext, 0, 35);
            }
            else{
                 encode_small_number(bytes, v->cause.radioNetwork.cause.ext);
            }
            break;
        case 1: /*CauseTransport*/
            setbits(bytes, 1, v->cause.transport.ext);
            if(v->cause.transport.ext==0){
                encode_constrained_number(bytes, v->cause.transport.cause.noext, 0, 1);
            }
            else{
                s1ap_msg(WARN, 0, "CauseTransport extension detected. Not available in current version.");
            }
            break;

        case 2: /*CauseNAS*/
            setbits(bytes, 1, v->cause.nas.ext);
            if(v->cause.nas.ext==0){
                encode_constrained_number(bytes, v->cause.nas.cause.noext, 0, 3);
            }
            else{
                encode_small_number(bytes, v->cause.nas.cause.ext);
            }
            break;

        case 3: /*CauseProtocol*/
            setbits(bytes, 1, v->cause.protocol.ext);
            if(v->cause.protocol.ext==0){
                encode_constrained_number(bytes, v->cause.protocol.cause.noext, 0, 6);
            }
            else{
                s1ap_msg(WARN, 0, "CauseProtocol extension detected. Not available in current version.");
            }
            break;

        case 4: /*CauseMisc*/
            setbits(bytes, 1, v->cause.misc.ext);
            if(v->cause.misc.ext==0){
                encode_constrained_number(bytes, v->cause.misc.cause.noext, 0, 5);
            }
            else{
                s1ap_msg(WARN, 0, "CauseMisc extension detected. Not available in current version.");
            }
            break;
        }
    }else{
        s1ap_msg(WARN, 0, "Found extension flag in Cause IE. Current protocol version hasn't any extension. Ignoring.");
    }
}

void enc_TimeToWait(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    TimeToWait_t *v;

    v = (TimeToWait_t *)ie->value;

    /*Set extension flag*/
    setbits(bytes, 1, v->ext);

    if(v->ext==0){
         encode_constrained_number(bytes, v->time.noext, 0, 5);
    }else{
        s1ap_msg(WARN, 0, "Found extension flag in TimeToWait IE. Current protocol version hasn't any extension. Ignoring.");
    }
}

void enc_ENB_UE_S1AP_ID(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    ENB_UE_S1AP_ID_t *v;

    v = (ENB_UE_S1AP_ID_t*)ie->value;

    /*Encode ID*/
    encode_constrained_number(bytes, v->eNB_id, 0, 16777215);
}

void enc_MME_UE_S1AP_ID(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    MME_UE_S1AP_ID_t *v;

    v = (MME_UE_S1AP_ID_t*)ie->value;
    /*Encode ID*/
    encode_constrained_number(bytes, v->mme_id, 0, 4294967295ULL);
}

/*void enc_NAS_PDU(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    NAS_PDU_t *v;

    v = (NAS_PDU_t*)ie->value;

    *//*Encode len*//*
    encode_unconstrained_number(bytes, v->len);

    *//*Encode String*//*
    encode_octet_string(bytes, v->str, v->len);
}*/

void enc_Unconstrained_Octed_String(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    Unconstrained_Octed_String_t *v;

    v = (Unconstrained_Octed_String_t*)ie->value;

    /*Encode len*/
    encode_unconstrained_number(bytes, v->len);

    /*Encode String*/
    encode_octet_string(bytes, v->str, v->len);
}

void enc_TAI(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    TAI_t *v;

    v = (TAI_t*)ie->value;

    /*Set extension flag*/
    setbits(bytes, 1, v->ext);

    /*Set optionals*/
    setbits(bytes, 1, v->opt);

    /*attribute number 1 with type pLMNidentity*/
    enc_PLMNidentity(bytes, v->pLMNidentity);

    /*attribute number 2 with type TAC*/
    encode_octet_string(bytes, v->tAC->s, 2);

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt!=0){
        s1ap_msg(ERROR, 0, "Optional iE-Extensions is present, encoding not included in current version.\n");
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding
        getextension(&extensions, bytes, item->ext);
        skipextensions(bytes, 1, &extensions);*/
    }
}

void enc_EUTRAN_CGI(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){

    EUTRAN_CGI_t *v;

    v = (EUTRAN_CGI_t*)ie->value;


    /*Set extension flag*/
    setbits(bytes, 1, v->ext);

    /*Set optionals*/
    setbits(bytes, 1, v->opt);

    /*attribute number 1 with type pLMNidentity*/
    enc_PLMNidentity(bytes, v->pLMNidentity);

    /*attribute number 2 with type Cell-ID*/
    align_enc(bytes);
    setbits(bytes, 28, v->cell_ID.id);


    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt!=0){
        s1ap_msg(ERROR, 0, "Optional iE-Extensions is present, encoding not included in current version.\n");
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding
        getextension(&extensions, bytes, item->ext);
        skipextensions(bytes, 1, &extensions);*/
    }
}

void enc_RRC_Establishment_Cause(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    RRC_Establishment_Cause_t *v;
    v = (RRC_Establishment_Cause_t *)ie->value;

    setbits(bytes, 1, v->ext);
    if(v->ext==0){
        encode_constrained_number(bytes, v->cause.noext, 0, 4);
    }
    else{
         encode_small_number(bytes, v->cause.ext);
    }
}

void enc_UEAggregateMaximumBitrate (struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    UEAggregateMaximumBitrate_t *v;

    v = (UEAggregateMaximumBitrate_t*)ie->value;


    /*Set extension flag*/
    setbits(bytes, 1, v->ext);

    /*Set optionals*/
    setbits(bytes, 1, v->opt);

    /*attribute number 1 with type BitRate*/
    encode_constrained_number(bytes, v->uEaggregateMaximumBitRateDL.rate, 0, 10000000000ULL);

    /*attribute number 2 with type BitRate*/
    encode_constrained_number(bytes, v->uEaggregateMaximumBitRateUL.rate, 0, 10000000000ULL);


    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt!=0){
        s1ap_msg(ERROR, 0, "Optional iE-Extensions is present, encoding not included in current version.\n");
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding
        getextension(&extensions, bytes, item->ext);
        skipextensions(bytes, 1, &extensions);*/
    }
}

void enc_E_RABToBeSetupItemCtxtSUReq(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    S1AP_PROTOCOL_IES_t fakeie;

    E_RABToBeSetupItemCtxtSUReq_t *v = (E_RABToBeSetupItemCtxtSUReq_t*)ie->value;


    /*Set extension flag*/
    setbits(bytes, 1, v->ext);

    /*Set optionals*/
    setbits(bytes, 2, v->opt>>(8-2));

    /*attribute number 1 with type E_RAB_ID_t*/
    enc_E_RAB_ID(bytes, &(v->eRAB_ID));

    /*attribute number 2 with type E_RABLevelQoSParameters_t*/
    enc_E_RABLevelQoSParameters(bytes, v->eRABlevelQoSParameters);


    /*attribute number 3 with type TransportLayerAddress_t*/
    enc_TransportLayerAddress(bytes, v->transportLayerAddress);

    /*attribute number 4 with type GTP_TEID_t*/
    encode_octet_string(bytes, v->gTP_TEID.teid, 4);

    /*attribute number 5 with type GTP_TEID_t*/
    if( (v->opt&0x80)==0x80 ){
        fakeie.value = v->nAS_PDU;
        enc_Unconstrained_Octed_String(bytes, &fakeie);
    }

    /* attribute number 6 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if((v->opt&0x40)==0x40){
        enc_protocolIEs(bytes, (S1AP_PROTOCOL_IES_t*)v->iEext);
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding
        getextension(&extensions, bytes, item->ext);
        skipextensions(bytes, 1, &extensions);*/
    }
}

void enc_E_RABToBeSetupListCtxtSUReq(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    uint8_t i;
    E_RABToBeSetupListCtxtSUReq_t *v = (E_RABToBeSetupListCtxtSUReq_t *)ie->value;

    /*Encode length*/
    encode_constrained_number(bytes, v->size, 1, maxNrOfERABs);

    /*Encode E_RABToBeSetupItemCtxtSUReq_t*/
    for(i=0 ; i < v->size ; i++){
        enc_protocolIEs(bytes, (S1AP_PROTOCOL_IES_t*)v->item[i]);
    }
}

void enc_SecurityKey(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    SecurityKey_t *v = (SecurityKey_t *)ie->value;

    /*Encode Key*/
    setoctets(bytes, 32, v->key);
}

void enc_SubscriberProfileIDforRFP(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    SubscriberProfileIDforRFP_t *v = (SubscriberProfileIDforRFP_t *)ie->value;

    /*Encode*/
    encode_constrained_number(bytes, v->subscriberProfileIDforRFP, 1, 256);

}

void enc_UESecurityCapabilities(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){

    UESecurityCapabilities_t *v = (UESecurityCapabilities_t *)ie->value;

    /*Set extension flag*/
    setbits(bytes, 1, v->ext);

    /*Set optionals*/
    setbits(bytes, 1, v->opt);

    /*Encode encryptionAlgorithms*/
    /*Set ext*/
    setbits(bytes, 1, v->encryptionAlgorithms.ext);
    if(v->encryptionAlgorithms.ext==0 ){
        setbits(bytes, 16, v->encryptionAlgorithms.v);
    }else{
        s1ap_msg(WARN, 0, "encryptionAlgorithms extension encoder not implemented yet.");
    }

    /*Encode integrityProtectionAlgorithms*/
    /*Set ext*/
    setbits(bytes, 1, v->integrityProtectionAlgorithms.ext);
    if(v->integrityProtectionAlgorithms.ext==0 ){
        setbits(bytes, 16, v->integrityProtectionAlgorithms.v);
    }else{
        s1ap_msg(WARN, 0, "integrityProtectionAlgorithms extension encoder not implemented yet.");
    }

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==0x01){
        enc_protocolIEs(bytes, (S1AP_PROTOCOL_IES_t*)v->iEext);
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding
        getextension(&extensions, bytes, item->ext);
        skipextensions(bytes, 1, &extensions);*/
    }
}

void enc_UERadioCapability(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    UERadioCapability_t *v;

    v = (UERadioCapability_t*)ie->value;

    /*Encode len*/
    encode_unconstrained_number(bytes, v->len);

    /*Encode String*/
    encode_octet_string(bytes, v->str, v->len);
}

void enc_UE_S1AP_IDs(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    S1AP_PROTOCOL_IES_t fakeie;
    UE_S1AP_IDs_t *v;

    v = (UE_S1AP_IDs_t*)ie->value;

    /*attribute number 2 eNB-ID with type ENB-ID*/
    set_choice_ext(bytes, v->choice, 2, v->ext);

    if(v->ext!=0 || v->choice>1){
        s1ap_msg(ERROR, 0, "Extensions not implemented or choice not correct");
        return;
    }

    //align_enc(bytes);
    if(v->choice == 0){
        /* UE-S1AP-ID-pair*/
        enc_UE_S1AP_ID_pair(bytes, v->uE_S1AP_ID.uE_S1AP_ID_pair);
    }else{
        /*MME-UE-S1AP-ID*/
        fakeie.value = v->uE_S1AP_ID.mME_UE_S1AP_ID;
        enc_MME_UE_S1AP_ID(bytes, &fakeie);
    }
}

void enc_ENB_StatusTransfer_TransparentContainer(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    ENB_StatusTransfer_TransparentContainer_t *v;
    v = (ENB_StatusTransfer_TransparentContainer_t*)ie->value;

    /*Set extension flag*/
    setbits(bytes, 1, v->ext);

    /*Set optionals*/
    setbits(bytes, 1, v->opt);

    /*Encode Bearers-SubjectToStatusTransferList*/
    enc_Bearers_SubjectToStatusTransferList(bytes, v->bearers_SubjectToStatusTransferList);

    /* attribute number 2 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==0x01){
        enc_protocolIEs(bytes, (S1AP_PROTOCOL_IES_t*)v->iEext);
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding*/
    }
}

void enc_Bearers_SubjectToStatusTransfer_Item(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    Bearers_SubjectToStatusTransfer_Item_t *v = (Bearers_SubjectToStatusTransfer_Item_t*)ie->value;

    /*Set extension flag*/
    setbits(bytes, 1, v->ext);

    /*Set optionals*/
    setbits(bytes, 2, v->opt>>(8-2));

    /*attribute number 1 with type E_RAB_ID_t*/
    enc_E_RAB_ID(bytes, &(v->eRAB_ID));

    /* attribute number 3  with type COUNTvalue */
    enc_COUNTvalue(bytes, v->uL_COUNTvalue);
    /* attribute number 4  with type COUNTvalue */
    enc_COUNTvalue(bytes, v->dL_COUNTvalue );

    /* attribute number 5  with type ReceiveStatusofULPDCPSDUs */
    if((v->opt&0x80)==0x80){
     /*v->receiveStatusofULPDCPSDUs*/
        s1ap_msg(WARN, 0, "Optional ReceiveStatusofULPDCPSDUs is present, encoding not included in current version.");
    }

    /* attribute number 6 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if((v->opt&0x40)==0x40){
        enc_protocolIEs(bytes, (S1AP_PROTOCOL_IES_t*)v->iEext);
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding*/
    }
}

void enc_E_RABItem(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    S1AP_PROTOCOL_IES_t fakeie;
    E_RABItem_t *v=(E_RABItem_t *)ie->value;

    /*Set extension flag*/
    setbits(bytes, 1, v->ext);

    /*Set optionals*/
    setbits(bytes, 1, v->opt);

    /*attribute number 1 with type E_RAB_ID_t*/
    enc_E_RAB_ID(bytes, &(v->eRAB_ID));

    /*attribute number 2 with type Cause*/
    fakeie.value = v->cause;
    enc_Cause(bytes, &fakeie);

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==0x01){
        enc_protocolIEs(bytes, (S1AP_PROTOCOL_IES_t*)v->iEext);
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding*/
    }
}

void enc_E_RABList(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    uint8_t i;
    E_RABList_t *v =  (E_RABList_t*)ie->value;

    /*Encode length*/
    encode_constrained_number(bytes, v->size, 1, maxNrOfERABs);

    /*Encode E_RABList_t, type E_RABItem*/
    for(i=0 ; i < v->size ; i++){
        enc_protocolIEs(bytes, v->item[i]);
    }
}

void enc_E_RABToBeModifiedItemBearerModReq(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    S1AP_PROTOCOL_IES_t fakeie;
    E_RABToBeModifiedItemBearerModReq_t *v=(E_RABToBeModifiedItemBearerModReq_t *)ie->value;

    /*Set extension flag*/
    setbits(bytes, 1, v->ext);

    /*Set optionals*/
    setbits(bytes, 1, v->opt);

    /*attribute number 1 with type E_RAB_ID_t*/
    enc_E_RAB_ID(bytes, &(v->eRAB_ID));

    /*attribute number 2 with type E_RABLevelQoSParameters_t*/
    enc_E_RABLevelQoSParameters(bytes, v->eRABLevelQoSParameters);

    /*attribute number 3 with type NAS_PDU_t*/
    fakeie.value = v->nAS_PDU;
    enc_Unconstrained_Octed_String(bytes, &fakeie);

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==0x01){
        enc_protocolIEs(bytes, (S1AP_PROTOCOL_IES_t*)v->iEext);
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding*/
    }
}

void enc_E_RABToBeModifiedListBearerModReq(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    uint8_t i;
    E_RABToBeModifiedListBearerModReq_t *v =  (E_RABToBeModifiedListBearerModReq_t*)ie->value;

    /*Encode length*/
    encode_constrained_number(bytes, v->size, 1, maxNrOfERABs);

    /*Encode E_RABToBeModifiedListBearerModReq_t, type E-RABToBeModifiedItemBearerModReq*/
    for(i=0 ; i < v->size ; i++){
        enc_protocolIEs(bytes, v->item[i]);
    }
}

void enc_E_RABToBeSetupItemBearerSUReq(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    S1AP_PROTOCOL_IES_t fakeie;
    E_RABToBeSetupItemBearerSUReq_t *v=(E_RABToBeSetupItemBearerSUReq_t *)ie->value;

    /*Set extension flag*/
    setbits(bytes, 1, v->ext);

    /*Set optionals*/
    setbits(bytes, 1, v->opt);

    /*attribute number 1 with type E_RAB_ID_t*/
    enc_E_RAB_ID(bytes, &(v->eRAB_ID));

    /*attribute number 2 with type E_RABLevelQoSParameters_t*/
    enc_E_RABLevelQoSParameters(bytes, v->eRABLevelQoSParameters);

    /*attribute number 3 with type TransportLayerAddress*/
    enc_TransportLayerAddress(bytes, v->transportLayerAddress);

    /*attribute number 4 with type GTP_TEID*/
    encode_octet_string(bytes, v->gTP_TEID.teid, 4);

    /*attribute number 5 with type NAS_PDU_t*/
    fakeie.value = v->nAS_PDU;
    enc_Unconstrained_Octed_String(bytes, &fakeie);

    /* attribute number 6 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==0x01){
        enc_protocolIEs(bytes, (S1AP_PROTOCOL_IES_t*)v->iEext);
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding*/
    }
}

void enc_E_RABToBeSetupListBearerSUReq(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    uint8_t i;
    E_RABToBeModifiedListBearerModReq_t *v =  (E_RABToBeModifiedListBearerModReq_t*)ie->value;

    /*Encode length*/
    encode_constrained_number(bytes, v->size, 1, maxNrOfERABs);

    /*Encode E_RABToBeModifiedListBearerModReq_t, type E-RABToBeModifiedItemBearerModReq*/
    for(i=0 ; i < v->size ; i++){
        enc_protocolIEs(bytes, v->item[i]);
    }
}

void enc_E_RABSetupItemBearerSURes(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    E_RABSetupItemBearerSURes_t *v=(E_RABSetupItemBearerSURes_t *)ie->value;

    /*Set extension flag*/
    setbits(bytes, 1, v->ext);

    /*Set optionals*/
    setbits(bytes, 1, v->opt);

    /*attribute number 1 with type E_RAB_ID_t*/
    enc_E_RAB_ID(bytes, &(v->eRAB_ID));

    /*attribute number 2 with type TransportLayerAddress*/
    enc_TransportLayerAddress(bytes, v->transportLayerAddress);

    /*attribute number 3 with type GTP_TEID*/
    encode_octet_string(bytes, v->gTP_TEID.teid, 4);

    /* attribute number 4 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==0x01){
        enc_protocolIEs(bytes, (S1AP_PROTOCOL_IES_t*)v->iEext);
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding*/
    }
}

void enc_E_RABSetupListBearerSURes(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    uint8_t i;
    E_RABSetupListBearerSURes_t *v =  (E_RABSetupListBearerSURes_t*)ie->value;

    /*Encode length*/
    encode_constrained_number(bytes, v->size, 1, maxNrOfERABs);

    /*Encode E_RABSetupListBearerSURes, type E-E_RABSetupItemBearerSURes*/
    for(i=0 ; i < v->size ; i++){
        enc_protocolIEs(bytes, v->item[i]);
    }
}

void enc_HandoverType(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){

    HandoverType_t *v =  (HandoverType_t*)ie->value;
    setbits(bytes, 1, v->ext);
    if(v->ext==0){
        encode_constrained_number(bytes, v->ht.noext, 0, 4);
    }
    else{
        s1ap_msg(WARN, 0, "Handover Type with extension detected. Current version doesn't support it.");
        /*encode_small_number(bytes, v->ht.ext);*/
    }
}

void enc_LAI(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    LAI_t *v = (LAI_t*)ie->value;

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Set extension flag*/
    setbits(bytes, 1, v->ext);

    /*Set optionals*/
    setbits(bytes, 1, v->opt);

    /*attribute number 1 (pLMNidentity) with type OCTET STRING */
    enc_PLMNidentity(bytes, v->pLMNidentity);

    /*attribute number 2 (pLMNidentity) with type OCTET STRING */
    encode_octet_string(bytes, v->lAC.s, 2);

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==0x01){
        enc_protocolIEs(bytes, (S1AP_PROTOCOL_IES_t*)v->iEext);
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding*/
    }
}

void enc_TargetID(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    TargetID_t *v;

    v = (TargetID_t*)ie->value;

    set_choice_ext(bytes, v->choice, 3, v->ext);

    if(v->ext!=0 || v->choice>1){
        s1ap_msg(ERROR, 0, "Extensions not implemented or choice not correct");
        return;
    }

    //align_enc(bytes);
    if(v->choice == 0){
        /* TargeteNB-ID*/
        enc_TargeteNB_ID(bytes, v->targetID.targeteNB_ID);
    }else if(v->choice == 1){
        /*TargetRNC-ID*/
        enc_TargetRNC_ID(bytes, v->targetID.targetRNC_ID);
    }else if(v->choice == 1){
        /*CGI*/
        enc_CGI(bytes, v->targetID.cGI);
    }
}

void enc_Direct_Forwarding_Path_Availability(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    Direct_Forwarding_Path_Availability_t *v = (Direct_Forwarding_Path_Availability_t*)ie->value;

    /*Encode extension*/
    setbits(bytes, 1, v->ext);

    if(v->ind == Direct_Forwarding_Path_Availability_directPathAvailable && v->ext == 0){
        return; /*Single option No encoded*/
    }
    else{
        encode_small_number(bytes, v->ind);
    }
}

void enc_E_RAB_IE_ContainerList(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    uint32_t i;
    ProtocolIE_Container_t *v = (ProtocolIE_Container_t *)ie->value;

    /*attribute number 1 with type SEQUENCE OF*/
    encode_constrained_number(bytes, v->size, 1, maxNrOfERABs);

    /*Encode IEs*/
    for(i=0; i<v->size;i++){
        enc_protocolIEs(bytes, v->elem[i]);

    }
}

void enc_E_RABToBeSetupItemHOReq(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    E_RABToBeSetupItemHOReq_t *v = (E_RABToBeSetupItemHOReq_t*)ie->value;

    /*Set extension flag*/
    setbits(bytes, 1, v->ext);

    /*Set optionals*/
    setbits(bytes, 1, v->opt);

    /*attribute number 1 with type E_RAB_ID_t*/
    enc_E_RAB_ID(bytes, &(v->eRAB_ID));

    /*attribute number 2 with type TransportLayerAddress_t*/
    enc_TransportLayerAddress(bytes, v->transportLayerAddress);

    /*attribute number 3 with type GTP_TEID_t*/
    encode_octet_string(bytes, v->gTP_TEID.teid, 4);

    /*attribute number 4 with type E_RABLevelQoSParameters_t*/
    enc_E_RABLevelQoSParameters(bytes, v->eRABLevelQoSParameters);

    /* attribute number 5 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==0x01){
        enc_protocolIEs(bytes, (S1AP_PROTOCOL_IES_t*)v->iEext);
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding
        getextension(&extensions, bytes, item->ext);
        skipextensions(bytes, 1, &extensions);*/
    }
}

void enc_SecurityContext(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    S1AP_PROTOCOL_IES_t fakeie;
    SecurityContext_t *v = (SecurityContext_t*)ie->value;

    /*Set extension flag*/
     setbits(bytes, 1, v->ext);

     /*Set optionals*/
     setbits(bytes, 1, v->opt);

    /*attribute number 1 with type INTEGER (0 .. 7)*/
    encode_constrained_number(bytes, v->nextHopChainingCount, 0, 7);

    /*attribute number 2 with type SecurityKey*/
    fakeie.value = v->nextHopParameter;
    enc_SecurityKey(bytes, &fakeie);

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if((v->opt&0x40)==0x40){
        enc_protocolIEs(bytes, (S1AP_PROTOCOL_IES_t*)v->iEext);
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding
        getextension(&extensions, bytes, item->ext);
        skipextensions(bytes, 1, &extensions);*/
    }
}

void enc_E_RABAdmittedItem(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    E_RABAdmittedItem_t *v = (E_RABAdmittedItem_t*)ie->value;

    /*Set extension flag*/
    setbits(bytes, 1, v->ext);

    /*Set optionals*/
    setbits(bytes, 5, v->opt>>(8-5));

    /*attribute number 1 with type E_RAB_ID_t*/
    /*printf("test enc v->eRAB_ID %u ext %u\n", v->eRAB_ID.id, v->eRAB_ID.ext);*/
    enc_E_RAB_ID(bytes, &(v->eRAB_ID));

    /*attribute number 2 with type TransportLayerAddress_t*/
    enc_TransportLayerAddress(bytes, v->transportLayerAddress);

    /*attribute number 3 with type GTP_TEID_t*/
    encode_octet_string(bytes, v->gTP_TEID.teid, 4);

    /*attribute number 4 with type TransportLayerAddress*/
     if((v->opt&0x80)==0x80){
         enc_TransportLayerAddress(bytes, v->dL_transportLayerAddress);
     }

     /*attribute number 5 with type GTP_TEID*/
     if((v->opt&0x40)==0x40){
         encode_octet_string(bytes, v->dL_GTP_TEID.teid, 4);
     }

     /*attribute number 6 with type TransportLayerAddress*/
     if((v->opt&0x20)==0x20){
         enc_TransportLayerAddress(bytes, v->uL_transportLayerAddress);
     }

     /*attribute number 7 with type GTP_TEID*/
     if((v->opt&0x10)==0x10){
         encode_octet_string(bytes, v->uL_GTP_TEID.teid, 4);
     }

    /* attribute number 8 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if((v->opt&0x08)==0x08){
        enc_protocolIEs(bytes, (S1AP_PROTOCOL_IES_t*)v->iEext);
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding
        getextension(&extensions, bytes, item->ext);
        skipextensions(bytes, 1, &extensions);*/
    }
}

void enc_E_RABDataForwardingItem(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    E_RABDataForwardingItem_t *v = (E_RABDataForwardingItem_t*)ie->value;

    /*Set extension flag*/
    setbits(bytes, 1, v->ext);

    /*Set optionals*/
    setbits(bytes, 5, v->opt>>(8-5));

    /*attribute number 1 with type E_RAB_ID_t*/
    //printf("test enc v->eRAB_ID %u ext %u\n", v->eRAB_ID.id, v->eRAB_ID.ext);
    enc_E_RAB_ID(bytes, &(v->eRAB_ID));

    /*attribute number 2 with type TransportLayerAddress*/
     if((v->opt&0x80)==0x80){
         enc_TransportLayerAddress(bytes, v->dL_transportLayerAddress);
     }

     /*attribute number 3 with type GTP_TEID*/
     if((v->opt&0x40)==0x40){
         encode_octet_string(bytes, v->dL_GTP_TEID.teid, 4);
     }

     /*attribute number 4 with type TransportLayerAddress*/
     if((v->opt&0x20)==0x20){
         enc_TransportLayerAddress(bytes, v->uL_transportLayerAddress);
     }

     /*attribute number 5 with type GTP_TEID*/
     if((v->opt&0x10)==0x10){
         encode_octet_string(bytes, v->uL_GTP_TEID.teid, 4);
     }

    /* attribute number 6 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if((v->opt&0x08)==0x08){
        enc_protocolIEs(bytes, (S1AP_PROTOCOL_IES_t*)v->iEext);
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding
        getextension(&extensions, bytes, item->ext);
        skipextensions(bytes, 1, &extensions);*/
    }
}


void enc_S_TMSI(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    S_TMSI_t *v = (S_TMSI_t*)ie->value;

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Set extension flag*/
    setbits(bytes, 1, v->ext);

    /*Set optionals*/
    setbits(bytes, 1, v->opt);

    /*attribute number 1 (mMEC) with type MME-Code */
    enc_MME_Code(bytes, v->mMEC);

    /*attribute number 2 (m_TMSI) with type OCTET STRING */
    encode_octet_string(bytes, v->m_TMSI.s, 4);

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==0x01){
        enc_protocolIEs(bytes, (S1AP_PROTOCOL_IES_t*)v->iEext);
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, encoding not included in current version.\n");
        /*TODO extensions encoding*/
    }
}


void enc_ResetType(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){
    ResetType_t *v;

    v = (ResetType_t*)ie->value;

    set_choice_ext(bytes, v->choice, 2, v->ext);

    if(v->ext!=0 || v->choice>1){
        s1ap_msg(ERROR, 0, "Extensions not implemented or choice not correct");
        return;
    }

    //align_enc(bytes);
    if(v->choice == 0){
        /* ResetAll_e */
        encode_constrained_number(bytes, v->type.s1_Interface, 0, 1);
    }else if(v->choice == 1){
        /* UE_associatedLogicalS1_ConnectionListRes_t */
        enc_UE_associatedLogicalS1_ConnectionListRes(bytes, v->type.partOfS1_Interface);
    }
}


const getEncS1AP_IE getenc_S1AP_IE[] = {
        enc_MME_UE_S1AP_ID,/*"id-MME-UE-S1AP-ID"*/
        enc_HandoverType,/*"id-HandoverType"*/
        enc_Cause,/*"id-Cause"*/
        NULL,/*"id-SourceID"*/
        enc_TargetID,/*"id-TargetID"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-undefined"*/
        enc_ENB_UE_S1AP_ID,/*"id-eNB-UE-S1AP-ID"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-undefined"*/
        enc_E_RABList,/*"id-E-RABSubjecttoDataForwardingList"*/
        enc_E_RABList,/*"id-E-RABtoReleaseListHOCmd"*/
        enc_E_RABDataForwardingItem,/*"id-E-RABDataForwardingItem"*/
        NULL,/*"id-E-RABReleaseItemBearerRelComp"*/
        enc_E_RABToBeSetupListBearerSUReq,/*"id-E-RABToBeSetupListBearerSUReq"*/
        enc_E_RABToBeSetupItemBearerSUReq,/*"id-E-RABToBeSetupItemBearerSUReq"*/
        enc_E_RABList,/*"id-E-RABAdmittedList"*/
        NULL,/*"id-E-RABFailedToSetupListHOReqAck"*/
        enc_E_RABAdmittedItem,/*"id-E-RABAdmittedItem"*/
        NULL,/*"id-E-RABFailedtoSetupItemHOReqAck"*/
        enc_E_RABSetupListBearerSURes,/*"id-E-RABToBeSwitchedDLList"*/
        enc_E_RABSetupItemBearerSURes,/*"id-E-RABToBeSwitchedDLItem"*/
        enc_E_RABToBeSetupListCtxtSUReq,/*"id-E-RABToBeSetupListCtxtSUReq"*/
        NULL,/*"id-TraceActivation"*/
        enc_Unconstrained_Octed_String,/*"id-NAS-PDU"*/
        enc_E_RABToBeSetupItemHOReq,/*"id-E-RABToBeSetupItemHOReq"*/
        enc_E_RABSetupListBearerSURes,/*"id-E-RABSetupListBearerSURes"*/
        enc_E_RABList,/*"id-E-RABFailedToSetupListBearerSURes"*/
        enc_E_RABToBeModifiedListBearerModReq,/*"id-E-RABToBeModifiedListBearerModReq"*/
        NULL,/*"id-E-RABModifyListBearerModRes"*/
        enc_E_RABList,/*"id-E-RABFailedToModifyList"*/
        enc_E_RABList,/*"id-E-RABToBeReleasedList"*/
        enc_E_RABList,/*"id-E-RABFailedToReleaseList"*/
        enc_E_RABItem,/*"id-E-RABItem"*/
        enc_E_RABToBeModifiedItemBearerModReq,/*"id-E-RABToBeModifiedItemBearerModReq"*/
        NULL,/*"id-E-RABModifyItemBearerModRes"*/
        NULL,/*"id-E-RABReleaseItem"*/
        enc_E_RABSetupItemBearerSURes,/*"id-E-RABSetupItemBearerSURes"*/
        enc_SecurityContext,/*"id-SecurityContext"*/
        NULL,/*"id-HandoverRestrictionList"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-UEPagingID"*/
        NULL,/*"id-pagingDRX"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-TAIList"*/
        NULL,/*"id-TAIItem"*/
        enc_E_RABList,/*"id-E-RABFailedToSetupListCtxtSURes"*/
        NULL,/*"id-E-RABReleaseItemHOCmd"*/
        enc_E_RABSetupItemBearerSURes,/*"id-E-RABSetupItemCtxtSURes"*/
        enc_E_RABSetupListBearerSURes,/*"id-E-RABSetupListCtxtSURes"*/
        enc_E_RABToBeSetupItemCtxtSUReq,/*"id-E-RABToBeSetupItemCtxtSUReq"*/
        enc_E_RABToBeSetupListCtxtSUReq,/*"id-E-RABToBeSetupListHOReq"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-GERANtoLTEHOInformationRes"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-UTRANtoLTEHOInformationRes"*/
        NULL,/*"id-CriticalityDiagnostics"*/
        enc_Global_ENB_ID,/*"id-Global-ENB-ID"*/
        enc_ENBname,/*"id-eNBname"*/
        enc_MMEname,/*"id-MMEname"*/
        NULL,/*"id-undefined"*/
        enc_ServedPLMNs,/*"id-ServedPLMNs"*/
        enc_SupportedTAs,/*"id-SupportedTAs"*/
        enc_TimeToWait,/*"id-TimeToWait"*/
        enc_UEAggregateMaximumBitrate,/*"id-uEaggregateMaximumBitrate"*/
        enc_TAI,/*"id-TAI"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-E-RABReleaseListBearerRelComp"*/
        NULL,/*"id-cdma2000PDU"*/
        NULL,/*"id-cdma2000RATType"*/
        NULL,/*"id-cdma2000SectorID"*/
        enc_SecurityKey,/*"id-SecurityKey"*/
        enc_UERadioCapability,/*"id-UERadioCapability"*/
        NULL,/*"id-GUMMEI-ID"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-E-RABInformationListItem"*/
        enc_Direct_Forwarding_Path_Availability,/*"id-Direct-Forwarding-Path-Availability"*/
        NULL,/*"id-UEIdentityIndexValue"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-cdma2000HOStatus"*/
        NULL,/*"id-cdma2000HORequiredIndication"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-E-UTRAN-Trace-ID"*/
        enc_RelativeMMECapacity,/*"id-RelativeMMECapacity"*/
        enc_MME_UE_S1AP_ID,/*"id-SourceMME-UE-S1AP-ID"*/
        enc_Bearers_SubjectToStatusTransfer_Item,/*"id-Bearers-SubjectToStatusTransfer-Item"*/
        enc_ENB_StatusTransfer_TransparentContainer,/*"id-eNB-StatusTransfer-TransparentContainer"*/
        enc_UE_associatedLogicalS1_ConnectionItem,/*"id-UE-associatedLogicalS1-ConnectionItem"*/
        enc_ResetType,/*"id-ResetType"*/
        enc_UE_associatedLogicalS1_ConnectionListResAck,/*"id-UE-associatedLogicalS1-ConnectionListResAck"*/
        enc_E_RABSetupItemBearerSURes,/*"id-E-RABToBeSwitchedULItem"*/
        enc_E_RABSetupListBearerSURes,/*"id-E-RABToBeSwitchedULList"*/
        enc_S_TMSI,/*"id-S-TMSI"*/
        NULL,/*"id-cdma2000OneXRAND"*/
        NULL,/*"id-RequestType"*/
        enc_UE_S1AP_IDs,/*"id-UE-S1AP-IDs"*/
        enc_EUTRAN_CGI,/*"id-EUTRAN-CGI"*/
        NULL,/*"id-OverloadResponse"*/
        NULL,/*"id-cdma2000OneXSRVCCInfo"*/
        NULL,/*"id-E-RABFailedToBeReleasedList"*/
        enc_Unconstrained_Octed_String,/*"id-Source-ToTarget-TransparentContainer"*/
        enc_ServedGUMMEIs,/*"id-ServedGUMMEIs"*/
        enc_SubscriberProfileIDforRFP,/*"id-SubscriberProfileIDforRFP"*/
        enc_UESecurityCapabilities,/*"id-UESecurityCapabilities"*/
        NULL,/*"id-CSFallbackIndicator"*/
        NULL,/*"id-CNDomain"*/
        enc_E_RABList,/*"id-E-RABReleasedList"*/
        NULL,/*"id-MessageIdentifier"*/
        NULL,/*"id-SerialNumber"*/
        NULL,/*"id-WarningAreaList"*/
        NULL,/*"id-RepetitionPeriod"*/
        NULL,/*"id-NumberofBroadcastRequest"*/
        NULL,/*"id-WarningType"*/
        NULL,/*"id-WarningSecurityInfo"*/
        NULL,/*"id-DataCodingScheme"*/
        NULL,/*"id-WarningMessageContents"*/
        NULL,/*"id-BroadcastCompletedAreaList"*/
        NULL,/*"id-Inter-SystemInformationTransferTypeEDT"*/
        NULL,/*"id-Inter-SystemInformationTransferTypeMDT"*/
        enc_Unconstrained_Octed_String,/*"id-Target-ToSource-TransparentContainer"*/
        NULL,/*"id-SRVCCOperationPossible"*/
        NULL,/*"id-SRVCCHOIndication"*/
        NULL,/*"id-NAS-DownlinkCount"*/
        enc_CSGid,/*"id-CSG-Id"*/
        enc_CSGidList,/*"id-CSG-IdList"*/
        NULL,/*"id-SONConfigurationTransferECT"*/
        NULL,/*"id-SONConfigurationTransferMCT"*/
        NULL,/*"id-TraceCollectionEntityIPAddress"*/
        NULL,/*"id-MSClassmark2"*/
        NULL,/*"id-MSClassmark3"*/
        enc_RRC_Establishment_Cause,/*"id-RRC-Establishment-Cause"*/
        enc_Unconstrained_Octed_String,/*"id-NASSecurityParametersfromE-UTRAN"*/
        enc_Unconstrained_Octed_String,/*"id-NASSecurityParameterstoE-UTRAN"*/
        enc_PagingDRX,/*"id-DefaultPagingDRX"*/
        enc_Unconstrained_Octed_String,/*"id-Source-ToTarget-TransparentContainer-Secondary"*/
        enc_Unconstrained_Octed_String,/*"id-Target-ToSource-TransparentContainer-Secondary"*/
        NULL,/*"id-EUTRANRoundTripDelayEstimationInfo"*/
        NULL,/*"id-BroadcastCancelledAreaList"*/
        NULL,/*"id-ConcurrentWarningMessageIndicator"*/
        NULL,/*"id-Data-Forwarding-Not-Possible"*/
        NULL,/*"id-ExtendedRepetitionPeriod"*/
        NULL,/*"id-CellAccessMode"*/
        NULL,/*"id-CSGMembershipStatus"*/
        NULL,/*"id-LPPa-PDU"*/
        NULL,/*"id-Routing-ID"*/
        NULL,/*"id-Time-Synchronization-Info"*/
        NULL,/*"id-PS-ServiceNotAvailable"*/
        NULL,/*"id-PagingPriority"*/
        NULL,/*"id-x2TNLConfigurationInfo"*/
        NULL,/*"id-eNBX2ExtendedTransportLayerAddresses"*/
        NULL,/*"id-GUMMEIList"*/
        NULL,/*"id-GW-TransportLayerAddress"*/
        NULL,/*"id-Correlation-ID"*/
        NULL,/*"id-SourceMME-GUMMEI"*/
        NULL,/*"id-MME-UE-S1AP-ID-2"*/
        enc_LAI,/*"id-RegisteredLAI"*/
        NULL,/*"id-RelayNode-Indicator"*/
        NULL,/*"id-TrafficLoadReductionIndication"*/
        NULL,/*"id-MDTConfiguration"*/
        enc_MMERelaySupportIndicator,/*"id-MMERelaySupportIndicator"*/
        NULL,/*"id-GWContextReleaseIndication"*/
        NULL,/*"id-ManagementBasedMDTAllowed"*/
        NULL,/*"id-PrivacyIndicator"*/
        NULL,/*"id-Time-UE-StayedInCell-EnhancedGranularity"*/
        NULL,/*"id-HO-Cause"*/
        NULL,/*"id-VoiceSupportMatchIndicator"*/
        NULL,/*"id-GUMMEIType"*/
};

void enc_protocolIEs(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t *ie){

    getEncS1AP_IE ieEnc;
    struct BinaryData Tmpterm1;
    uint8_t buffer[MAXDATABYTES];

    memset(buffer, 0, MAXDATABYTES);
    Tmpterm1.data=buffer;
    Tmpterm1.offset=buffer;
    Tmpterm1.length=0;
    Tmpterm1.pos=0;

    if(!ie){
        s1ap_msg(ERROR, 0, "Error IE not allocated");
    }

    /*attribute number 1 with type id*/
    encode_constrained_number(bytes, ie->id, 0, 65535);
    /*attribute number 2 with type criticality*/
    encode_constrained_number(bytes, ie->criticality, 0, 2);

    /*attribute number 3 with type Value (Open Type)*/
    ieEnc = getenc_S1AP_IE[ie->id];
    if(ieEnc == NULL){
        s1ap_msg(ERROR, 0, "Function encoder for IE %s(%u) not found", IEName[ie->id], ie->id);
        return ;
    }
    ieEnc(&Tmpterm1, ie);

    /*printf("IE #%u Buffer:\n", ie->id);*/
    /*printf_buffer(Tmpterm1.data, Tmpterm1.lenght);*/
    encode_open_type(bytes, &Tmpterm1);
    /*printf_buffer(bytes->data, bytes->lenght);*/


}
