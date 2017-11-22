/* AaltoMME - Mobility Management Entity for LTE networks
 * Copyright (C) 2013 Vicent Ferrer Guash & Jesus Llorente Santos
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file   rt_per_bin.c
 * @Author Vicent Ferrer
 * @date   April, 2013
 * @brief
 *
 * This module try to implement the asn1rt_per_bin.erl erlang functions needed
 */
#include "rt_per_bin.h"
#include "S1APlog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>


uint64_t htonll(uint64_t value)
{
    // The answer is 42
    static const int num = 42;

    // Check the endianness
    if (*(const uint8_t*)(&num) == num)
    {
        const uint32_t high_part = htonl((uint32_t)(value >> 32));
        const uint32_t low_part = htonl((uint32_t)(value & 0xFFFFFFFFLL));

        return ( ((uint64_t)low_part) << 32) | high_part;
    }else{
        return value;
    }
}

void printf_buffer(uint8_t* buf, uint32_t blen){
    uint8_t buf2[1000];
    uint32_t pos, n, len;

    len = blen%8==0?blen/8:blen/8+1;

    snprintf((char*)buf2, 1000, "Buffer: len %lu:\n", (size_t)blen);
    pos = strlen((char*)buf2);
    for(n=0; n<len; n++) {
        if ((pos+4)<1000) {
            sprintf((char*)(buf2+pos), " %02hhx", ((uint8_t*)buf)[n]);
            pos += 3;
        }
    }
    buf2[pos] = 0;
    printf("%s\n", buf2);
    printf("End buff \n");
}


void align_dec(struct BinaryData *bytes){
    if(bytes->pos != 0){
        /*s1ap_msg(DEBUG, 0, "**Align**");*/
        bytes->data+=1;
        bytes->pos=0;
        bytes->length-=8;
    }
    /*
    align({0,L}) ->
        {0,L};
    align({_Pos,<<_H,T/binary>>}) ->
        {0,T};
    align(Bytes) ->
        {0,Bytes}.
    */
}

void getbit(struct BinaryData *bytes, uint8_t *bit){
/*
    getbit({7,<<_:7,B:1,Rest/binary>>}) ->        {B,{0,Rest}};
    getbit({0,Buffer = <<B:1,_:7,_/binary>>}) ->        {B,{1,Buffer}};
    getbit({Used,Buffer}) ->
        Unused = (8 - Used) - 1,
        <<_:Used,B:1,_:Unused,_/binary>> = Buffer,
        {B,{Used+1,Buffer}};
    getbit(Buffer) when binary(Buffer) ->        getbit({0,Buffer}).
*/
    if(bytes->pos<0 || bytes->pos>=8){
        bytes->pos=0;
    }
    if(bytes->pos>=0 && bytes->pos<8 && bytes->data!=NULL){
        uint8_t byte = bytes->data[0];
        /* *bit = byte >> (7 - bytes->pos) & 1;*/
        *bit = (byte >> (7 - bytes->pos)) &0x1;
        if(bytes->pos + 1 == 8){
            bytes->data++;
        }
        bytes->pos = (bytes->pos + 1 )%8;
    }
}

void getbits(struct BinaryData *bits, struct BinaryData *bytes, uint8_t num){
    uint32_t newused, numbytes, i, limit;
    uint8_t *tmp;

    bits->pos=0;
    bits->length = num;
    if(num%8==0 && bytes->pos==0){
        memcpy(bits->data, bytes->data, num/8);
        bytes->length -= num;
        bytes->data += num/8;
    }else{
        newused = (num + bytes->pos)%8;
        numbytes = (num + bytes->pos)/8;
        if(newused%8!=0){
            numbytes++;
        }
        /*printf("getbits num=%u, newused=%u, numbytes=%u\n",num, newused, numbytes);
        printf("Bytes to be copied %#x %#x %#x\n", bytes->data[0], bytes->data[1], bytes->data[2]);*/
        tmp = malloc(numbytes);
        memcpy(tmp, bytes->data, numbytes);
        /*printf("Copied bytes [%u] %#x %#x %#x\n", numbytes+1, bits->data[0], bits->data[1], bits->data[2]);*/
        /*printf("Copied bytes [%u] %#x %#x %#x\n", numbytes+1, tmp[0], tmp[1], tmp[2]);*/
        /*if(!bytes->pos + num < 8)printf("getbits bytes %#x %#x %#x, pos% u\n", bytes->data[0], bytes->data[1], bytes->data[2], bytes->pos);*/

        /*Shift bits of the last byte*/
        /*bits->data[numbytes] <<= bytes->pos;*/
        /*bits->pos=bytes->pos;*/
        if(bytes->pos + num <= 8){
            tmp[0] <<= bytes->pos;
            bits->pos=0;
            memcpy(bits->data, tmp, 1);
        }else{
            limit = num/8;
            if(num%8!=0){
                limit++;
            }
            /*printf("getbits limit %u\n", limit);*/
            for(i=0 ; i< limit; i++){
                /*printf("*getbits for [%i] %#x , pos %u\n", i, tmp[i], bytes->pos);*/
                tmp[i] <<= bytes->pos;
                if(((i+1)*8 <= num || num<8)){
                    /*printf("**getbits for [%d] %#x, pos %u , [%d] = %x\n", i, (tmp[i+1]&(0xFF<<(8-bytes->pos)))>>(8-bytes->pos), bytes->pos, i+1, tmp[i+1]);*/
                    tmp[i]|= (tmp[i+1]&(0xFF<<(8-bytes->pos)))>>(8-bytes->pos);
                }else{
                    tmp[i]&= (0xff << (8-newused));
                }
                /*printf("getbits for [%i] %#x \n", i, tmp[i]);*/
            }
            bits->pos=0;
            /*printf("getbits %#x %#x %#x\n", tmp[0], tmp[1], tmp[2]);*/
            memcpy(bits->data, tmp, limit);
        }
        free(tmp);

        bytes->data +=numbytes-1;
        if(newused==0){
            bytes->data++;
        }
        if((8-newused)%8 == 0){
            bytes->pos=0;
        }else{
            bytes->pos=newused;
        }
    }

    /*
    getbits({0,Buffer},Num) when (Num rem 8) == 0 ->
        <<Bits:Num,Rest/binary>> = Buffer,
        {Bits,{0,Rest}};
    getbits({Used,Bin},Num) ->
        NumPlusUsed = Num + Used,
        NewUsed = NumPlusUsed rem 8,
        Unused = (8-NewUsed) rem 8,
        case Unused of
        0 ->
            <<_:Used,Bits:Num,Rest/binary>> = Bin,
            {Bits,{0,Rest}};
        _ ->
            Bytes = NumPlusUsed div 8,
            <<_:Used,Bits:Num,_UBits:Unused,_/binary>> = Bin,
            <<_:Bytes/binary,Rest/binary>> = Bin,
            {Bits,{NewUsed,Rest}}
        end;
    getbits(Bin,Num) when binary(Bin) ->
        getbits({0,Bin},Num).
    */
}

