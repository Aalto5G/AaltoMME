/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S1AP.c
 * @Author Vicent Ferrer
 * @date   March, 2013
 * @brief  S1AP definition
 *
 * This module implements the S1AP 3GPP protocol standard.
 */

#include <stdio.h>
#include <string.h>

#include "S1AP.h"
#include "S1APlog.h"
#include "rt_per_bin.h"
#include "S1AP_IEdec.h"
#include "S1AP_IEenc.h"




void info(){
    printf("\n*************** Hello World ***************\n\n");
}

/*  ********************   Procedures decoding ********************   */
void dec_ElementaryProcedure(S1AP_PDU_t *pdu, struct BinaryData * bytes){
    ProtocolIE_Container_t * ieContainer;
    S1AP_PROTOCOL_IES_t *ie;
    uint8_t ext;
    uint32_t len, i;

    ieContainer = pdu->value;
    getbit(bytes, &ext);

    /*printf("dec_S1SetupRequest():after ext reading\nbytes->pos=%u, 0x%u %u %u\n", bytes->pos, bytes->data[0], bytes->data[1], bytes->data[2]);    */
    /*attribute number 1 with type SEQUENCE OF*/
    len = decode_constrained_number(bytes, 0, 65535);

    /*s1ap_msg(DEBUG, 0,"ext = %u, attr#1 num = %u ", ext, ieContainer->size);*/
    /*Decode IEs*/
    for(i=0; i<len;i++){
        ie = dec_protocolIEs(bytes);
        if(ie!=NULL){
            ieContainer->addIe(ieContainer, ie);
        }
    }

/*
'dec_S1SetupRequest_protocolIEs'(Bytes,_) ->

{Num,Bytes1} = ?RT_PER:decode_length(Bytes,{0,65535}),
'dec_S1SetupRequest_protocolIEs_components'(Num, Bytes1, telltype, []).

'dec_S1SetupRequest_protocolIEs_components'(0, Bytes, _, Acc) ->
   {lists:reverse(Acc), Bytes};
'dec_S1SetupRequest_protocolIEs_components'(Num, Bytes, _, Acc) ->
   {Term,Remain} = 'dec_S1SetupRequest_protocolIEs_ProtocolIE-Field'(Bytes, telltype),
   'dec_S1SetupRequest_protocolIEs_components'(Num-1, Remain, telltype, [Term|Acc])
*/
/*
'dec_S1SetupRequest'(Bytes,_) ->
{Ext,Bytes1} = ?RT_PER:getext(Bytes),

%% attribute number 1 with type SEQUENCE OF
{Term1,Bytes2} = 'dec_S1SetupRequest_protocolIEs'(Bytes1, telltype),
{Extensions,Bytes3} = ?RT_PER:getextension(Ext,Bytes2),
Bytes4= ?RT_PER:skipextensions(Bytes3,1,Extensions)
,
{{'S1SetupRequest',Term1},Bytes4}.
*/
}

void dec_S1AP_PDU(S1AP_PDU_t *pdu, struct BinaryData *bytes){

    ProcedureCode_e procedureCode;
    Criticality_e criticality;
    struct BinaryData Tmpterm1;
    uint8_t buffer[MAXDATABYTES];

    Tmpterm1.data = buffer;
    /*attribute number 1 with type procedureCode*/

    procedureCode = decode_constrained_number(bytes, 0, 255);
    pdu->procedureCode = procedureCode;

    /*attribute number 2 with type criticality*/
    criticality = decode_enumerated(bytes, 0, 2);
    pdu->criticality = criticality;

    /*attribute number 3 with type InitiatingMessage*/
    decode_open_type(&Tmpterm1, bytes);
    dec_ElementaryProcedure( pdu, &Tmpterm1);
    /*s1ap_msg(INFO, 0, "procedure code = %s (%u), criticality = %s", elementaryProcedureName[procedureCode], procedureCode, CriticalityName[criticality]);*/

    /*
    'dec_InitiatingMessage'(Bytes,_) ->

    %%  attribute number 1 with type procedureCode
    {Term1,Bytes1} = ?RT_PER:decode_integer(Bytes,[{'ValueRange',{0,255}}]),

    %%  attribute number 2 with type criticality
    {Term2,Bytes2} = ?RT_PER:decode_enumerated(Bytes1,[{'ValueRange',{0,2}}],{reject,ignore,notify}),

    %%  attribute number 3 with type InitiatingMessage
    {Tmpterm1, Bytes3} = ?RT_PER:decode_open_type(Bytes2, []),
    DecObjprocedureCodeTerm1 =
       'S1AP':'getdec_S1AP-ELEMENTARY-PROCEDURES'(procedureCode, Term1),
    Term3 =
       case (catch DecObjprocedureCodeTerm1('InitiatingMessage', Tmpterm1, telltype,[])) of
          {'EXIT', Reason1} ->
             exit({'Type not compatible with table constraint',Reason1});
          {Tmpterm2,_} ->
             Tmpterm2
       end,

    {{'InitiatingMessage',Term1,Term2,Term3},Bytes3}
     */
}

