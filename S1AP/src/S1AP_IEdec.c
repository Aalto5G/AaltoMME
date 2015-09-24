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


#include "rt_per_bin.h"
#include "S1AP_IEdec.h"
#include "S1AP_IE.h"
#include "S1APlog.h"

/* Prototypes required*/
void dec_CSGid(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes);
void dec_servedPLMNs(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes);
void dec_ENB_UE_S1AP_ID(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes);
void dec_MME_UE_S1AP_ID(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes);
void dec_Global_ENB_ID(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes);
void dec_TAI(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes);
void dec_LAI(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes);

/* ******************** IE decoder functions ******************** */

PLMNidentity_t *dec_PLMNidentity(struct BinaryData *bytes){
    PLMNidentity_t *id;
    id = new_PLMNidentity();
    decode_octet_string(id->tbc.s, bytes, 3);
    plmnId_tbcd2MccMnc(id);

    return id;
}

BPLMNs_t *dec_BPLMNs(struct BinaryData *bytes){
    BPLMNs_t *v;
    PLMNidentity_t *item;
    uint8_t i;

    v=new_BPLMNs();

    /*Decode length*/
    v->size = decode_constrained_number(bytes, 1, maxnoofBPLMNs);
    /*printf("\t\tdec_BPLMNs() size = %u\n", v->size);*/

    /*Decode SupportedTAs_Items*/
    for(i=0 ; i < v->size ; i++){
        item=NULL;
        item = dec_PLMNidentity(bytes);
        v->addPLMNid(v, item);
    }
    return v;

/*
{Num,Bytes1} = ?RT_PER:decode_length(Bytes,{1,6}),
'dec_BPLMNs_components'(Num, Bytes1, telltype, []).
'dec_BPLMNs_components'(0, Bytes, _, Acc) ->
   {lists:reverse(Acc), Bytes};
'dec_BPLMNs_components'(Num, Bytes, _, Acc) ->
   {Term,Remain} = ?RT_PER:decode_octet_string(Bytes,[{'SizeConstraint',3}]),
   'dec_BPLMNs_components'(Num-1, Remain, telltype, [Term|Acc]).
*/
}

SupportedTAs_Item_t * dec_SupportedTAs_Item(struct BinaryData *bytes){
    SupportedTAs_Item_t *item;
    struct BinaryData extensions;

    item = new_SupportedTAs_Item();
    /*Get extension flag*/
    getbit(bytes, &(item->ext));

    /*Get optionals*/
    getbit(bytes, &(item->opt));

    /*attribute number 1 with type OCTET STRING*/
    /*{Term1,Bytes3} = ?RT_PER:decode_octet_string(Bytes2,[{'SizeConstraint',2}])*/
    /*printf("dec_SupportedTAs_Item TAC %#x %#x\n", item->tAC->s[0], item->tAC->s[1]);*/
    decode_octet_string(item->tAC->s, bytes, 2);
    /*printf("dec_SupportedTAs_Item TAC %#x %#x\n", item->tAC->s[0], item->tAC->s[1]);*/

    /*attribute number 2 with type BPLMNs*/
    item->broadcastPLMNs = dec_BPLMNs(bytes);

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(item->opt==0){
        /*printf("No Optional fields. iE-Extensions is void: asn1_NOVALUE\n");*/
    }else{
        s1ap_msg(WARN, 0, "Optional iE-Extensions is present, parsing not included in current version.");
    }

    /*Extensions*/
    getextension(&extensions, bytes, item->ext);
    skipextensions(bytes, 1, &extensions);

    return item;
/*
'dec_SupportedTAs-Item'(Bytes,_) ->
{Ext,Bytes1} = ?RT_PER:getext(Bytes),
{Opt,Bytes2} = ?RT_PER:getoptionals2(Bytes1,1),
%% attribute number 1 with type OCTET STRING
{Term1,Bytes3} = ?RT_PER:decode_octet_string(Bytes2,[{'SizeConstraint',2}]),

%%  attribute number 2 with type BPLMNs
{Term2,Bytes4} = 'dec_BPLMNs'(Bytes3,telltype),

%% attribute number 3 with type SEQUENCE OF
{Term3,Bytes5} = case Opt band (1 bsl 0) of
  _Opt3 when _Opt3 > 0 ->'dec_SupportedTAs-Item_iE-Extensions'(Bytes4, telltype);
0 ->{asn1_NOVALUE,Bytes4}

end,
{Extensions,Bytes6} = ?RT_PER:getextension(Ext,Bytes5),
Bytes7= ?RT_PER:skipextensions(Bytes6,1,Extensions)
,
{{'SupportedTAs-Item',Term1,Term2,Term3},Bytes7}
*/
}

CSG_IdList_Item_t * dec_CSG_IdList_Item(struct BinaryData *bytes){
    CSG_IdList_Item_t *item;
    S1AP_PROTOCOL_IES_t fakeie;
    struct BinaryData opt, extensions;
    uint8_t buffer[MAXDATABYTES];

    opt.data=buffer;

    item = new_CSG_IdList_Item();
    /*Get extension flag*/
    getbit(bytes, &(item->ext));

    /*Get optionals*/
    getbits(&opt, bytes, 1);
    item->opt=opt.data[0];

    /*attribute number 1 with type OCTET STRING*/
    dec_CSGid(&fakeie, bytes);
    item->cSG_id = fakeie.value;

    /* attribute number 2 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(opt.data[0]==0){
        /*printf("No Optional fields. iE-Extensions is void: asn1_NOVALUE\n");*/
    }else{
        s1ap_msg(WARN, 0, "Optional iE-Extensions is present, parsing not included in current version.");
    }

    /*Extensions*/
    getextension(&extensions, bytes, item->ext);
    skipextensions(bytes, 1, &extensions);

    return item;
}

MME_Group_ID_t *dec_MME_Group_ID(struct BinaryData *bytes){
    MME_Group_ID_t *v;

    v=new_MME_Group_ID();
    decode_octet_string(v->s, bytes, 2);
    return v;
}

ServedGroupIDs_t *dec_servedGroupIDs(struct BinaryData *bytes){
    uint8_t i, length;
    MME_Group_ID_t *item;
    ServedGroupIDs_t *v = new_ServedGroupIDs();

    /*Decode length*/
    length = decode_constrained_number(bytes, 1, maxnoofGroupIDs);

    /*Decode MME_Group_ID_t*/
    for(i=0 ; i < length ; i++){
        item = NULL;
        item = dec_MME_Group_ID(bytes);
        v->additem(v, item);
    }
    return v;
}

MME_Code_t *dec_MME_Code(struct BinaryData *bytes){
    MME_Code_t *v;

    v=new_MME_Code();
    decode_octet_string(v->s, bytes, 1);
    return v;
}

ServedMMECs_t *dec_servedMMECs(struct BinaryData *bytes){
    uint8_t i, length;
    MME_Code_t *item;
    ServedMMECs_t *v = new_ServedMMECs();

    /*Decode length*/
    length = decode_constrained_number(bytes, 1, maxnoofMMECs);

    /*Decode MME_Code_t*/
    for(i=0 ; i < length ; i++){
        item = NULL;
        item = dec_MME_Code(bytes);
        v->additem(v, item);
    }
    return v;
}

ServedGUMMEIsItem_t *dec_ServedGUMMEIsItem(struct BinaryData *bytes){
    ServedGUMMEIsItem_t *item;
    S1AP_PROTOCOL_IES_t fakeie;

    struct BinaryData opt, extensions;
    uint8_t buffer[MAXDATABYTES];

    opt.data=buffer;

    item = new_ServedGUMMEIsItem();
    /*Get extension flag*/
    getbit(bytes, &(item->ext));

    /*Get optionals*/
    getbits(&opt, bytes, 1);
    item->opt=opt.data[0];

    /*attribute number 1 with type servedPLMNs*/
    dec_servedPLMNs(&fakeie, bytes);
    item->servedPLMNs = fakeie.value;

    /*attribute number 2 with type servedGroupIDs*/
    item->servedGroupIDs = dec_servedGroupIDs(bytes);

    /*attribute number 3 with type servedGroupIDs*/
    item->servedMMECs = dec_servedMMECs(bytes);

    /* attribute number 4 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(opt.data[0]==0){
        /*printf("No Optional fields. iE-Extensions is void: asn1_NOVALUE\n");*/
    }else{
        s1ap_msg(WARN, 0, "Optional iE-Extensions is present, parsing not included in current version.");
    }

    /*Extensions*/
    getextension(&extensions, bytes, item->ext);
    skipextensions(bytes, 1, &extensions);

    /*item->servedGroupIDs=;
    item->servedMMECs=;
    item->servedPLMNs=*/

    return item;
}

CriticalityDiagnostics_IE_Item_t *dec_CriticalityDiagnostics_IE_Item(struct BinaryData *bytes){
    CriticalityDiagnostics_IE_Item_t *item;
    struct BinaryData opt, extensions;
    uint8_t buffer[MAXDATABYTES];

    item = new_CriticalityDiagnostics_IE_Item();

    /*Get extension flag*/
    getbit(bytes, &item->ext);

    /*Get optional flags*/
    opt.data=buffer;
    getbits(&opt, bytes, 5);
    item->opt=opt.data[0];

    /* attribute number 1 with type ENUMERATED*/
    if((item->opt&0x8) == 0x8){
        item->iECriticality=decode_constrained_number(bytes, 0, 2);
    }
    /*attribute number 2 with type INTEGER*/
    if((item->opt&0x4) == 0x4){
        item->iE_ID=decode_constrained_number(bytes, 0, 255);
    }
    /* attribute number 3 with type ENUMERATED*/
    if((item->opt&0x2) == 0x2){
        item->typeOfError=decode_constrained_number(bytes, 0, 1);
    }
    /* attribute number 4 with type  ProtocolExtensionContainer*/
    if((item->opt&0x1) == 0x1){
        s1ap_msg(ERROR, 0, "Optional iE-Extensions is present, parsing not included in current version.\n");
    }

    /*Extensions*/
    getextension(&extensions, bytes, item->ext);
    skipextensions(bytes, 1, &extensions);
    return item;
}