/*num parameter in bytes*/
void getoctets(struct BinaryData *bits, struct BinaryData *bytes, uint32_t num){
    /*s1ap_msg(DEBUG, 0,"Enter getoctets\n");*/

    if(bytes->pos==0){
        if(bytes->length/8 < num){
            s1ap_msg(ERROR, 0,"Trying to extract more bytes (%u bytes)than available (%u bytes).", num, bytes->length/8);
            return;
        }
        bits->pos=0;
        bits->length = num*8;
        if(bits->data == NULL)
            s1ap_msg(ERROR, 0,"bits null");
        if(bytes->data == NULL)
            s1ap_msg(ERROR, 0,"bits null");
        memcpy(bits->data, bytes->data, num);
        bytes->data+=num;
        bytes->length-=num*8;
    }
    else{
        /*Align Buffer*/
        align_dec(bytes);
        getoctets(bits, bytes, num);
    }
    /*
    %% First align buffer, then pick the first Num octets.
    %% Returns octets as an integer with bit significance as in buffer.
    getoctets({0,Buffer},Num) ->
        <<Val:Num/integer-unit:8,RestBin/binary>> = Buffer,
        {Val,{0,RestBin}};
    getoctets({U,<<_Padding,Rest/binary>>},Num) when U /= 0 ->
        getoctets({0,Rest},Num);
    getoctets(Buffer,Num) when binary(Buffer) ->
        getoctets({0,Buffer},Num).
    */
}

/**Not fully implemented, range > 0x10000000000 not accepted yet*/
uint64_t decode_constrained_number(struct BinaryData *bytes, uint32_t Lb, uint64_t Ub){
    struct BinaryData bits;
    uint8_t buffer[MAXDATABYTES], temp;
    uint32_t val=0, len;
    uint64_t range =  ((uint64_t)Ub - Lb + 1);
    bits.data = buffer;
    len=1;
    /*printf("decode_constrained_number range=%llu Ub=%u Lb=%u\n", range, Ub, Lb);*/
    if(range == 2){
        getbits(&bits, bytes, 1);
    }else if(range<=4){
        getbits(&bits, bytes, 2);
    }else if(range<=8){
        getbits(&bits, bytes, 3);
    }else if(range<=16){
        getbits(&bits, bytes, 4);
    }else if(range<=32){
        getbits(&bits, bytes, 5);
    }else if(range<=64){
        getbits(&bits, bytes, 6);
    }else if(range<=128){
        getbits(&bits, bytes, 7);
    }else if(range<=255){
        getbits(&bits, bytes, 8);
    }else if(range<=256){
        getoctets(&bits, bytes, 1);
    }else if(range<=65536){
        getoctets(&bits, bytes, 2);
        len = 2;
    }else if(range<=0x1000000){
        len = decode_constrained_number(bytes, 1,3);
        getoctets(&bits, bytes, len);
    }else if(range<=0x100000000){
        len = decode_constrained_number(bytes, 1,4);
        getoctets(&bits, bytes, len);
    }else if(range<=0x10000000000){
        len = decode_constrained_number(bytes, 1,5);
        getoctets(&bits, bytes, len);
        /* Endianess problem if More than one byte*/
    }else{
        /*Range not supported*/
        s1ap_msg(ERROR, 0,"Range not supported");
    }

    /*Correct endianness*/
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    if(len == 2){
        temp = bits.data[0];
        bits.data[0]=bits.data[1];
        bits.data[1]=temp;
    }else if (len == 3){
        temp = bits.data[0];
        bits.data[0]=bits.data[2];
        bits.data[2]=temp;
    }else if(len ==4){
        temp = bits.data[0];
        bits.data[0]=bits.data[3];
        bits.data[3]=temp;
        temp = bits.data[1];
        bits.data[1]=bits.data[2];
        bits.data[2]=temp;
    }
#endif

    /* Convert Decoded value from bytes to a variable*/
    if(range <=255){    /*Maximum of 8 bits*/
        /*Remove previous bits*/
        bits.data[0]&=(0xFF>>bits.pos);
        val = bits.data[0]<<bits.pos;
        if((bits.pos!=0) && (bits.length>=(8-bits.pos)) ){
            /*When the result is not aligned and is split on 2 different bytes */
            val |= bits.data[1]>>(8-bytes->pos);
        }else{
            /*The result is in a single byte. Move to the right*/
            val>>=(8-bits.length);
        }
    }else{  /*More than 8 bits, In this case the result is aligned*/
        memcpy(&val, bits.data, bits.length/8);
    }

    /* Check Bounds*/
    if((val + Lb > Ub) && (val + Lb <Lb)){
        s1ap_msg(ERROR, 0,"out of bounds");
    }

    return (uint32_t)(val + Lb);

    /*
     decode_constrained_number(Buffer,{Lb,Ub}) ->
        Range = Ub - Lb + 1,
                            %    Val2 = Val - Lb,
        {Val,Remain} =
        if
            Range  == 2 ->
            getbits(Buffer,1);
            Range  =< 4 ->
            getbits(Buffer,2);
            Range  =< 8 ->
            getbits(Buffer,3);
            Range  =< 16 ->
            getbits(Buffer,4);
            Range  =< 32 ->
            getbits(Buffer,5);
            Range  =< 64 ->
            getbits(Buffer,6);
            Range  =< 128 ->
            getbits(Buffer,7);
            Range  =< 255 ->
            getbits(Buffer,8);
            Range  =< 256 ->
            getoctets(Buffer,1);
            Range  =< 65536 ->
            getoctets(Buffer,2);
            Range =< 16#1 00 00 00  ->
            {Len,Bytes2} = decode_length(Buffer,{1,3}),
            {Octs,Bytes3} = getoctets_as_list(Bytes2,Len),
            {dec_pos_integer(Octs),Bytes3};
            Range =< 16#1 00 00 00 00  ->
            {Len,Bytes2} = decode_length(Buffer,{1,4}),
            {Octs,Bytes3} = getoctets_as_list(Bytes2,Len),
            {dec_pos_integer(Octs),Bytes3};
            Range =< 16#10000000000  ->
            {Len,Bytes2} = decode_length(Buffer,{1,5}),
            {Octs,Bytes3} = getoctets_as_list(Bytes2,Len),
            {dec_pos_integer(Octs),Bytes3};
            true  ->
            exit({not_supported,{integer_range,Range}})
        end,
        {Val+Lb,Remain}.
    */
}