S1AP_Message_t *s1ap_decode(void* data, uint32_t size){

    S1AP_Message_t *msg;
    uint8_t ext;
    uint32_t choice;
    struct BinaryData bytes;

    msg = S1AP_newMsg();
    bytes.pos=0;
    bytes.data = (uint8_t *)data;
    bytes.offset = (uint8_t *)data;
    bytes.length = size*8;
    getbit(&bytes, &ext);
    msg->extension = ext;

    choice = getchoice(&bytes, 3, ext);
    msg->choice = choice;

    if((msg->choice + ext*3)<3){
        dec_S1AP_PDU(msg->pdu, &bytes);
    }else{
        s1ap_msg(ERROR, 0, "PDU extension not implemented yet.");
    }

    return msg;
    /*
    'dec_S1AP-PDU'(Bytes,_) ->
    {Ext,Bytes1} = ?RT_PER:getbit(Bytes),
    {Choice,Bytes2} = ?RT_PER:getchoice(Bytes1,3,Ext ),
    {Cname,{Val,NewBytes}} = case Choice + Ext*3 of
    0 -> {initiatingMessage,
    'dec_InitiatingMessage'(Bytes2,telltype)};
    1 -> {successfulOutcome,
    'dec_SuccessfulOutcome'(Bytes2,telltype)};
    2 -> {unsuccessfulOutcome,
    'dec_UnsuccessfulOutcome'(Bytes2,telltype)};
    _ -> {asn1_ExtAlt,
          fun() ->
              {XTerm,XBytes} = ?RT_PER:decode_open_type(Bytes2,[]),
              {binary_to_list(XTerm),XBytes}
          end()}
    end,

    {{Cname,Val},NewBytes}.
     */
}

/*  ********************   Procedures encoding ********************   */

void enc_ElementaryProcedure(struct BinaryData *bytes, S1AP_PDU_t *pdu){
    ProtocolIE_Container_t * ieContainer;

    uint32_t i;

    ieContainer = pdu->value;
    setbits(bytes, 1, pdu->ext);

    /*attribute number 1 with type SEQUENCE OF*/
    encode_constrained_number(bytes, ieContainer->size, 0, 65535);

    /*printf_buffer(bytes->data, bytes->lenght);*/
    /*Encode IEs*/
    for(i=0; i<ieContainer->size ; i++){
        enc_protocolIEs(bytes, ieContainer->elem[i]);
    }

/*
'enc_S1SetupRequest'(Val) ->
Val1 = ?RT_PER:list_to_record('S1SetupRequest', Val),
[
?RT_PER:setext(false),
%% attribute number 1 with type SEQUENCE OF
'enc_S1SetupRequest_protocolIEs'(?RT_PER:cindex(2,Val1,protocolIEs))].
 */
}