CriticalityDiagnostics_IE_List_t *dec_CriticalityDiagnostics_IE_List(struct BinaryData *bytes){
    uint8_t i, length;
    CriticalityDiagnostics_IE_Item_t *item;
    CriticalityDiagnostics_IE_List_t *v = new_CriticalityDiagnostics_IE_List();

    /*Decode length*/
    length = decode_constrained_number(bytes, 1, maxNrOfErrors);

    /*Decode MME_Group_ID_t*/
    for(i=0 ; i < length ; i++){
        item = NULL;
        item = dec_CriticalityDiagnostics_IE_Item(bytes);
        v->additem(v, item);
    }
    return v;
}

AllocationAndRetentionPriority_t *dec_AllocationAndRetentionPriority(struct BinaryData *bytes){
    AllocationAndRetentionPriority_t *v;
    struct BinaryData extensions;

    v = new_AllocationAndRetentionPriority();

    /*Get extension flag*/
    getbit(bytes, &v->ext);

    /*Get optional flags*/
    getbit(bytes, &v->opt);

    /* attribute number 1 with type priorityLevel*/
    v->priorityLevel = decode_constrained_number(bytes, 0, 15);

    /* attribute number 2 with type enum*/
    v->pre_emptionCapability = decode_enumerated(bytes, 0, 1);
    /* attribute number 3 with type enum*/
    v->pre_emptionVulnerability = decode_enumerated(bytes, 0, 1);

    /* attribute number 4 with type  ProtocolExtensionContainer*/
    if(v->opt == 1){
        s1ap_msg(ERROR, 0, "Optional iE-Extensions is present, parsing not included in current version.\n");
    }

    /*Extensions*/
    getextension(&extensions, bytes, v->ext);
    skipextensions(bytes, 1, &extensions);

    return v;
}

GBR_QosInformation_t *dec_GBR_QosInformation(struct BinaryData *bytes){
    struct BinaryData  extensions;

    GBR_QosInformation_t *v;

    v = new_GBR_QosInformation();

    /*Get extension flag*/
    getbit(bytes, &(v->ext));

    /*Get optionals*/
    getbit(bytes, &(v->opt));

    /*attribute number 1 with type BitRate_t*/
    v->eRAB_MaximumBitrateDL.rate = decode_constrained_number(bytes, 0, 10000000000ULL);

    /*attribute number 2 with type BitRate_t*/
    v->eRAB_MaximumBitrateUL.rate = decode_constrained_number(bytes, 0, 10000000000ULL);

    /*attribute number 3 with type BitRate_t*/
    v->eRAB_GuaranteedBitrateDL.rate = decode_constrained_number(bytes, 0, 10000000000ULL);

    /*attribute number 4 with type BitRate_t*/
    v->eRAB_GuaranteedBitrateUL.rate = decode_constrained_number(bytes, 0, 10000000000ULL);

    /* attribute number 4 with type  ProtocolExtensionContainer*/
    if(v->opt == 1){
        s1ap_msg(ERROR, 0, "Optional iE-Extensions is present, parsing not included in current version.\n");
    }

    /*Extensions*/
    getextension(&extensions, bytes, v->ext);
    skipextensions(bytes, 1, &extensions);
    return v;
}

E_RABLevelQoSParameters_t *dec_E_RABLevelQoSParameters(struct BinaryData *bytes){
    E_RABLevelQoSParameters_t *v;
    struct BinaryData opt, extensions;
    uint8_t buffer[MAXDATABYTES];

    v = new_E_RABLevelQoSParameters();

    /*Get extension flag*/
    getbit(bytes, &v->ext);

    /*Get optional flags*/
    opt.data=buffer;
    getbits(&opt, bytes, 5);
    v->opt=opt.data[0];

    /* attribute number 1 with type QCI_t*/
    v->qCI = decode_constrained_number(bytes, 0, 255);

    /* attribute number 2 with type AllocationAndRetentionPriority_t*/
    v->allocationRetentionPriority = dec_AllocationAndRetentionPriority(bytes);

    /* attribute number 3 with type AllocationAndRetentionPriority_t*/
    if((v->opt&0x80) == 0x80){
        v->gbrQosInformation = dec_GBR_QosInformation(bytes);
    }else{
        /*s1ap_msg(WARN, 0, "Optional gbrQosInformation is not present. v->opt = %#x\n", v->opt);*/
    }

    /* attribute number 4 with type  ProtocolExtensionContainer*/
    if((v->opt&0x40) == 0x40){
        s1ap_msg(ERROR, 0, "Optional iE-Extensions is present, parsing not included in current version.");
    }

    /*Extensions*/
    getextension(&extensions, bytes, v->ext);
    skipextensions(bytes, 1, &extensions);

    return v;
}

TransportLayerAddress_t *dec_TransportLayerAddress(struct BinaryData *bytes){

    TransportLayerAddress_t *v;
    struct BinaryData bits;

    v = new_TransportLayerAddress();

    /*Get extension flag*/
    getbit(bytes, &(v->ext));

    bits.pos=0;
    bits.length=0;
    bits.offset=v->addr;
    bits.data=v->addr;

    /*Decode length*/
    v->len = decode_constrained_number(bytes, 1, 160);

    align_dec(bytes);
    getbits(&bits, bytes ,v->len);

    return v;
}

UE_S1AP_ID_pair_t *dec_UE_S1AP_ID_pair(struct BinaryData *bytes){
    S1AP_PROTOCOL_IES_t fakeie1, fakeie2;
    UE_S1AP_ID_pair_t *v;
    v = new_UE_S1AP_ID_pair();

    /*Get extension flag*/
    getbit(bytes, &(v->ext));
    /*Get optionals*/
    getbit(bytes, &(v->opt));
    /* attribute number 1 with type MME_UE_S1AP_ID */
    dec_MME_UE_S1AP_ID(&fakeie1, bytes);
    v->mME_UE_S1AP_ID = fakeie1.value;
    /* attribute number 2 with type ENB_UE_S1AP_ID */
    dec_ENB_UE_S1AP_ID(&fakeie2, bytes);
    v->eNB_UE_S1AP_ID = fakeie2.value;
    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==1){
        v->iEext->addIe(v->iEext, dec_protocolIEs(bytes));
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, decoding not included in current version.\n");
        /*TODO extensions encoding*/
    }

    return v;
}

Bearers_SubjectToStatusTransferList_t *dec_Bearers_SubjectToStatusTransferList(struct BinaryData *bytes){
    uint8_t i, length;
    Bearers_SubjectToStatusTransferList_t *v;

    v = new_Bearers_SubjectToStatusTransferList();

    /*Decode length*/
    length = decode_constrained_number(bytes, 1, maxNrOfERABs);

    /*Decode Bearers_SubjectToStatusTransfer_Item_t, tyep Bearers_SubjectToStatusTransfer_Item*/
    for(i=0 ; i < length ; i++){
        v->additem(v, (ProtocolIE_SingleContainer_t*)dec_protocolIEs(bytes));
    }
    return v;
}

COUNTvalue_t *dec_COUNTvalue(struct BinaryData *bytes){
    struct BinaryData extensions;
    COUNTvalue_t *v;
    v = new_COUNTvalue();

    /*Get extension flag*/
    getbit(bytes, &(v->ext));

    /*Get optionals*/
    getbit(bytes, &(v->opt));

    /* attribute number 1  with type PDCP-SN ( INTEGER (0..4095) ) */
    v->pDCP_SN.pDCP_SN = decode_constrained_number(bytes, 0, 4095);

    /* attribute number 2  with type HFN ( INTEGER (0..1048575) ) */
    v->hFN.hFN = decode_constrained_number(bytes, 0, 1048575);

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if((v->opt&0x40)==0x40){
        v->iEext->addIe(v->iEext, dec_protocolIEs(bytes));
    }

    /*Extensions*/
    getextension(&extensions, bytes, v->ext);
    skipextensions(bytes, 1, &extensions);

    return v;
}

void dec_E_RAB_ID(E_RAB_ID_t *v, struct BinaryData *bytes){

    getbit(bytes, &(v->ext));
    if(v->ext == 0){
        v->id = decode_constrained_number(bytes, 0, 15);
        /*printf("test eRAB id %u, v->eRAB_ID.ext %u\n",v->id, v->ext);*/
    }else{
        v->id = decode_semi_constrained_number(bytes, 0);
    }
}

TargeteNB_ID_t *dec_TargeteNB_ID(struct BinaryData *bytes){
    S1AP_PROTOCOL_IES_t fakeie1, fakeie2;
    struct BinaryData extensions;
    TargeteNB_ID_t *v;
    v = new_TargeteNB_ID();

    /*Get extension flag*/
    getbit(bytes, &(v->ext));

    /*Get optionals*/
    getbit(bytes, &(v->opt));

    /* attribute number 1  with type Global_ENB_ID */
    dec_Global_ENB_ID(&fakeie1, bytes);
    v->global_ENB_ID = fakeie1.value;

    /* attribute number 2  with type TAI */
    dec_TAI(&fakeie2, bytes);
    v->selected_TAI = fakeie2.value;

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if((v->opt&0x40)==0x40){
        v->iEext->addIe(v->iEext, dec_protocolIEs(bytes));
    }

    /*Extensions*/
    getextension(&extensions, bytes, v->ext);
    skipextensions(bytes, 1, &extensions);

    return v;
}