uint16_t decode_length_undef(struct BinaryData *bytes){
    uint16_t val;
    /*s1ap_msg(DEBUG, 0,"enter");*/
    align_dec(bytes);
    /*s1ap_msg(DEB, 0,"decode_length_undef() : bytes pos=%u, data %x (%u) ", bytes->pos, bytes->data[0], bytes->data[0]);*/
    if((bytes->data[0]&0x80) == 0x0){
        memcpy(&val, bytes->data,1);
        bytes->data+=1;
        bytes->length-=8;
        val&=0x7F;
        /*s1ap_msg(DEB, 0,"val 0x%x, data 0x%x\n", val, bytes->data[-1]);*/
    }else if((bytes->data[0]&0xC0) == 0x80){
        /*s1ap_msg(DEB, 0,"data %#x %#x", bytes->data[0], bytes->data[1]);*/
        memcpy(&val, bytes->data, 2);
        val = htons(val);
        bytes->data+=2;
        bytes->length-=16;
        val&=0x3FFF;
        /*s1ap_msg(DEB, 0,"val %u = %#x", val, val);*/
    }else if((bytes->data[0]&0xC0) == 0xC0){
        s1ap_msg(ERROR, 0,"Above 16K. Not implemented yet.");
        /*Above 16K*/
    }else{
        s1ap_msg(ERROR, 0,"This message shouldn't appear. data %x %x %x", bytes->data[0], bytes->data[1], bytes->data[2]);
    }
    return val;

    /*
    decode_length(Buffer,undefined)  -> % un-constrained
    {0,Buffer2} = align(Buffer),
    case Buffer2 of
    <<0:1,Oct:7,Rest/binary>> ->
        {Oct,{0,Rest}};
    <<2:2,Val:14,Rest/binary>> ->
        {Val,{0,Rest}};
    <<3:2,_:14,_Rest/binary>> ->
        %% this case should be fixed
        exit({error,{asn1,{decode_length,{nyi,above_16k}}}})
    end;
     */
}


uint32_t decode_semi_constrained_number (struct BinaryData *bytes, uint8_t Lb){
    uint32_t val = 0;
    struct BinaryData bits;
    uint16_t len = decode_length_undef(bytes);
    getoctets(&bits, bytes, len);
    if(bits.length<=32){
        if(bits.length%8){
            memcpy(&val, &bits, bits.length/8);
        }
        else{
            memcpy(&val, &bits, bits.length/8 +1);
        }
    }
    return val + Lb;
    /*
    decode_semi_constrained_number(Bytes,{Lb,_}) ->
        decode_semi_constrained_number(Bytes,Lb);
    decode_semi_constrained_number(Bytes,Lb) ->
    {Len,Bytes2} = decode_length(Bytes,undefined),
    {V,Bytes3} = getoctets(Bytes2,Len),
    {V+Lb,Bytes3}.
     */
}