void enc_S1AP_PDU(struct BinaryData *bytes, S1AP_PDU_t *pdu){

    struct BinaryData tmp;
    uint8_t buff[10000];

    memset(buff, 0, 10000);
    tmp.data=buff;
    tmp.offset=buff;
    tmp.length=0;
    tmp.pos=0;

    /*attribute number 1 with type procedureCode */
    encode_constrained_number(bytes, pdu->procedureCode, 0, 255);

    /*attribute number 2 with type criticality*/
    encode_constrained_number(bytes, pdu->criticality, 0, 2);

    /*attribute number 3 with type Open Type*/
    enc_ElementaryProcedure(&tmp, pdu);
    /*tmp.data[0]=0x00;
    tmp.data[1]=0x04;
    tmp.data[2]=0x00;
    tmp.data[3]=0x3b;
    tmp.lenght=32;*/

    /*printf_buffer(tmp.data, tmp.lenght);*/
    encode_open_type(bytes, &tmp);

/*
'enc_InitiatingMessage'(Val) ->
Val1 = ?RT_PER:list_to_record('InitiatingMessage', Val),
ObjprocedureCode =
  'S1AP':'getenc_S1AP-ELEMENTARY-PROCEDURES'(procedureCode,
                                          ?RT_PER:cindex(2,Val1,procedureCode)),
[

%% attribute number 1 with type fixedtypevaluefieldprocedureCodetypeINTEGERValueRange0255no
?RT_PER:encode_integer([{'ValueRange',{0,255}}],?RT_PER:cindex(2,Val1,procedureCode)),

%% attribute number 2 with type fixedtypevaluefieldcriticalitytypeENUMERATEDreject0ignore1notify2no
case (case ?RT_PER:cindex(3,Val1,criticality) of {_,Enumval2}->Enumval2;_->?RT_PER:cindex(3,Val1,criticality) end) of
'reject' -> ?RT_PER:encode_integer([{'ValueRange',{0,2}}], 0);
'ignore' -> ?RT_PER:encode_integer([{'ValueRange',{0,2}}], 1);
'notify' -> ?RT_PER:encode_integer([{'ValueRange',{0,2}}], 2);

EnumVal -> exit({error,{asn1, {enumerated_not_in_range, EnumVal}}})
end,

%% attribute number 3 with type typefieldInitiatingMessage
?RT_PER:encode_open_type([],?RT_PER:complete(
   ObjprocedureCode('InitiatingMessage', ?RT_PER:cindex(4,Val1,value), [])))].
 */
}

void s1ap_encode(uint8_t* data, uint32_t *size, S1AP_Message_t *msg){

    struct BinaryData bytes;
    bytes.length=0;
    bytes.pos=0;
    bytes.data=data;
    bytes.offset=data;
    set_choice_ext(&bytes, msg->choice, 3, msg->extension);
    enc_S1AP_PDU(&bytes, msg->pdu);

    *size = bytes.length%8==0?bytes.length/8:bytes.length/8+1;
/*
'enc_S1AP-PDU'(Val) ->
[
?RT_PER:set_choice(element(1,Val),{[initiatingMessage,successfulOutcome,unsuccessfulOutcome],[]}, {3,0}),
case element(1,Val) of
initiatingMessage ->
'enc_InitiatingMessage'(element(2,Val));
successfulOutcome ->
'enc_SuccessfulOutcome'(element(2,Val));
unsuccessfulOutcome ->
'enc_UnsuccessfulOutcome'(element(2,Val))
end
]
*/
}

/*  ********************   Tool Functions ********************   */

void *s1ap_findIe(S1AP_Message_t *msg, ProtocolIE_ID_t id){
    uint16_t i, elem ;
    S1AP_PROTOCOL_IES_t *ie = NULL;

    /* Find IE*/
    for(i = 0; i<msg->pdu->value->size ; i++){
        if( ((S1AP_PROTOCOL_IES_t*)msg->pdu->value->elem[i])->id == id){
            elem = i;
        }
    }
    if(elem != msg->pdu->value->size){
        ie = (S1AP_PROTOCOL_IES_t*)(msg->pdu->value->elem[elem]);
    }


    if(ie!=NULL){
        return ie->value;
    }
    return NULL;

}