TargetRNC_ID_t *dec_TargetRNC_ID(struct BinaryData *bytes){
    S1AP_PROTOCOL_IES_t fakeie1;
    struct BinaryData extensions, opt;
    uint8_t buffer[MAXDATABYTES];
    TargetRNC_ID_t *v;

    v = new_TargetRNC_ID();

    /*Get extension flag*/
    getbit(bytes, &(v->ext));

    /*Get optionals*/
    /*Get optional flags*/
    opt.data=buffer;
    getbits(&opt, bytes, 5);
    v->opt=opt.data[0];

    /* attribute number 1  with type  LAI*/
    dec_LAI(&fakeie1, bytes);
    v->lAI = fakeie1.value;

    /* attribute number 2  with type  RAC (optional)*/
    if((v->opt&0x80) == 0x80){
        s1ap_msg(WARN, 0, "Optional is RAC present. v->opt = %#x, not implemented yet\n", v->opt);
    }else{
        /*s1ap_msg(WARN, 0, "Optional is not present. v->opt = %#x\n", v->opt);*/
    }

    /* attribute number 3 with type RNC-ID*/
    v->rNC_ID.pDCP_SN = decode_constrained_number(bytes, 0, 4095);;

    /* attribute number 4  with type  ExtendedRNC-ID (optional)*/
    if((v->opt&0x40) == 0x40){
        s1ap_msg(WARN, 0, "Optional ExtendedRNC-ID is present. v->opt = %#x, not implemented yet\n", v->opt);
    }else{
        /*s1ap_msg(WARN, 0, "Optional is not present. v->opt = %#x\n", v->opt);*/
    }

    /* attribute number 5 with type  ProtocolExtensionContainer*/
    if((v->opt&0x20) == 0x20){
        s1ap_msg(ERROR, 0, "Optional iE-Extensions is present, parsing not included in current version.\n");
    }

    /*Extensions*/
    getextension(&extensions, bytes, v->ext);
    skipextensions(bytes, 1, &extensions);

    return v;
}

CGI_t *dec_CGI(struct BinaryData *bytes){
    struct BinaryData extensions, opt;
    uint8_t buffer[MAXDATABYTES];
    CGI_t *v;

    v = new_CGI();

    /*Get extension flag*/
    getbit(bytes, &(v->ext));

    /*Get optional flags*/
    opt.data=buffer;
    getbits(&opt, bytes, 5);
    v->opt=opt.data[0];

    /* attribute number 1  with type PLMNidentity */
    v->pLMNidentity = dec_PLMNidentity(bytes);

    /* attribute number 2  with type LAC */
    decode_octet_string(v->lAC.s, bytes, 2);

    /* attribute number 3  with type CI */
    decode_octet_string(v->cI.s, bytes, 2);


    /* attribute number 4 with type RAC (optional)*/
    if((v->opt&0x80) == 0x80){
        s1ap_msg(ERROR, 0, "Optional RAC is present, not implemented yet\n");

    }else{
        /*s1ap_msg(WARN, 0, "Optional is not present. v->opt = %#x\n", v->opt);*/
    }

    /* attribute number 5 with type  ProtocolExtensionContainer*/
    if((v->opt&0x40) == 0x40){
        s1ap_msg(ERROR, 0, "Optional iE-Extensions is present, parsing not included in current version.\n");
    }

    /*Extensions*/
    getextension(&extensions, bytes, v->ext);
    skipextensions(bytes, 1, &extensions);

    return v;
}