uint32_t getchoice(struct BinaryData *bytes, uint8_t numChoices, uint8_t ext){
    if(numChoices == 1 && ext == 0){
        return 0;
    }
    else{
        if(ext == 1){
            return decode_small_number(bytes);
        }else{
            return decode_constrained_number(bytes, 0, numChoices-1 );
        }
    }
/*    getchoice(Bytes,1,0) -> % only 1 alternative is not encoded
        {0,Bytes};
    getchoice(Bytes,_,1) ->
        decode_small_number(Bytes);
    getchoice(Bytes,NumChoices,0) ->
        decode_constrained_number(Bytes,{0,NumChoices-1}).
*/
}

uint32_t decode_small_number(struct BinaryData *bytes){
    uint8_t bit;
    uint32_t res, len;
    struct BinaryData bits;
    getbit(bytes, &bit);
    if( bit == 0){
        getbits(&bits, bytes, 6);
        if((bits.length%8 == 0))
            len= (bits.length/8);
        else
            len = (bits.length/8) + 1;
        memcpy(&res, &bits, len);
        if(bits.pos!=0){
            res<<=bits.pos;
        }
    }else{
            res = decode_semi_constrained_number(bytes, 0);
    }
    return res;
    /*
    decode_small_number(Bytes) ->
        {Bit,Bytes2} = getbit(Bytes),
        case Bit of
        0 ->
            getbits(Bytes2,6);
        1 ->
            decode_semi_constrained_number(Bytes2,0)
        end.
    */

}

void decode_integer(){
    /*
    decode_integer(Buffer,Range,NamedNumberList) ->
        {Val,Buffer2} = decode_integer(Buffer,Range),
        case lists:keysearch(Val,2,NamedNumberList) of
        {value,{NewVal,_}} -> {NewVal,Buffer2};
        _ -> {Val,Buffer2}
        end.

    decode_integer(Buffer,[{Rc,_Ec}]) when tuple(Rc) ->
        {Ext,Buffer2} = getext(Buffer),
        case Ext of
        0 -> decode_integer(Buffer2,[Rc]);
        1 -> decode_unconstrained_number(Buffer2)
        end;
    decode_integer(Buffer,undefined) ->
        decode_unconstrained_number(Buffer);
    decode_integer(Buffer,C) ->
        case get_constraint(C,'SingleValue') of
        V when integer(V) ->
            {V,Buffer};
        V when list(V) ->
            {Val,Buffer2} = decode_integer1(Buffer,C),
            case lists:member(Val,V) of
            true ->
                {Val,Buffer2};
            _ ->
                exit({error,{asn1,{illegal_value,Val}}})
            end;
        _ ->
            decode_integer1(Buffer,C)
        end.
    */
}

uint32_t decode_enumerated(struct BinaryData *bytes, uint32_t Lb, uint32_t Ub){
    return decode_constrained_number(bytes, Lb, Ub);
    /*
    decode_enumerated(Buffer,C,NamedNumberTup) when tuple(NamedNumberTup) ->
        {Val,Buffer2} = decode_integer(Buffer,C),
        case catch (element(Val+1,NamedNumberTup)) of
        NewVal when atom(NewVal) -> {NewVal,Buffer2};
        _Error -> exit({error,{asn1,{decode_enumerated,{Val,NamedNumberTup}}}})
        end.
    */
}

/*  Not Implemented*/
/*void decode_object_identifier(struct BinaryData *bytes){
    uint16_t len;
    len = decode_length_undef(bytes);
*/
/*
decode_object_identifier(Bytes) ->
    {Len,Bytes2} = decode_length(Bytes,undefined),
    {Octs,Bytes3} = getoctets_as_list(Bytes2,Len),
    [First|Rest] = dec_subidentifiers(Octs,0,[]),
    Idlist = if
         First < 40 ->
             [0,First|Rest];
         First < 80 ->
             [1,First - 40|Rest];
         true ->
             [2,First - 80|Rest]
         end,
    {list_to_tuple(Idlist),Bytes3}
*/
/*}*/

void decode_open_type(struct BinaryData *octets, struct BinaryData *bytes){

    octets->length = decode_length_undef(bytes)*8;
    octets->pos=0;
    /*s1ap_msg(DEB, 0,"decode_open_type: len = %u bits = %u bytes", octets->length, octets->length/8);*/
    /*s1ap_msg(DEB, 0,"octets.length = %u, bytes.length = %u\n", octets->length, bytes->length);*/
    getoctets(octets, bytes, octets->length/8);
    /*s1ap_msg(DEB, 0,"octets.length = %u, bytes.length = %u\n", octets->length, bytes->length);*/

    /*s1ap_msg(DEB, 0,"End decode_open_type");*/
/*
decode_open_type(Bytes, _C) ->
    {Len,Bytes2} = decode_length(Bytes,undefined),
    getoctets_as_bin(Bytes2,Len).
*/
}