void *s1ap_getIe(S1AP_Message_t *msg, ProtocolIE_ID_t id){
    int i, elem=-1;
    S1AP_PROTOCOL_IES_t *ie = NULL;

    /* Find IE*/
    for(i = 0; i<msg->pdu->value->size ; i++){
        if( ((S1AP_PROTOCOL_IES_t*)msg->pdu->value->elem[i])->id == id){
            elem = i;
        }
    }
    if(elem==-1){
        s1ap_msg(DEB, 0, "IE #%d not found on the message", id);
        return NULL;
    }

    /* Extract IE*/
    if(elem != msg->pdu->value->size){
        ie = (S1AP_PROTOCOL_IES_t*)(msg->pdu->value->elem[elem]);
        /*msg->pdu->value->elem[elem] = NULL;*/
        for(i = elem+1; i<msg->pdu->value->size ; i++){
            msg->pdu->value->elem[i-1] = msg->pdu->value->elem[i];
        }
        msg->pdu->value->size--;
    }

    if(ie!=NULL){
        return ie->value;
    }
    return NULL;

}

/* ********************* s1ap_setValueOnNewIE ******************** */
/** @brief locate the Value struct on a new IE of the message
 *  @param [in] s1msg   s1 message structure pointer
 *  @param [in] id      IE ID
 *  @param [in] p       IE presence
 *  @param [in] c       IE criticality
 *  @param [in] val     IE value
 *
 * Link and allocate all the structures needed to add an IE value to the message. The function to free the value is not linked.
 *
 * */
void s1ap_setValueOnNewIE(S1AP_Message_t *s1msg, ProtocolIE_ID_t id, Presence_e p, Criticality_e c, GenericVal_t *val){
    S1AP_PROTOCOL_IES_t *ie;

    ie=newProtocolIE();
    if(ie == NULL){
        s1ap_msg(ERROR, 0, "S1AP: Coudn't allocate new Protocol IE structure");
    }
    ie->value = val;
    ie->showValue = val->showIE;
    s1msg->pdu->value->addIe(s1msg->pdu->value, ie);
    ie->id = id;
    ie->presence = p;
    ie->criticality = c;
}

/* ********************* s1ap_setValueOnNewIE ******************** */
/** @brief locate the Value struct on a new IE of the message
 *  @param [in] s1msg   s1 message structure pointer
 *  @param [in] id      IE ID
 *  @param [in] p       IE presence
 *  @param [in] c       IE criticality
 *  @param [in] val     IE value
 *
 * Link and allocate all the structures needed to add an IE value to the message.
 * The copy of the value is not implemented
 *
 * */
void s1ap_copyValueOnNewIE(S1AP_Message_t *s1msg, ProtocolIE_ID_t id, Presence_e p, Criticality_e c, GenericVal_t *val){
    S1AP_PROTOCOL_IES_t *ie;
    GenericVal_t *copy = val;

    /*copy = getconstructor[id]();*/

    ie=newProtocolIE();
    if(ie == NULL){
        s1ap_msg(ERROR, 0, "S1AP: Coudn't allocate new Protocol IE structure");
    }
    ie->value = copy;
    ie->showValue = copy->showIE;
    ie->freeValue = copy->freeIE;
    s1msg->pdu->value->addIe(s1msg->pdu->value, ie);
    ie->id = id;
    ie->presence = p;
    ie->criticality = c;
}

/* ********************* Generic Structure ******************** */
/** @brief Constructor of generic IE
 *  @param [in] s1msg   s1 message structure pointer
 *  @param [in] id      IE ID
 *  @param [in] p       IE presence
 *  @param [in] c       IE criticality
 * Link and allocate all the structures needed to add and IE to the message
 *
 * */
void *s1ap_newIE(S1AP_Message_t *s1msg, ProtocolIE_ID_t id, Presence_e p, Criticality_e c){
    GenericVal_t *val;

    val = getconstructor[id]();
    s1ap_copyValueOnNewIE(s1msg, id, p, c, val);

    return (void*)val;
}