void dec_Global_ENB_ID(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    Global_ENB_ID_t *v;
    struct BinaryData opt, extensions;
    uint8_t buffer[MAXDATABYTES], ext;
    uint32_t choice;

    v = new_Global_ENB_ID();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    opt.data=buffer;
    getbit(bytes, &ext);
    v->ext=ext;
    getbits(&opt, bytes, 1);
    v->opt=opt.data[0];

    /*attribute number 1 (pLMNidentity) with type OCTET STRING */
    v->pLMNidentity = dec_PLMNidentity(bytes);

    /*attribute number 2 eNB-ID with type ENB-ID*/
    getbit(bytes, &ext);
    v->eNBid->ext=ext;
    choice = getchoice(bytes, 2, ext);
    v->eNBid->choice=choice;
    /*printf("Extension not implemented %u, choice %u\n", ext, choice);*/
    align_dec(bytes);
    switch(choice+ext*2){
    case 0:
        /*macroENB-ID*/
        v->eNBid->id.macroENB_ID = decode_bit_string(bytes, 20);
        break;
    case 1:
        /*homeENB-ID*/
        v->eNBid->id.homeENB_ID = decode_bit_string(bytes, 28);
        break;
    default:
        s1ap_msg(WARN, 0, "Extension not implemented\n");
        /*Extension not implemented*/
        break;
    }

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(opt.data[0]==0){
        /*printf("No Optional fields. iE-Extensions is void: asn1_NOVALUE\n");*/
    }else{
        s1ap_msg(WARN, 0, "Optional iE-Extensions is present, parsing not included in current version.");
    }

    /*Extensions*/
    getextension(&extensions, bytes, ext);
    skipextensions(bytes, 1, &extensions);

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

void dec_ENBname(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    ENBname_t *v = new_ENBname();
    ie->value=v;

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;
    decode_known_multiplier_string_PrintableString_withExt(v->name, bytes, 1, 150);
/*
'dec_ENBname'(Bytes,_) ->
?RT_PER:decode_PrintableString(Bytes,[{'SizeConstraint',{{1,150},[]}}]).

decode_PrintableString(Bytes,C) ->
    decode_known_multiplier_string(Bytes,aligned,'PrintableString',C,false).

decode_known_multiplier_string(Bytes,aligned,StringType,C,_Ext) ->
    NumBits = get_NumBits(C,StringType),
    case get_constraint(C,'SizeConstraint') of
    Ub when integer(Ub), Ub*NumBits =< 16  ->
        chars_decode(Bytes,NumBits,StringType,C,Ub);
    Ub when integer(Ub),Ub =<65535 -> % fixed length
        Bytes1 = align(Bytes),
        chars_decode(Bytes1,NumBits,StringType,C,Ub);
    0 ->
        {[],Bytes};
    Vl when list(Vl) ->
        {Len,Bytes1} = decode_length(Bytes,{hd(Vl),lists:max(Vl)}),
        Bytes2 = align(Bytes1),
        chars_decode(Bytes2,NumBits,StringType,C,Len);
    no  ->
        {Len,Bytes1} = decode_length(Bytes,undefined),
        Bytes2 = align(Bytes1),
        chars_decode(Bytes2,NumBits,StringType,C,Len);
    {Lb,Ub}->
        {Len,Bytes1} = decode_length(Bytes,{Lb,Ub}),
        Bytes2 = align(Bytes1),
        chars_decode(Bytes2,NumBits,StringType,C,Len)
    end.
 */
}

void dec_MMEname(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    MMEname_t *v = new_MMEname();
    ie->value=v;

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;
    decode_known_multiplier_string_PrintableString_withExt(v->name, bytes, 1, 150);
}

void dec_SupportedTAs(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    SupportedTAs_t *v;
    SupportedTAs_Item_t *item;

    uint8_t length;
    uint16_t i;

    v = new_SupportedTAs();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Decode length*/
    length = decode_constrained_number(bytes, 1, maxnoofTACs);
    /*printf("\tIE SupportedTAs size %u\n", v->size);*/

    /*Decode SupportedTAs_Items*/
    for(i=0 ; i < length ; i++){
        item = NULL;
        item = dec_SupportedTAs_Item(bytes);
        v->additem(v, item);
    }
}

void dec_PagingDRX(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    PagingDRX_t *v;

    v = new_PagingDRX();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    v->pagingDRX = decode_enumerated(bytes, 0, 3);
}

void dec_CSGid(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){

    cSG_id_t *v;
    uint32_t tmp;
    uint8_t bCSG[27];
    struct BinaryData cSG;

    cSG.data=bCSG;
    cSG.offset=bCSG;
    cSG.length=0;
    cSG.pos=0;

    v = new_CSG_id();
    /*Link functions*/
    ie->freeValue = v->freeIE;
    ie->showValue = v->showIE;
    ie->value=v;

    /*Decode*/
    getbits(&cSG, bytes, 27);
    printf("CSG %#x %#x %#x %#x\n", cSG.data[0], cSG.data[1], cSG.data[2], cSG.data[3]);
    memcpy(&tmp, cSG.data, 4);
    v->id = ntohl(tmp)>>(32-27);
}

void dec_CSGidList(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    uint8_t i, length;
    CSG_IdList_Item_t *item;
    CSG_IdList_t *v = new_CSG_IdList();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Decode length*/
    length = decode_constrained_number(bytes, 1, maxNrOfCSGs);

    /*Decode CSGidList_Items*/
    for(i=0 ; i < length ; i++){
        item = NULL;
        item = dec_CSG_IdList_Item(bytes);
        v->additem(v, item);
    }
}

void dec_servedPLMNs(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    uint8_t i, length;
    PLMNidentity_t *item;
    ServedPLMNs_t *v = new_ServedPLMNs();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Decode length*/
    length = decode_constrained_number(bytes, 1, maxnoofPLMNsPerMME);

    /*Decode ServedGUMMEIsItem_t*/
    for(i=0 ; i < length ; i++){
        item = NULL;
        item = dec_PLMNidentity(bytes);
        v->additem(v, item);
    }
}

void dec_ServedGUMMEIs(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    uint8_t i, length;
    ServedGUMMEIsItem_t *item;
    ServedGUMMEIs_t *v = new_ServedGUMMEIs();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Decode length*/
    length = decode_constrained_number(bytes, 1, maxNrOfCSGs);

    /*Decode ServedGUMMEIsItem_t*/
    for(i=0 ; i < length ; i++){
        item = NULL;
        item = dec_ServedGUMMEIsItem(bytes);
        v->additem(v, item);
    }
}

void dec_RelativeMMECapacity(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    RelativeMMECapacity_t *v = new_RelativeMMECapacity();

    /*Link functions*/
    ie->freeValue = v->freeIE;
    ie->showValue = v->showIE;
    ie->value=v;

    v->cap = decode_constrained_number(bytes, 0, 255);
}

void dec_MMERelaySupportIndicator(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    MMERelaySupportIndicator_t *v = new_MMERelaySupportIndicator();

    /*Link functions*/
    ie->freeValue = v->freeIE;
    ie->showValue = v->showIE;
    ie->value=v;

    /*Get Extension flag*/
    getbit(bytes, &v->ext);

    if(v->ext!=0){
        v->ind = decode_small_number(bytes);
    }
}

void dec_CriticalityDiagnostics(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    CriticalityDiagnostics_t *v;
    struct BinaryData opt, extensions;
    uint8_t buffer[MAXDATABYTES];

    v = new_CriticalityDiagnostics();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Get extension flag*/
    getbit(bytes, &v->ext);

    /*Get optional flags*/
    opt.data=buffer;
    getbits(&opt, bytes, 5);
    v->opt=opt.data[0];

    /*attribute number 1 with type INTEGER*/
    if((v->opt&0x10) == 0x10){
        v->procedureCode=decode_constrained_number(bytes, 0, 255);
    }
    /* attribute number 2 with type ENUMERATED*/
    if((v->opt&0x8) == 0x8){
        v->triggeringMessage=decode_constrained_number(bytes, 0, 2);
    }
    /* attribute number 3 with type ENUMERATED*/
    if((v->opt&0x4) == 0x4){
        v->procedureCriticality=decode_constrained_number(bytes, 0, 2);
    }
    /* attribute number 4 with type  CriticalityDiagnostics-IE-List*/
    if((v->opt&0x2) == 0x2){
        v->iEsCriticalityDiagnostics=dec_CriticalityDiagnostics_IE_List(bytes);
    }
    /* attribute number 5 with type  CriticalityDiagnostics-IE-List*/
    if((v->opt&0x1) == 0x1){
        s1ap_msg(ERROR, 0,"Optional iE-Extensions is present, parsing not included in current version.\n");
    }

    /*Extensions*/
    getextension(&extensions, bytes, v->ext);
    skipextensions(bytes, 1, &extensions);

}

void dec_Cause(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    Cause_t *v;
    uint8_t ext;


    v = new_Cause();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Get extension flag*/
    getbit(bytes, &v->ext);

    if(v->ext==0){
        /* Get Choice*/
        v->choice = getchoice(bytes, 5, v->ext);

        /*Get extension flag*/
        getbit(bytes, &ext);

        switch(v->choice){
        case 0: /*CauseRadioNetwork*/
            v->cause.radioNetwork.ext = ext;
            if(ext==0){
                v->cause.radioNetwork.cause.noext = decode_constrained_number(bytes, 0, 35);

            }
            else{
                v->cause.radioNetwork.cause.ext = decode_small_number(bytes);
            }
            break;
        case 1: /*CauseTransport*/
            v->cause.transport.ext=ext;
            if(ext==0){
                v->cause.transport.cause.noext = decode_constrained_number(bytes, 0, 1);
            }
            else{
                printf("CauseTransport extension detected. Not available in current version.");
            }
            break;

        case 2: /*CauseNAS*/
            v->cause.nas.ext=ext;
            if(ext==0){
                v->cause.nas.cause.noext = decode_constrained_number(bytes, 0, 3);
            }
            else{
                v->cause.nas.cause.ext = decode_small_number(bytes);
            }
            break;

        case 3: /*CauseProtocol*/
            v->cause.protocol.ext=ext;
            if(ext==0){
                v->cause.protocol.cause.noext = decode_constrained_number(bytes, 0, 6);
            }
            else{
                printf("CauseProtocol extension detected. Not available in current version.");
            }
            break;

        case 4: /*CauseMisc*/
            v->cause.misc.ext=ext;
            if(ext==0){
                v->cause.misc.cause.noext = decode_constrained_number(bytes, 0, 5);
            }
            else{
                printf("CauseMisc extension detected. Not available in current version.");
            }
            break;
        }
    }else{
        s1ap_msg(WARN, 0, "Found extension flag in Cause IE. Current protocol version hasn't any extension. Ignoring.");
        /*TODO IGNORE */
    }
}

void dec_TimeToWait(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    TimeToWait_t *v;

    v = new_TimeToWait();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Get extension flag*/
    getbit(bytes, &v->ext);

    if(v->ext==0){
        v->time.noext = decode_constrained_number(bytes, 0, 5);
    }else{
        s1ap_msg(WARN, 0, "Found extension flag in TimeToWait IE. Current protocol version hasn't any extension. Ignoring.");
        /*TODO IGNORE */
    }
}

void dec_ENB_UE_S1AP_ID(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    ENB_UE_S1AP_ID_t *v;

    v = new_ENB_UE_S1AP_ID();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Decode ID*/
    v->eNB_id = decode_constrained_number(bytes, 0, 16777215);
    /*printf("v->eNB_id %u\n", v->eNB_id);*/

}

void dec_MME_UE_S1AP_ID(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    MME_UE_S1AP_ID_t *v;

    v = new_MME_UE_S1AP_ID();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Decode ID*/
    /*printf("bytes->pos %u, bytes->data %#x %#x %#x %#x %#x\n", bytes->pos, bytes->data[0], bytes->data[1], bytes->data[2], bytes->data[3], bytes->data[4]);*/
    v->mme_id = decode_constrained_number(bytes, 0, 4294967295ULL);
    /*printf("v->mme_id %u\n", v->mme_id);*/
}

void dec_Unconstrained_Octed_String(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    struct BinaryData bits;
    Unconstrained_Octed_String_t *v;

    v = new_Unconstrained_Octed_String();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Decode len*/
    v->len = decode_length_undef(bytes);

    /*Decode String*/
    v->str = malloc(v->len);
    if(v->str!=NULL){
        bits.data = v->str;
        getoctets(&bits, bytes, v->len);
    }else{
        s1ap_msg(ERROR, 0, "Couldn't allocate Unconstrained_Octed_String_t string memory.");
    }
}

/*void dec_NAS_PDU(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    struct BinaryData bits;
    NAS_PDU_t *v;

    v = new_NAS_PDU();
*/
    /*Link functions*//*
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    *//*Decode len*//*
    v->len = decode_length_undef(bytes);

    *//*Decode String*//*
    v->str = malloc(v->len);
    if(v->str!=NULL){
        bits.data = v->str;
        getoctets(&bits, bytes, v->len);
    }else{
        s1ap_msg(ERROR, 0, "Couldn't allocate NAS_PDU string memory.");
    }
}*/

void dec_TAI(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    struct BinaryData extensions;
    TAI_t *v;

    v = new_TAI();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Get extension flag*/
    getbit(bytes, &(v->ext));

    /*Get optionals*/
    getbit(bytes, &(v->opt));


    /*attribute number 1 with type pLMNidentity*/
    v->pLMNidentity = dec_PLMNidentity(bytes);

    /*attribute number 2 with type TAC*/
    decode_octet_string(v->tAC->s, bytes, 2);

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==0){
        /*printf("No Optional fields. iE-Extensions is void: asn1_NOVALUE\n");*/
    }else{
        s1ap_msg(WARN, 0, "Optional iE-Extensions is present, parsing not included in current version.");
    }

    /*Extensions*/
    getextension(&extensions, bytes, v->ext);
    skipextensions(bytes, 1, &extensions);
}

void dec_EUTRAN_CGI (S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    struct BinaryData opt, extensions;
    uint8_t buffer[MAXDATABYTES];
    EUTRAN_CGI_t *v;

    opt.data=buffer;
    v = new_EUTRAN_CGI();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Get extension flag*/
    getbit(bytes, &(v->ext));

    /*Get optionals*/
    getbits(&opt, bytes, 1);

    v->opt=opt.data[0];
    /*attribute number 1 with type pLMNidentity*/
    v->pLMNidentity = dec_PLMNidentity(bytes);

    /*attribute number 2 with type Cell-ID*/
    v->cell_ID.id = decode_bit_string(bytes, 28);

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(opt.data[0]==0){
        /*printf("No Optional fields. iE-Extensions is void: asn1_NOVALUE\n");*/
    }else{
        s1ap_msg(WARN, 0, "Optional iE-Extensions is present, parsing not included in current version.");
    }

    /*Extensions*/
    getextension(&extensions, bytes, v->ext);
    skipextensions(bytes, 1, &extensions);
}

void dec_RRC_Establishment_Cause(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    RRC_Establishment_Cause_t *v;

    v = new_RRC_Establishment_Cause();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Get extension flag*/
    getbit(bytes, &v->ext);
    /*printf("dec_RRC_Establishment_Cause() : bytes %#x %#x, %u\n", bytes->data[0], bytes->data[1], bytes->pos);*/

    if(v->ext==0){
        v->cause.noext = decode_constrained_number(bytes, 0, 4);
        /*printf("dec_RRC_Establishment_Cause() : bytes %#x %#x, %u\n", bytes->data[0], bytes->data[1], v->cause.noext);*/

    }else{
        v->cause.ext = decode_small_number(bytes);
    }
}

void dec_UEAggregateMaximumBitrate (S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    struct BinaryData extensions;
    UEAggregateMaximumBitrate_t *v;

    v = new_UEAggregateMaximumBitrate();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Get extension flag*/
    getbit(bytes, &(v->ext));

    /*Get optionals*/
    getbit(bytes, &v->opt);

    /*attribute number 1 with type BitRate*/
    v->uEaggregateMaximumBitRateDL.rate = decode_constrained_number(bytes, 0, 10000000000ULL);

    /*attribute number 2 with type BitRate*/
    v->uEaggregateMaximumBitRateUL.rate = decode_constrained_number(bytes, 0, 10000000000ULL);

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==0){
        /*printf("No Optional fields. iE-Extensions is void: asn1_NOVALUE\n");*/
    }else{
        s1ap_msg(WARN, 0, "Optional iE-Extensions is present, parsing not included in current version.");
    }

    /*Extensions*/
    getextension(&extensions, bytes, v->ext);
    skipextensions(bytes, 1, &extensions);
}