void decode_octet_string(uint8_t *str, struct BinaryData *bytes, uint32_t size){
    struct BinaryData res;
    res.length=0;
    res.pos=0;
    res.data = malloc(size);
    memset(res.data, 0, size);
    /*printf("**decode_octet_string() size = %u, bytes = 0x%x 0x%x, pos %u\n", size, bytes->data[0], bytes->data[1], bytes->pos);*/
    //printf("decode_octet_string() bytes->data %#x %#x %#x, pos %u, size %u\n", bytes->data[0], bytes->data[1], bytes->data[2], bytes->pos, size);

    if (size == 0){
        return;
    }else if (size == 1){
        getbits(&res, bytes, 8);
        if(res.pos != 0){
            res.data[0]&=(0xFF>>res.pos);
            res.data[0] = res.data[0]>>(8-res.length);
            if((res.pos!=0) && (res.length>=(8-res.pos)) ){
                /*When the result is not aligned and is split on 2 diferent bytes */
                res.data[0] <<= res.pos;
                res.data[0] |= res.data[1]>>(8-bytes->pos);
            }
        }
    }else if (size == 2){
        /*printf("dos : bytes->por %u, data %x %x\n",bytes->pos, bytes->data[0], bytes->data[1]);*/
        getbits(&res, bytes, 16);
        /*Remove result offset*/
        if(res.pos!=0){
            res.data[0] <<= res.pos;
            res.data[0] |= (res.data[1]&(0xFF<<(8-res.pos)))>>(8-res.pos);
            res.data[1]<<=res.pos;
            res.data[1] |= (res.data[2]&(0xFF<<(8-res.pos)))>>(8-res.pos);
        }

    }else if (size<=65535){
        getoctets(&res, bytes, size);
    }else{
        s1ap_msg(ERROR, 0,"Fragmentation Not implemented");
        /*fragmented*/
    }
    memcpy(str, res.data, size);
    //printf("str %#x %#x, res.data %#x %#x, size %u\n", str[0], str[1], res.data[0], res.data[1], size);
    free(res.data);
}

/*
%% This very inefficient and should be moved to compiletime
charbits(NumOfChars,aligned) ->
    case charbits(NumOfChars) of
    1 -> 1;
    2 -> 2;
    B when B =< 4 -> 4;
    B when B =< 8 -> 8;
    B when B =< 16 -> 16;
    B when B =< 32 -> 32
    end.

charbits(NumOfChars) when NumOfChars =< 2 -> 1;
charbits(NumOfChars) when NumOfChars =< 4 -> 2;
charbits(NumOfChars) when NumOfChars =< 8 -> 3;
charbits(NumOfChars) when NumOfChars =< 16 -> 4;
charbits(NumOfChars) when NumOfChars =< 32 -> 5;
charbits(NumOfChars) when NumOfChars =< 64 -> 6;
charbits(NumOfChars) when NumOfChars =< 128 -> 7;
charbits(NumOfChars) when NumOfChars =< 256 -> 8;
charbits(NumOfChars) when NumOfChars =< 512 -> 9;
charbits(NumOfChars) when NumOfChars =< 1024 -> 10;
charbits(NumOfChars) when NumOfChars =< 2048 -> 11;
charbits(NumOfChars) when NumOfChars =< 4096 -> 12;
charbits(NumOfChars) when NumOfChars =< 8192 -> 13;
charbits(NumOfChars) when NumOfChars =< 16384 -> 14;
charbits(NumOfChars) when NumOfChars =< 32768 -> 15;
charbits(NumOfChars) when NumOfChars =< 65536 -> 16;
charbits(NumOfChars) when integer(NumOfChars) ->
    16 + charbits1(NumOfChars bsr 16).

charbits1(0) ->
    0;
charbits1(NumOfChars) ->
    1 + charbits1(NumOfChars bsr 1)
*/
/*
get_constraint([{Key,V}],Key) ->
    V;
get_constraint([],_Key) ->
    no;
get_constraint(C,Key) ->
    case lists:keysearch(Key,1,C) of
    false ->
        no;
    {value,{_,V}} ->
        V
    end.
*/
/*
get_NumBits(C,StringType) ->
    case get_constraint(C,'PermittedAlphabet') of
    {'SingleValue',Sv} ->
        charbits(length(Sv),aligned);
    no ->
        case StringType of
        'IA5String' ->
            charbits(128,aligned); % 16#00..16#7F
        'VisibleString' ->
            charbits(95,aligned); % 16#20..16#7E
        'PrintableString' ->
            charbits(74,aligned); % [$\s,$',$(,$),$+,$,,$-,$.,$/,"0123456789",$:,$=,$?,$A..$Z,$a..$z
        'NumericString' ->
            charbits(11,aligned); % $ ,"0123456789"
        'UniversalString' ->
            32;
        'BMPString' ->
            16
        end
    end
*/

/*
chars_decode(Bytes,NumBits,StringType,C,Len) ->
    CharInTab = get_CharInTab(C,StringType),
    chars_decode2(Bytes,CharInTab,NumBits,Len).
*/

void decode_known_multiplier_string_PrintableString_withExt(uint8_t *str, struct BinaryData *bytes, uint32_t Lb, uint32_t Ub){
    struct BinaryData c;
    uint8_t buffer[Ub];
    uint32_t numBits = 8, len, i;
    uint8_t bit;

    c.data=buffer;

    /*printf("bytes->data %#x %x %x pos %u\n", bytes->data[0], bytes->data[1], bytes->data[2], bytes->pos);*/
    getbit(bytes, &bit);
    /*printf("bytes->data %#x %x %x pos %u\n", bytes->data[0], bytes->data[1], bytes->data[2], bytes->pos);*/

    if(bit==0){
        len = decode_constrained_number(bytes, Lb, Ub);
        /*printf("len = %u\n", len);
        printf("bytes->data %x %x %x pos %u\n", bytes->data[0], bytes->data[1], bytes->data[2], bytes->pos);*/

        align_dec(bytes);
        /*printf("bytes->data %x %x %x pos %u\n", bytes->data[0], bytes->data[1], bytes->data[2], bytes->pos);*/

    }else{
        printf("decode_known_multiplier_string_PrintableString_ext(): Error");
        return;
        /*ERROR */
    }
    for(i=0;i<len;i++){
        getbits(&c, bytes, numBits);
        str[i]=c.data[0];
        /*printf("len = %u, i = %u, c.data[0] %c\n", len, i, c.data[0]);*/
    }
    str[len]='\0';
    /*printf("PrintableString %s\n", str);*/

/*
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
    end
 */
}

void getextension(struct BinaryData *extensions, struct BinaryData *bytes, uint8_t ext){
    if(ext==0){
        /*No extensions*/
        extensions->data = NULL;
    }
/*
getextension(0, Bytes) ->
    {{},Bytes};
getextension(1, Bytes) ->
    {Len,Bytes2} = decode_small_length(Bytes),
    {Blist, Bytes3} = getbits_as_list(Len,Bytes2),
    {list_to_tuple(Blist),Bytes3}.
*/
}

void skipextensions(struct BinaryData *bytes, uint8_t num, struct BinaryData *extensions){
    if(extensions->data==NULL){
        return;
    }
    else{
        printf("skipextensions() Not implemented yet\n");
    }

/*
skipextensions(Bytes,Nr,ExtensionBitPattern) ->
    case (catch element(Nr,ExtensionBitPattern)) of
    1 ->
        {_,Bytes2} = decode_open_type(Bytes,[]),
        skipextensions(Bytes2, Nr+1, ExtensionBitPattern);
    0 ->
        skipextensions(Bytes, Nr+1, ExtensionBitPattern);
    {'EXIT',_} -> % badarg, no more extensions
        Bytes
    end.
*/
}

uint32_t decode_bit_string(struct BinaryData *bytes, uint32_t num){
    struct BinaryData bitstr;
    uint8_t buffer[MAXDATABYTES];
    uint32_t tmp, numbytes;

    memset(buffer, 0, MAXDATABYTES);
    bitstr.data=buffer;
    if(num>=16){
        align_dec(bytes);
    }
    getbits(&bitstr, bytes, num);

    numbytes = num/8;
    if(num%8!=0){
        numbytes++;
    }

    memcpy(&tmp, bitstr.data, numbytes);
    return ntohl(tmp)>>(32-num);
/*
decode_bit_string(Buffer, C, NamedNumberList) ->
    case get_constraint(C,'SizeConstraint') of
    {Lb,Ub} when integer(Lb),integer(Ub) ->
        {Len,Bytes2} = decode_length(Buffer,{Lb,Ub}),
        Bytes3 = align(Bytes2),
        bit_list_or_named(Bytes3,Len,NamedNumberList);
    no ->
        {Len,Bytes2} = decode_length(Buffer,undefined),
        Bytes3 = align(Bytes2),
        bit_list_or_named(Bytes3,Len,NamedNumberList);
    0 -> % fixed length
        {[],Buffer}; % nothing to encode
    V when integer(V),V=<16 -> % fixed length 16 bits or less
        bit_list_or_named(Buffer,V,NamedNumberList);
    V when integer(V),V=<65536 ->
        Bytes2 = align(Buffer),
        bit_list_or_named(Bytes2,V,NamedNumberList);
    V when integer(V) ->
        Bytes2 = align(Buffer),
        {BinBits,_} = decode_fragmented_bits(Bytes2,V),
        bit_list_or_named(BinBits,V,NamedNumberList);
    Sc -> % extension marker
        {Len,Bytes2} = decode_length(Buffer,Sc),
        Bytes3 = align(Bytes2),
        bit_list_or_named(Bytes3,Len,NamedNumberList)
    end.
 */
}

/* ******************** Decode Functions ******************* */

void align_enc(struct BinaryData *bytes){
    if(bytes->pos!=0){
        bytes->length+=8-bytes->pos;
        bytes->pos=0;
        bytes->offset++;
    }
}

void setbits(struct BinaryData *bytes, uint32_t numbits, uint32_t val){

    uint32_t numbytes, i;
    uint8_t byte;

    /*Number of bytes*/
    numbytes=numbits/8;
    if (numbits%8!=0){
        numbytes++;
    }

    if ( (numbits+bytes->pos)<=8 ){
        /*Set bits only on the current octet*/
        byte = val & (0xFF >> (8-numbits)%8);
        bytes->offset[0] |= (byte<<((8-numbits-bytes->pos)%8));
    }else if (bytes->pos==0){
        /*Aligned bits*/
        val = htonl(val<<(32-numbits));
        memcpy(bytes->offset, &val, numbytes);
    }else{
        /*printf("setbits val %u %#x\n", val, val);*/
        val<<=(32-numbits); /*Align to the left to simplify the patterns*/
        /*printf("setbits val %#x, bytes->pos %u\n", val, bytes->pos);*/
        /*unaligned bits on more than one byte*/
        for(i=0; i<numbytes ; i++){
            byte = (val & (0xFF<<(32-8*(i+1))))>>(32-8*(i+1));
            bytes->offset[i] |= byte>>bytes->pos;
            /*printf("setbits first byte part  %#x\n",  byte>>bytes->pos);*/
            bytes->offset[i+1] |= (byte&(0xFF>>(8-bytes->pos)))<<(8-bytes->pos);
            /*printf("setbits val second byte part %#x\n", byte);*/
            /*printf("setbits val second byte part %#x\n", (byte&(0xFF>>(8-bytes->pos)))<<(8-bytes->pos));*/

        }
    }

    /*Refresh Lenght & Position variables*/
    bytes->pos+=numbits;
    if (bytes->pos>=8){
        bytes->offset += numbytes;
        bytes->pos = bytes->pos%8;
    }
    bytes->length += numbits;
}