void dec_E_RABToBeSetupItemCtxtSUReq(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    S1AP_PROTOCOL_IES_t fakeie, *iEext;
    struct BinaryData opt, extensions;
    uint8_t buffer[MAXDATABYTES];

    E_RABToBeSetupItemCtxtSUReq_t *v;

    v = new_E_RABToBeSetupItemCtxtSUReq();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Get extension flag*/
    getbit(bytes, &(v->ext));

    /*Get optionals*/
    opt.data=buffer;
    getbits(&opt, bytes, 2);
    v->opt=buffer[0]&0xC0;

    /*attribute number 1 with type E_RAB_ID_t*/
    dec_E_RAB_ID(&(v->eRAB_ID), bytes);

    /*attribute number 2 with type E_RABLevelQoSParameters_t*/
    v->eRABlevelQoSParameters = dec_E_RABLevelQoSParameters(bytes);

    /*attribute number 3 with type TransportLayerAddress_t*/
    v->transportLayerAddress = dec_TransportLayerAddress(bytes);

    /*attribute number 4 with type GTP_TEID_t*/
    decode_octet_string(v->gTP_TEID.teid, bytes, 4);

    /*attribute number 5 with type Unconstrained_Octed_String (NAS_PDU)*/
    if( (v->opt&0x80)==0x80 ){
        dec_Unconstrained_Octed_String(&fakeie, bytes);
        v->nAS_PDU = (Unconstrained_Octed_String_t *)fakeie.value;
    }

    /* attribute number 6 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if((v->opt&0x40)==0x40){
        iEext = dec_protocolIEs(bytes);
        v->iEext->addIe(v->iEext, iEext);
    }

    /*Extensions*/
    getextension(&extensions, bytes, v->ext);
    skipextensions(bytes, 1, &extensions);
}

void dec_E_RABToBeSetupListCtxtSUReq(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    uint8_t i, length;
    S1AP_PROTOCOL_IES_t *fakeie;
    E_RABToBeSetupListCtxtSUReq_t *v = new_E_RABToBeSetupListCtxtSUReq();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Decode length*/
    length = decode_constrained_number(bytes, 1, maxNrOfERABs);

    /*Decode E_RABToBeSetupItemCtxtSUReq_t*/
    for(i=0 ; i < length ; i++){
        fakeie = dec_protocolIEs(bytes);
        v->additem(v, (ProtocolIE_SingleContainer_t*)fakeie);
    }
}

void dec_SecurityKey(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    struct BinaryData bits;
    SecurityKey_t *v = new_SecurityKey();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    bits.data = v->key;
    bits.offset = v->key;
    bits.pos=0;
    bits.length=0;

    /*Decode key 256 bits = 32 bytes*/
    getoctets(&bits, bytes, 32);

}

void dec_SubscriberProfileIDforRFP(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    SubscriberProfileIDforRFP_t *v = new_SubscriberProfileIDforRFP();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Decode*/
    v->subscriberProfileIDforRFP = decode_constrained_number(bytes, 1, 256);

}

void dec_UESecurityCapabilities(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    struct BinaryData bits;
    uint8_t buff[10];
    UESecurityCapabilities_t *v = new_UESecurityCapabilities();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    bits.data=buff;
    bits.offset= buff;
    bits.length=0;
    bits.pos=0;

    /*Get extension flag*/
    getbit(bytes, &(v->ext));

    /*Get optionals*/
    getbit(bytes, &(v->opt));

    /*Decode encryptionAlgorithms*/
    /*Get ext*/
    getbit(bytes, &(v->encryptionAlgorithms.ext));
    if(v->encryptionAlgorithms.ext==0 ){
        getbits(&bits, bytes, 16);
        memcpy(&(v->encryptionAlgorithms.v), bits.data, 2);
        v->encryptionAlgorithms.v = ntohs(v->encryptionAlgorithms.v);
        /*printf("** test encryptionAlgorithms data %#x\n", v->encryptionAlgorithms.v);*/
    }else{
        s1ap_msg(WARN, 0, "encryptionAlgorithms extension decoder not implemented yet.");
    }

    /*Decode integrityProtectionAlgorithms*/
    /*Get ext*/
    getbit(bytes, &(v->integrityProtectionAlgorithms.ext));
    if(v->integrityProtectionAlgorithms.ext==0 ){
        getbits(&bits, bytes, 16);
        memcpy(&(v->integrityProtectionAlgorithms.v), bits.data, 2);
        v->integrityProtectionAlgorithms.v = ntohs(v->integrityProtectionAlgorithms.v);
    }else{
        s1ap_msg(WARN, 0, "integrityProtectionAlgorithms extension decoder not implemented yet.");
    }

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==0x01){
        v->iEext->addIe(v->iEext, dec_protocolIEs(bytes));
    }

    /*Extensions*/
    if(v->ext!=0){
        s1ap_msg(ERROR, 0, "Extensions are present, decoding not included in current version.\n");
        /*TODO extensions encoding
        getextension(&extensions, bytes, item->ext);
        skipextensions(bytes, 1, &extensions);*/
    }
}

void dec_UERadioCapability(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    struct BinaryData bits;
    UERadioCapability_t *v;

    v = new_UERadioCapability();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Decode len*/
    v->len = decode_length_undef(bytes);

    /*Decode String*/
    v->str = malloc(v->len);
    if(v->str!=NULL){
        bits.data = v->str;
        getoctets(&bits, bytes, v->len);
    }else{
        s1ap_msg(ERROR, 0, "Couldn't allocate UERadioCapability string memory.");
    }
}

void dec_UE_S1AP_IDs(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    S1AP_PROTOCOL_IES_t fakeie;
    UE_S1AP_IDs_t *v = new_UE_S1AP_IDs();
    ie->value=v;

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Get Extension flag*/
    getbit(bytes, &(v->ext));

    /*Get Extension choice*/
    v->choice = getchoice(bytes, 2, v->ext);

    //align_dec(bytes);
    switch(v->choice+v->ext*2){
    case 0:
        /* UE-S1AP-ID-pair*/
        v->uE_S1AP_ID.uE_S1AP_ID_pair = dec_UE_S1AP_ID_pair(bytes);
        break;
    case 1:
        /*MME-UE-S1AP-ID*/
        dec_MME_UE_S1AP_ID(&fakeie, bytes);
        v->uE_S1AP_ID.mME_UE_S1AP_ID = fakeie.value;
        break;
    default:
        s1ap_msg(WARN, 0, "Extension not implemented\n");
        /*Extension not implemented*/
        break;
    }
}

void dec_ENB_StatusTransfer_TransparentContainer(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    struct BinaryData extensions;
    ENB_StatusTransfer_TransparentContainer_t *v = new_ENB_StatusTransfer_TransparentContainer();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Get extension flag*/
    getbit(bytes, &(v->ext));

    /*Get optionals*/
    getbit(bytes, &(v->opt));

    /*Decode Bearers-SubjectToStatusTransferList*/
    v->bearers_SubjectToStatusTransferList = dec_Bearers_SubjectToStatusTransferList(bytes);

    /* attribute number 2 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==0x01){
     v->iEext->addIe(v->iEext, dec_protocolIEs(bytes));
    }

    /*Extensions*/
    getextension(&extensions, bytes, v->ext);
    skipextensions(bytes, 1, &extensions);
}

void dec_Bearers_SubjectToStatusTransfer_Item(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    struct BinaryData opt, extensions;
    uint8_t buffer[MAXDATABYTES];

    Bearers_SubjectToStatusTransfer_Item_t *v = new_Bearers_SubjectToStatusTransfer_Item();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Get extension flag*/
    getbit(bytes, &(v->ext));

    /*Get optionals*/
    opt.data=buffer;
    getbits(&opt, bytes, 2);
    v->opt=buffer[0]&0xC0;

    /*attribute number 1 with type E_RAB_ID_t*/
    dec_E_RAB_ID(&(v->eRAB_ID), bytes);

    /* attribute number 3  with type COUNTvalue */
    v->uL_COUNTvalue = dec_COUNTvalue(bytes);
    /* attribute number 4  with type COUNTvalue */
    v->dL_COUNTvalue = dec_COUNTvalue(bytes);

    /* attribute number 5  with type ReceiveStatusofULPDCPSDUs */
    if((v->opt&0x80)==0x80){
     /*v->receiveStatusofULPDCPSDUs*/
        s1ap_msg(WARN, 0, "Optional ReceiveStatusofULPDCPSDUs is present, parsing not included in current version.");
    }

    /* attribute number 6 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if((v->opt&0x40)==0x40){
        v->iEext->addIe(v->iEext, dec_protocolIEs(bytes));
    }

    /*Extensions*/
    getextension(&extensions, bytes, v->ext);
    skipextensions(bytes, 1, &extensions);
}

void dec_E_RABItem(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    struct BinaryData extensions;
    S1AP_PROTOCOL_IES_t fakeie;
    E_RABItem_t *v = new_E_RABItem();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Get extension flag*/
    getbit(bytes, &(v->ext));

    /*Get optionals*/
    getbit(bytes, &(v->opt));

    /*attribute number 1 with type E_RAB_ID_t*/
    dec_E_RAB_ID(&(v->eRAB_ID), bytes);

    /*attribute number 2 with type Cause*/
    dec_Cause(&fakeie, bytes);
    v->cause=fakeie.value;

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==0x01){
     v->iEext->addIe(v->iEext, dec_protocolIEs(bytes));
    }

    /*Extensions*/
    getextension(&extensions, bytes, v->ext);
    skipextensions(bytes, 1, &extensions);
}