void setoctets(struct BinaryData *bytes, uint32_t numbytes, uint8_t *val){
    uint32_t i;

    align_enc(bytes);
    /*printf("setoctets (): size %u\n", numbytes);*/
    /*printf("offset %p\n", bytes->offset);*/
    for (i=0 ; i<numbytes ; i++){
        /*printf("setoctets (): val[i] %#x\n", val[i]);*/
        setbits(bytes, 8, val[i]);
    }
}
void set_choice_ext(struct BinaryData *bytes, uint32_t choice, uint32_t maxChoices, uint8_t ext){

    setbits(bytes, 1, ext);

    if(ext == 1){
        /*encode_small_number(choice);*/
    }
    else if(maxChoices>1){
        encode_constrained_number(bytes, choice, 0, maxChoices-1);
    }else{
        /*no encoding if only 0 or 1 alternative*/
    }

/*
set_choice(Alt,{L1,L2},{Len1,_Len2}) ->
    case set_choice_tag(Alt,L1) of
    N when integer(N), Len1 > 1 ->
        [{bits,1,0}, % the value is in the root set
         encode_integer([{'ValueRange',{0,Len1-1}}],N)];
    N when integer(N) ->
        [{bits,1,0}]; % no encoding if only 0 or 1 alternative
    false ->
        [{bits,1,1}, % extension value
         case set_choice_tag(Alt,L2) of
         N2 when integer(N2) ->
             encode_small_number(N2);
         false ->
             unknown_choice_alt
         end]
    end;
set_choice(Alt,L,Len) ->
    case set_choice_tag(Alt,L) of
    N when integer(N), Len > 1 ->
        encode_integer([{'ValueRange',{0,Len-1}}],N);
    N when integer(N) ->
        []; % no encoding if only 0 or 1 alternative
    false ->
        [unknown_choice_alt]
    end
*/

}

void eint(uint8_t *bytes, uint64_t Val2, uint32_t *len){

    uint64_t tmp=Val2;
    uint8_t i, rev;
    *len = 0;
    while(tmp!=0){
        /*printf("*eint tmp&0xFF = %x\n", (uint8_t)tmp&0xFF);*/
        bytes[*len]=tmp&0xFF;
        (*len)++;
         tmp>>=8;
    }

    if(*len == 0){ /*To encode 0*/
        *len=1;
        bytes[0]=0;
    }

    /* Reverse vector*/
    for(i=0;i<*len/2;i++){
        rev = bytes[i];
        bytes[i] = bytes[*len-i-1];
        bytes[*len-i-1] = rev;
    }

}

void encode_constrained_number(struct BinaryData *bytes, uint64_t Val, uint32_t Lb, uint64_t Ub){
    uint64_t range;
    uint32_t Val2, len;
    uint8_t byte[10];
    range = Ub - Lb + 1ULL;
    Val2 = Val - Lb;
    /*printf("encode_constrained_number Val2=%#x, Val=%#x, range = %#u%llu\n", Val2, Val, range);
    printf("encode_constrained_number bytes->data %x, bytes->pos %u, bytes->length %u\n", bytes->data, bytes->pos, bytes->length);*/

    if(range == 2){
        setbits(bytes, 1, Val2);
    }else if(range<=4){
        setbits(bytes, 2, Val2);
    }else if(range<=8){
        setbits(bytes, 3, Val2);
    }else if(range<=16){
        setbits(bytes, 4, Val2);
    }else if(range<=32){
        setbits(bytes, 5, Val2);
    }else if(range<=64){
        setbits(bytes, 6, Val2);
    }else if(range<=128){
        setbits(bytes, 7, Val2);
    }else if(range<=255){
        setbits(bytes, 8, Val2);
    }else if(range<=256){
        byte[0]=Val2;
        setoctets(bytes, 1, byte);
    }else if(range<=65536){
        byte[0]=(Val2&0xFF00)>>8;
        byte[1] = Val2&0xFF;
        setoctets(bytes, 2, byte);
    }else if(range<=0x1000000){
        eint(byte, Val, &len);
        encode_constrained_number(bytes, len, 1,3);
        setoctets(bytes, len, byte);
    }else if(range<=0x100000000){
        eint(byte, Val, &len);
        encode_constrained_number(bytes, len, 1,4);
        setoctets(bytes, len, byte);
    }else if(range<=0x10000000000){
        eint(byte, Val, &len);
        encode_constrained_number(bytes, len, 1,5);
        setoctets(bytes, len, byte);

    }else{
        /*Range not supported*/
    }
    /*
encode_constrained_number({Lb,Ub},Val) when Val >= Lb, Ub >= Val ->
    Range = Ub - Lb + 1,
    Val2 = Val - Lb,
    if
    Range  == 2 ->
        {bits,1,Val2};
    Range  =< 4 ->
        {bits,2,Val2};
    Range  =< 8 ->
        {bits,3,Val2};
    Range  =< 16 ->
        {bits,4,Val2};
    Range  =< 32 ->
        {bits,5,Val2};
    Range  =< 64 ->
        {bits,6,Val2};
    Range  =< 128 ->
        {bits,7,Val2};
    Range  =< 255 ->
        {bits,8,Val2};
    Range  =< 256 ->
        {octets,[Val2]};
    Range  =< 65536 ->
        {octets,<<Val2:16>>};
    Range =< 16#1000000  ->
        Octs = eint_positive(Val2),
        [{bits,2,length(Octs)-1},{octets,Octs}];
    Range =< 16#100000000  ->
        Octs = eint_positive(Val2),
        [{bits,2,length(Octs)-1},{octets,Octs}];
    Range =< 16#10000000000  ->
        Octs = eint_positive(Val2),
        [{bits,3,length(Octs)-1},{octets,Octs}];
    true  ->
        exit({not_supported,{integer_range,Range}})
    end;
 */
}