void dec_E_RABList(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    uint8_t length, i;
    S1AP_PROTOCOL_IES_t *fakeie;
    E_RABList_t *v = new_E_RABList();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Decode length*/
    length = decode_constrained_number(bytes, 1, maxNrOfERABs);

    /*Decode E_RABToBeSetupItemCtxtSUReq_t*/
    for(i=0 ; i < length ; i++){
        fakeie = dec_protocolIEs(bytes);
        v->additem(v, (ProtocolIE_SingleContainer_t*)fakeie);
    }
}

void dec_E_RABToBeModifiedItemBearerModReq(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    struct BinaryData extensions;
    S1AP_PROTOCOL_IES_t fakeie;
    E_RABToBeModifiedItemBearerModReq_t *v = new_E_RABToBeModifiedItemBearerModReq();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Get extension flag*/
    getbit(bytes, &(v->ext));

    /*Get optionals*/
    getbit(bytes, &(v->opt));

    /*attribute number 1 with type E_RAB_ID_t*/
    dec_E_RAB_ID(&(v->eRAB_ID), bytes);

    /*attribute number 2 with type E_RABLevelQoSParameters*/
    v->eRABLevelQoSParameters = dec_E_RABLevelQoSParameters(bytes);

    /*attribute number 3 with type Cause*/
    dec_Unconstrained_Octed_String(&fakeie, bytes);
    v->nAS_PDU=fakeie.value;

    /* attribute number 4 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==0x01){
     v->iEext->addIe(v->iEext, dec_protocolIEs(bytes));
    }

    /*Extensions*/
    getextension(&extensions, bytes, v->ext);
    skipextensions(bytes, 1, &extensions);
}

void dec_E_RABToBeModifiedListBearerModReq(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    uint8_t length, i;
    S1AP_PROTOCOL_IES_t *fakeie;
    E_RABToBeModifiedListBearerModReq_t *v = new_E_RABToBeModifiedListBearerModReq();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Decode length*/
    length = decode_constrained_number(bytes, 1, maxNrOfERABs);

    /*Decode E_RABToBeSetupItemCtxtSUReq_t*/
    for(i=0 ; i < length ; i++){
        fakeie = dec_protocolIEs(bytes);
        v->additem(v, (ProtocolIE_SingleContainer_t*)fakeie);
    }
}

void dec_E_RABToBeSetupItemBearerSUReq(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    struct BinaryData extensions;
    S1AP_PROTOCOL_IES_t fakeie;
    E_RABToBeSetupItemBearerSUReq_t *v = new_E_RABToBeSetupItemBearerSUReq();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Get extension flag*/
    getbit(bytes, &(v->ext));

    /*Get optionals*/
    getbit(bytes, &(v->opt));

    /*attribute number 1 with type E_RAB_ID_t*/
    dec_E_RAB_ID(&(v->eRAB_ID), bytes);

    /*attribute number 2 with type E_RABLevelQoSParameters*/
    v->eRABLevelQoSParameters = dec_E_RABLevelQoSParameters(bytes);

    /*attribute number 3 with type TransportLayerAddress*/
    v->transportLayerAddress = dec_TransportLayerAddress(bytes);

    /*attribute number 4 with type GTP_TEID*/
    decode_octet_string(v->gTP_TEID.teid, bytes, 4);

    /*attribute number 5 with type NAS_PDU*/
    dec_Unconstrained_Octed_String(&fakeie, bytes);
    v->nAS_PDU=fakeie.value;

    /* attribute number 6 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==0x01){
     v->iEext->addIe(v->iEext, dec_protocolIEs(bytes));
    }

    /*Extensions*/
    getextension(&extensions, bytes, v->ext);
    skipextensions(bytes, 1, &extensions);
}

void dec_E_RABToBeSetupListBearerSUReq(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    uint8_t length, i;
    S1AP_PROTOCOL_IES_t *fakeie;
    E_RABToBeModifiedListBearerModReq_t *v = new_E_RABToBeModifiedListBearerModReq();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Decode length*/
    length = decode_constrained_number(bytes, 1, maxNrOfERABs);

    /*Decode E_RABToBeSetupItemCtxtSUReq_t*/
    for(i=0 ; i < length ; i++){
        fakeie = dec_protocolIEs(bytes);
        v->additem(v, (ProtocolIE_SingleContainer_t*)fakeie);
    }
}

void dec_E_RABSetupItemBearerSURes(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    struct BinaryData extensions;
    E_RABSetupItemBearerSURes_t *v = new_E_RABSetupItemBearerSURes();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Get extension flag*/
    getbit(bytes, &(v->ext));

    /*Get optionals*/
    getbit(bytes, &(v->opt));

    /*attribute number 1 with type E_RAB_ID_t*/
    dec_E_RAB_ID(&(v->eRAB_ID), bytes);

    /*attribute number 2 with type TransportLayerAddress*/
    v->transportLayerAddress = dec_TransportLayerAddress(bytes);

    /*attribute number 3 with type GTP_TEID*/
    decode_octet_string(v->gTP_TEID.teid, bytes, 4);

    /* attribute number 4 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==0x01){
     v->iEext->addIe(v->iEext, dec_protocolIEs(bytes));
    }

    /*Extensions*/
    getextension(&extensions, bytes, v->ext);
    skipextensions(bytes, 1, &extensions);
}

void dec_E_RABSetupListBearerSURes(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    uint8_t length, i;
    S1AP_PROTOCOL_IES_t *fakeie;
    E_RABSetupListBearerSURes_t *v = new_E_RABSetupListBearerSURes();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Decode length*/
    length = decode_constrained_number(bytes, 1, maxNrOfERABs);

    /*Decode E_RABSetupItemCtxtSUReq_t*/
    for(i=0 ; i < length ; i++){
        fakeie = dec_protocolIEs(bytes);
        v->additem(v, (ProtocolIE_SingleContainer_t*)fakeie);
    }
}

void dec_HandoverType(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    HandoverType_t *v = new_HandoverType();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Get extension flag*/
    getbit(bytes, &(v->ext));

    if(v->ext==0){
        v->ht.noext = decode_constrained_number(bytes, 0, 4);
    }
    else{
        s1ap_msg(WARN, 0, "Handover Type with extension detected. Current version doesn't support it.");
        /*v->ht.ext = decode_small_number(bytes);*/
    }
}

void dec_LAI(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    struct BinaryData extensions;
    LAI_t *v = new_LAI();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Get extension flag*/
    getbit(bytes, &(v->ext));

    /*Get optionals*/
    getbit(bytes, &(v->opt));

    /*attribute number 1 (pLMNidentity) with type OCTET STRING */
    v->pLMNidentity = dec_PLMNidentity(bytes);

    /*attribute number 2 (pLMNidentity) with type OCTET STRING */
    decode_octet_string(v->lAC.s, bytes, 2);

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==0x01){
        v->iEext->addIe(v->iEext, dec_protocolIEs(bytes));
    }

    /*Extensions*/
    getextension(&extensions, bytes, v->ext);
    skipextensions(bytes, 1, &extensions);
}

void dec_TargetID(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    TargetID_t *v = new_TargetID();
    ie->value=v;

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Get Extension flag*/
    getbit(bytes, &(v->ext));

    /*Get Extension choice*/
    v->choice = getchoice(bytes, 3, v->ext);

    //align_dec(bytes);
    switch(v->choice+v->ext*2){
    case 0:
        /* UE-S1AP-ID-pair*/
        v->targetID.targeteNB_ID = dec_TargeteNB_ID(bytes);
        break;
    case 1:
        /*MME-UE-S1AP-ID*/
        v->targetID.targetRNC_ID = dec_TargetRNC_ID(bytes);
        break;
    case 2:
        /*MME-UE-S1AP-ID*/
        v->targetID.cGI = dec_CGI(bytes);
        break;
    default:
        s1ap_msg(WARN, 0, "Extension not implemented\n");
        /*Extension not implemented*/
        break;
    }
}

void dec_Direct_Forwarding_Path_Availability(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    Direct_Forwarding_Path_Availability_t *v = new_Direct_Forwarding_Path_Availability();

    /*Link functions*/
    ie->freeValue = v->freeIE;
    ie->showValue = v->showIE;
    ie->value=v;

    /*Get Extension flag*/
    getbit(bytes, &v->ext);

    if(v->ext!=0){
        v->ind = decode_small_number(bytes);
    }
}

void dec_E_RAB_IE_ContainerList(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    ProtocolIE_Container_t *v = new_ProtocolIE_Container();
    S1AP_PROTOCOL_IES_t *elem;
    uint32_t len, i;

    typedef void (*cast_cb)(void *);

    /*Link functions*/
    ie->freeValue = (cast_cb) v->freeContainer;
    ie->showValue = (cast_cb) v->showIEs;
    ie->value=v;

    /*getbit(bytes, &ext);*/

    /*attribute number 1 with type SEQUENCE OF*/
    len = decode_constrained_number(bytes, 1, maxNrOfERABs);

    /*Decode IEs*/
    for(i=0; i<len;i++){
        elem = dec_protocolIEs(bytes);
        if(elem!=NULL){
            v->addIe(v, elem);
        }
    }
}

void dec_E_RABToBeSetupItemHOReq(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    struct BinaryData extensions;
    E_RABToBeSetupItemHOReq_t *v = new_E_RABToBeSetupItemHOReq();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Get extension flag*/
    getbit(bytes, &(v->ext));

    /*Get optionals*/
    getbit(bytes, &(v->opt));

    /*attribute number 1 with type E_RAB_ID_t*/
    dec_E_RAB_ID(&(v->eRAB_ID), bytes);

    /*attribute number 1 with type TransportLayerAddress*/
    v->transportLayerAddress = dec_TransportLayerAddress(bytes);

    /*attribute number 3 with type GTP_TEID*/
    decode_octet_string(v->gTP_TEID.teid, bytes, 4);

    /*attribute number 4 with type E_RABLevelQoSParameters*/
    v->eRABLevelQoSParameters = dec_E_RABLevelQoSParameters(bytes);

    /* attribute number 5 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==0x01){
     v->iEext->addIe(v->iEext, dec_protocolIEs(bytes));
    }

    /*Extensions*/
    getextension(&extensions, bytes, v->ext);
    skipextensions(bytes, 1, &extensions);
}

void dec_SecurityContext(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    S1AP_PROTOCOL_IES_t fakeie;
    struct BinaryData extensions;
    SecurityContext_t *v = new_SecurityContext();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Get extension flag*/
    getbit(bytes, &(v->ext));

    /*Get optionals*/
    getbit(bytes, &(v->opt));

    /*attribute number 1 with type INTEGER (0 .. 7)*/
    v->nextHopChainingCount = decode_constrained_number(bytes, 0, 7);

    /*attribute number 2 with type SecurityKey*/
    dec_SecurityKey(&fakeie, bytes);
    v->nextHopParameter = fakeie.value;

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==0x01){
        v->iEext->addIe(v->iEext, dec_protocolIEs(bytes));
    }

    /*Extensions*/
    getextension(&extensions, bytes, v->ext);
    skipextensions(bytes, 1, &extensions);
}

void dec_E_RABAdmittedItem(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    struct BinaryData opt, extensions;
    uint8_t buffer[MAXDATABYTES];
    E_RABAdmittedItem_t *v = new_E_RABAdmittedItem();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Get extension flag*/
    getbit(bytes, &(v->ext));

    /*Get optionals*/
    opt.data=buffer;
    getbits(&opt, bytes, 5);
    v->opt=opt.data[0];

    /*attribute number 1 with type E_RAB_ID_t*/
    dec_E_RAB_ID(&(v->eRAB_ID), bytes);

    /*attribute number 2 with type TransportLayerAddress*/
    v->transportLayerAddress = dec_TransportLayerAddress(bytes);

    /*attribute number 3 with type GTP_TEID*/
    decode_octet_string(v->gTP_TEID.teid, bytes, 4);

    /*attribute number 4 with type TransportLayerAddress*/
    if((v->opt&0x80)==0x80){
        v->dL_transportLayerAddress = dec_TransportLayerAddress(bytes);
    }

    /*attribute number 5 with type GTP_TEID*/
    if((v->opt&0x40)==0x40){
        decode_octet_string(v->dL_GTP_TEID.teid, bytes, 4);
    }

    /*attribute number 6 with type TransportLayerAddress*/
    if((v->opt&0x20)==0x20){
        v->uL_transportLayerAddress = dec_TransportLayerAddress(bytes);
    }

    /*attribute number 7 with type GTP_TEID*/
    if((v->opt&0x10)==0x10){
        decode_octet_string(v->uL_GTP_TEID.teid, bytes, 4);
    }

    /* attribute number 8 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if((v->opt&0x08)==0x08){
     v->iEext->addIe(v->iEext, dec_protocolIEs(bytes));
    }

    /*Extensions*/
    getextension(&extensions, bytes, v->ext);
    skipextensions(bytes, 1, &extensions);
}

void dec_E_RABDataForwardingItem(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    struct BinaryData opt, extensions;
    uint8_t buffer[MAXDATABYTES];
    E_RABDataForwardingItem_t *v = new_E_RABDataForwardingItem();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Get extension flag*/
    getbit(bytes, &(v->ext));

    /*Get optionals*/
    opt.data=buffer;
    getbits(&opt, bytes, 5);
    v->opt=opt.data[0];

    /*attribute number 1 with type E_RAB_ID_t*/
    dec_E_RAB_ID(&(v->eRAB_ID), bytes);

    /*attribute number 2 with type TransportLayerAddress*/
    if((v->opt&0x20)==0x20){
        v->dL_transportLayerAddress = dec_TransportLayerAddress(bytes);
    }

    /*attribute number 3 with type GTP_TEID*/
    if((v->opt&0x40)==0x40){
        decode_octet_string(v->dL_GTP_TEID.teid, bytes, 4);
    }

    /*attribute number 4 with type TransportLayerAddress*/
    if((v->opt&0x20)==0x20){
        v->uL_transportLayerAddress = dec_TransportLayerAddress(bytes);
    }

    /*attribute number 5 with type GTP_TEID*/
    if((v->opt&0x10)==0x10){
        decode_octet_string(v->uL_GTP_TEID.teid, bytes, 4);
    }

    /* attribute number 6 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if((v->opt&0x08)==0x08){
     v->iEext->addIe(v->iEext, dec_protocolIEs(bytes));
    }

    /*Extensions*/
    getextension(&extensions, bytes, v->ext);
    skipextensions(bytes, 1, &extensions);
}


S_TMSI_t *dec_S_TMSI(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){
    struct BinaryData extensions, opt;
    uint8_t buffer[MAXDATABYTES];
    S_TMSI_t *v;

    v = new_S_TMSI();

    /*Link functions*/
    ie->showValue = v->showIE;
    ie->freeValue = v->freeIE;
    ie->value=v;

    /*Get extension flag*/
    getbit(bytes, &(v->ext));

    /*Get optionals*/
    getbit(bytes, &(v->opt));

    /*attribute number 1 (mMEC) with type MME-Code */
    v->mMEC = dec_MME_Code(bytes);

    /*attribute number 2 (m_TMSI) with type OCTET STRING */
    decode_octet_string(v->m_TMSI.s, bytes, 4);

    /* attribute number 3 (iE-Extensions) with type (ProtocolExtensionContainer) SEQUENCE OF */
    if(v->opt==0x01){
        v->iEext->addIe(v->iEext, dec_protocolIEs(bytes));
    }

    /*Extensions*/
    getextension(&extensions, bytes, v->ext);
    skipextensions(bytes, 1, &extensions);

    return v;
}