void encode_unconstrained_number(struct BinaryData *bytes, uint32_t val){
    uint8_t byte[10];
    if(val<128){            /* 0... ....*/
        byte[0]=(val&0x7F);
        setoctets(bytes, 1, byte);
    }else if(val < 16384){  /* 10.. .... .... ....*/
        byte[0]=(val&0x3F00)>>8;
        byte[0]|=0x80;
        byte[1]=val&0xFF;
        setoctets(bytes, 2, byte);
    }else{
        s1ap_msg(ERROR, 0, "Encoding of unconstrained value greater than 16384 not implemented yet.");
    }

/*
encode_length(undefined,Len) -> % un-constrained
    if
    Len < 128 ->
        {octets,[Len]};
    Len < 16384 ->
        {octets,<<2:2,Len:14>>};
    true  -> % should be able to endode length >= 16384
        exit({error,{asn1,{encode_length,{nyi,above_16k}}}})
    end;
 */
}

void encode_open_type(struct BinaryData *bytes, struct BinaryData *opentypedata){
    uint32_t numBytes;
    numBytes = opentypedata->length/8;
    if(opentypedata->length%8!=0){
        numBytes++;
    }

    /*Encode Length*/
    encode_unconstrained_number(bytes, numBytes);
    /*printf_buffer(bytes->data, bytes->lenght);*/

    /*Encode value*/
    setoctets(bytes, numBytes, opentypedata->data);

}

void encode_octet_string(struct BinaryData *bytes, uint8_t *str, uint32_t size){
    /*printf("**decode_octet_string() size = %u, bytes = 0x%x 0x%x, pos %u\n", size, bytes->data[0], bytes->data[1], bytes->pos);*/

    if (size == 0){
        return;
    }else if (size == 1){
        setbits(bytes, 8, str[0]);
    }else if (size == 2){
        /*printf("encode_octet_string() %#x\n", ((str[0]<<8)|str[1]));*/
        setbits(bytes, 16, (str[0]<<8 | str[1]));
    }else if (size<=65535){
        setoctets(bytes, size, str);
    }else{
        s1ap_msg(ERROR, 0,"Fragmentation Not implemented");
        /*fragmented*/
    }
/*
encode_octet_string(C,Val) ->
    encode_octet_string(C,false,Val).

encode_octet_string(C,Bool,{_Name,Val}) ->
    encode_octet_string(C,Bool,Val);
encode_octet_string(_,true,_) ->
    exit({error,{asn1,{'not_supported',extensionmarker}}});
encode_octet_string(C,false,Val) ->
    case get_constraint(C,'SizeConstraint') of
    0 ->
        [];
    1 ->
        [V] = Val,
        {bits,8,V};
    2 ->
        [V1,V2] = Val,
        [{bits,8,V1},{bits,8,V2}];
    Sv when Sv =<65535, Sv == length(Val) -> % fixed length
        {octets,Val};
    {Lb,Ub}  ->
        [encode_length({Lb,Ub},length(Val)),{octets,Val}];
    Sv when list(Sv) ->
        [encode_length({hd(Sv),lists:max(Sv)},length(Val)),{octets,Val}];
    no  ->
        [encode_length(undefined,length(Val)),{octets,Val}]
    end.
*/
}

void encode_known_multiplier_string_PrintableString_withExt(struct BinaryData *bytes, uint32_t Lb, uint32_t Ub, uint8_t *str, uint32_t slen, uint8_t ext){
    uint32_t i;

    /*Encode extension*/
    setbits(bytes, 1, ext);

    if(ext!=0){
        printf("encode_known_multiplier_string_PrintableString_withExt(): Error, extension encoding not implemented\n");
        return;
    }

    /*Encode string length*/
    encode_constrained_number(bytes, slen, Lb, Ub);

    align_enc(bytes);

    /*Encode string. This may not be aligned*/
    for(i=0 ; i<slen ; i++){
        setbits(bytes, 8, str[i]);
    }
    if(str[i+1]!='\0'){
        printf("encode_known_multiplier_string_PrintableString_withExt(): Warning: The encoded string was not a null terminated string\n");
    }
}

void encode_semi_constrained_number(struct BinaryData *bytes, uint32_t Lb, uint32_t val){
    uint32_t val2 = val-Lb;
    printf("WARNING: Not Implemented yet, val2 = %u\n", val2);
/*
Val2 = Val - Lb,
Oct = eint_positive(Val2),
Len = length(Oct),
if
Len < 128 ->
    {octets,[Len|Oct]}; % equiv with encode_length(undefined,Len) but faster
true ->
    [encode_length(undefined,Len),{octets,Oct}]
end.
 */
}

void encode_small_number(struct BinaryData *bytes, uint32_t val){
    if(val<=63){
        setbits(bytes,1,0);
        setbits(bytes,6,val);
    }else{
        setbits(bytes,1,1);
        encode_semi_constrained_number(bytes, 0,val);
    }
/*
encode_small_number({Name,Val}) when atom(Name) ->
    encode_small_number(Val);
encode_small_number(Val) when Val =< 63 ->
%    [{bits,1,0},{bits,6,Val}];
    [{bits,7,Val}]; % same as above but more efficient
encode_small_number(Val) ->
    [{bits,1,1},encode_semi_constrained_number(0,Val)].*/
}