const getDecS1AP_IE getdec_S1AP_IE[] = {
        dec_MME_UE_S1AP_ID,/*"id-MME-UE-S1AP-ID"*/
        dec_HandoverType,/*"id-HandoverType"*/
        dec_Cause,/*"id-Cause"*/
        NULL,/*"id-SourceID"*/
        dec_TargetID,/*"id-TargetID"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-undefined"*/
        dec_ENB_UE_S1AP_ID,/*"id-eNB-UE-S1AP-ID"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-undefined"*/
        dec_E_RABList,/*"id-E-RABSubjecttoDataForwardingList"*/
        dec_E_RABList,/*"id-E-RABtoReleaseListHOCmd"*/
        dec_E_RABDataForwardingItem,/*"id-E-RABDataForwardingItem"*/
        NULL,/*"id-E-RABReleaseItemBearerRelComp"*/
        dec_E_RABToBeSetupListBearerSUReq,/*"id-E-RABToBeSetupListBearerSUReq"*/
        dec_E_RABToBeSetupItemBearerSUReq,/*"id-E-RABToBeSetupItemBearerSUReq"*/
        dec_E_RABList,/*"id-E-RABAdmittedList"*/
        NULL,/*"id-E-RABFailedToSetupListHOReqAck"*/
        dec_E_RABAdmittedItem,/*"id-E-RABAdmittedItem"*/
        NULL,/*"id-E-RABFailedtoSetupItemHOReqAck"*/
        dec_E_RABSetupListBearerSURes,/*"id-E-RABToBeSwitchedDLList"*/
        dec_E_RABSetupItemBearerSURes,/*"id-E-RABToBeSwitchedDLItem"*/
        dec_E_RABToBeSetupListCtxtSUReq,/*"id-E-RABToBeSetupListCtxtSUReq"*/
        NULL,/*"id-TraceActivation"*/
        dec_Unconstrained_Octed_String,/*"id-NAS-PDU"*/
        dec_E_RABToBeSetupItemHOReq,/*"id-E-RABToBeSetupItemHOReq"*/
        dec_E_RABSetupListBearerSURes,/*"id-E-RABSetupListBearerSURes"*/
        dec_E_RABList,/*"id-E-RABFailedToSetupListBearerSURes"*/
        dec_E_RABToBeModifiedListBearerModReq,/*"id-E-RABToBeModifiedListBearerModReq"*/
        NULL,/*"id-E-RABModifyListBearerModRes"*/
        dec_E_RABList,/*"id-E-RABFailedToModifyList"*/
        dec_E_RABList,/*"id-E-RABToBeReleasedList"*/
        dec_E_RABList,/*"id-E-RABFailedToReleaseList"*/
        dec_E_RABItem,/*"id-E-RABItem"*/
        dec_E_RABToBeModifiedItemBearerModReq,/*"id-E-RABToBeModifiedItemBearerModReq"*/
        NULL,/*"id-E-RABModifyItemBearerModRes"*/
        NULL,/*"id-E-RABReleaseItem"*/
        dec_E_RABSetupItemBearerSURes,/*"id-E-RABSetupItemBearerSURes"*/
        dec_SecurityContext,/*"id-SecurityContext"*/
        NULL,/*"id-HandoverRestrictionList"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-UEPagingID"*/
        NULL,/*"id-pagingDRX"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-TAIList"*/
        NULL,/*"id-TAIItem"*/
        dec_E_RABList,/*"id-E-RABFailedToSetupListCtxtSURes"*/
        NULL,/*"id-E-RABReleaseItemHOCmd"*/
        dec_E_RABSetupItemBearerSURes,/*"id-E-RABSetupItemCtxtSURes"*/
        dec_E_RABSetupListBearerSURes,/*"id-E-RABSetupListCtxtSURes"*/
        dec_E_RABToBeSetupItemCtxtSUReq,/*"id-E-RABToBeSetupItemCtxtSUReq"*/
        dec_E_RAB_IE_ContainerList,/*"id-E-RABToBeSetupListHOReq"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-GERANtoLTEHOInformationRes"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-UTRANtoLTEHOInformationRes"*/
        dec_CriticalityDiagnostics,/*"id-CriticalityDiagnostics"*/
        dec_Global_ENB_ID,/*"id-Global-ENB-ID"*/
        dec_ENBname,/*"id-eNBname"*/
        dec_MMEname,/*"id-MMEname"*/
        NULL,/*"id-undefined"*/
        dec_servedPLMNs,/*"id-ServedPLMNs"*/
        dec_SupportedTAs,/*"id-SupportedTAs"*/
        dec_TimeToWait,/*"id-TimeToWait"*/
        dec_UEAggregateMaximumBitrate,/*"id-uEaggregateMaximumBitrate"*/
        dec_TAI,/*"id-TAI"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-E-RABReleaseListBearerRelComp"*/
        NULL,/*"id-cdma2000PDU"*/
        NULL,/*"id-cdma2000RATType"*/
        NULL,/*"id-cdma2000SectorID"*/
        dec_SecurityKey,/*"id-SecurityKey"*/
        dec_UERadioCapability,/*"id-UERadioCapability"*/
        NULL,/*"id-GUMMEI-ID"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-E-RABInformationListItem"*/
        dec_Direct_Forwarding_Path_Availability,/*"id-Direct-Forwarding-Path-Availability"*/
        NULL,/*"id-UEIdentityIndexValue"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-cdma2000HOStatus"*/
        NULL,/*"id-cdma2000HORequiredIndication"*/
        NULL,/*"id-undefined"*/
        NULL,/*"id-E-UTRAN-Trace-ID"*/
        dec_RelativeMMECapacity,/*"id-RelativeMMECapacity"*/
        dec_MME_UE_S1AP_ID,/*"id-SourceMME-UE-S1AP-ID"*/
        dec_Bearers_SubjectToStatusTransfer_Item,/*"id-Bearers-SubjectToStatusTransfer-Item"*/
        dec_ENB_StatusTransfer_TransparentContainer,/*"id-eNB-StatusTransfer-TransparentContainer"*/
        NULL,/*"id-UE-associatedLogicalS1-ConnectionItem"*/
        NULL,/*"id-ResetType"*/
        NULL,/*"id-UE-associatedLogicalS1-ConnectionListResAck"*/
        dec_E_RABSetupItemBearerSURes,/*"id-E-RABToBeSwitchedULItem"*/
        dec_E_RABSetupListBearerSURes,/*"id-E-RABToBeSwitchedULList"*/
        dec_S_TMSI,/*"id-S-TMSI"*/
        NULL,/*"id-cdma2000OneXRAND"*/
        NULL,/*"id-RequestType"*/
        dec_UE_S1AP_IDs,/*"id-UE-S1AP-IDs"*/
        dec_EUTRAN_CGI,/*"id-EUTRAN-CGI"*/
        NULL,/*"id-OverloadResponse"*/
        NULL,/*"id-cdma2000OneXSRVCCInfo"*/
        NULL,/*"id-E-RABFailedToBeReleasedList"*/
        dec_Unconstrained_Octed_String,/*"id-Source-ToTarget-TransparentContainer"*/
        dec_ServedGUMMEIs,/*"id-ServedGUMMEIs"*/
        dec_SubscriberProfileIDforRFP,/*"id-SubscriberProfileIDforRFP"*/
        dec_UESecurityCapabilities,/*"id-UESecurityCapabilities"*/
        NULL,/*"id-CSFallbackIndicator"*/
        NULL,/*"id-CNDomain"*/
        dec_E_RABList,/*"id-E-RABReleasedList"*/
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
        dec_Unconstrained_Octed_String,/*"id-Target-ToSource-TransparentContainer"*/
        NULL,/*"id-SRVCCOperationPossible"*/
        NULL,/*"id-SRVCCHOIndication"*/
        NULL,/*"id-NAS-DownlinkCount"*/
        dec_CSGid,/*"id-CSG-Id"*/
        dec_CSGidList,/*"id-CSG-IdList"*/
        NULL,/*"id-SONConfigurationTransferECT"*/
        NULL,/*"id-SONConfigurationTransferMCT"*/
        NULL,/*"id-TraceCollectionEntityIPAddress"*/
        NULL,/*"id-MSClassmark2"*/
        NULL,/*"id-MSClassmark3"*/
        dec_RRC_Establishment_Cause,/*"id-RRC-Establishment-Cause"*/
        dec_Unconstrained_Octed_String,/*"id-NASSecurityParametersfromE-UTRAN"*/
        dec_Unconstrained_Octed_String,/*"id-NASSecurityParameterstoE-UTRAN"*/
        dec_PagingDRX,/*"id-DefaultPagingDRX"*/
        dec_Unconstrained_Octed_String,/*"id-Source-ToTarget-TransparentContainer-Secondary"*/
        dec_Unconstrained_Octed_String,/*"id-Target-ToSource-TransparentContainer-Secondary"*/
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
        dec_LAI,/*"id-RegisteredLAI"*/
        NULL,/*"id-RelayNode-Indicator"*/
        NULL,/*"id-TrafficLoadReductionIndication"*/
        NULL,/*"id-MDTConfiguration"*/
        dec_MMERelaySupportIndicator,/*"id-MMERelaySupportIndicator"*/
        NULL,/*"id-GWContextReleaseIndication"*/
        NULL,/*"id-ManagementBasedMDTAllowed"*/
        NULL,/*"id-PrivacyIndicator"*/
        NULL,/*"id-Time-UE-StayedInCell-EnhancedGranularity"*/
        NULL,/*"id-HO-Cause"*/
        NULL,/*"id-VoiceSupportMatchIndicator"*/
        NULL,/*"id-GUMMEIType"*/
};

S1AP_PROTOCOL_IES_t *dec_protocolIEs(struct BinaryData *bytes){

    S1AP_PROTOCOL_IES_t * ie;
    getDecS1AP_IE iedec;
    struct BinaryData Tmpterm1;
    uint8_t buffer[MAXDATABYTES];

    Tmpterm1.data = buffer;

    ie = newProtocolIE();

    if(!ie){
        printf("dec_S1SetupRequest_protocolIEs_ProtocolIE_Field(): Error IE not allocated\n");
    }

    /*attribute number 1 with type id*/
    ie->id = decode_constrained_number(bytes, 0, 65535);
    /*printf("test id %s\n", IEName[ie->id]);*/
    /*attribute number 2 with type criticality*/
    ie->criticality = decode_enumerated(bytes, 0, 2);

    /*printf("\n*IE INFO: %s (%u)\ncriticality = %s (%u)\n", IEName[ie->id], ie->id, CriticalityName[ie->criticality], ie->criticality);*/

    /*attribute number 3 with type Value*/
    decode_open_type(&Tmpterm1, bytes);

    /*s1ap_msg(DEB, 0,"Tmpterm1.length = %u, bytes.length = %u\n", Tmpterm1.length, bytes->length);*/
    iedec = getdec_S1AP_IE[ie->id];
    if(iedec == NULL){
        s1ap_msg(ERROR, 0,"function decoder for IE #%u not found", ie->id);
        return ie;
    }
    iedec(ie, &Tmpterm1);

    return ie;
/*
 'getdec_internal_object_set_argument_33'(id,59) ->
   fun(Type, Val, _, _) ->
      case Type of
         'Value' ->
            'dec_Global-ENB-ID'(Val, telltype)
      end
   end;
'getdec_internal_object_set_argument_33'(id,60) ->
   fun(Type, Val, _, _) ->
      case Type of
         'Value' ->
            'dec_ENBname'(Val, telltype)
      end
   end;
'getdec_internal_object_set_argument_33'(id,64) ->
   fun(Type, Val, _, _) ->
      case Type of
         'Value' ->
            'dec_SupportedTAs'(Val, telltype)
      end
   end;
'getdec_internal_object_set_argument_33'(id,128) ->
   fun(Type, Val, _, _) ->
      case Type of
         'Value' ->
            'dec_CSG-IdList'(Val, telltype)
      end
   end;
'getdec_internal_object_set_argument_33'(id,137) ->
   fun(Type, Val, _, _) ->
      case Type of
         'Value' ->
            'dec_PagingDRX'(Val, telltype)
      end
   end;
'getdec_internal_object_set_argument_33'(id, ErrV) ->
  fun(C,V,_,_) -> exit({{component,C},{value,V},{unique_name_and_value,id,ErrV}}) end.


'dec_S1SetupRequest_protocolIEs_ProtocolIE-Field'(Bytes,_) ->

%%  attribute number 1 with type id
{Term1,Bytes1} = ?RT_PER:decode_integer(Bytes,[{'ValueRange',{0,65535}}]),

%%  attribute number 2 with type criticality
{Term2,Bytes2} = ?RT_PER:decode_enumerated(Bytes1,[{'ValueRange',{0,2}}],{reject,ignore,notify}),

%%  attribute number 3 with type Value
{Tmpterm1, Bytes3} = ?RT_PER:decode_open_type(Bytes2, []),
DecObjidTerm1 =
   'S1AP':'getdec_internal_object_set_argument_33'(id, Term1),
Term3 =
   case (catch DecObjidTerm1('Value', Tmpterm1, telltype,[])) of
      {'EXIT', Reason1} ->
         exit({'Type not compatible with table constraint',Reason1});
      {Tmpterm2,_} ->
         Tmpterm2
   end,

{{'ProtocolIE-Field',Term1,Term2,Term3},Bytes3}.

*/
}
