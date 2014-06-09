/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   rt_per_bin.h
 * @Author Vicent Ferrer
 * @date   April, 2013
 * @brief
 *
 * This module try to implement the asn1rt_per_bin.erl erlang functions needed
 */

#ifndef _RT_PER_BIN_H
#define _RT_PER_BIN_H

#include <stdint.h>

/* 4096 is the maximum reading buffer length*/
#define MAXDATABYTES 4096

struct BinaryData{
    uint8_t     *data;
    uint8_t     *offset;
    uint8_t     pos;
    uint32_t    length;
};


extern void printf_buffer(uint8_t* buf, uint32_t blen);

extern void align_dec(struct BinaryData *bytes);

/**@brief get bit by pos
 * @param [in]  bitpos position of the bit to be extracted
 * @param [in]  datain data buffer
 * @param [out] bit bit extracted
 */
extern void getbit(struct BinaryData *bytes, uint8_t *bit);
extern void getbits(struct BinaryData *bits, struct BinaryData *bytes, uint8_t num);
extern void getoctets(struct BinaryData *bits, struct BinaryData *bytes, uint32_t num);
extern uint32_t getchoice(struct BinaryData *bytes, uint8_t numChoices, uint8_t ext);
extern uint32_t decode_small_number(struct BinaryData *bytes);
extern uint32_t decode_enumerated(struct BinaryData *bytes, uint32_t Lb, uint32_t Ub);

extern uint64_t decode_constrained_number(struct BinaryData *bytes, uint32_t Lb, uint64_t Ub);

extern uint32_t decode_semi_constrained_number (struct BinaryData *bytes, uint8_t Lb);

extern uint16_t decode_length_undef(struct BinaryData *bytes);

extern void decode_open_type(struct BinaryData *octets, struct BinaryData *bytes);

extern void decode_octet_string(uint8_t *str, struct BinaryData *bytes, uint32_t size);

extern void decode_known_multiplier_string_PrintableString_withExt(uint8_t *str, struct BinaryData *bytes, uint32_t Lb, uint32_t Ub);

extern uint32_t decode_bit_string(struct BinaryData *bytes, uint32_t size);

extern void getextension(struct BinaryData *extensions, struct BinaryData *bytes, uint8_t ext);
extern void skipextensions(struct BinaryData *bytes, uint8_t num, struct BinaryData *extensions);

/*Encoding Functions*/
extern void align_enc(struct BinaryData *bytes);

extern void setbits(struct BinaryData *bytes, uint32_t numbits, uint32_t val);

extern void setoctets(struct BinaryData *bytes, uint32_t numbytes, uint8_t *val);

extern void set_choice_ext(struct BinaryData *bytes, uint32_t choice, uint32_t maxChoices, uint8_t ext);

extern void encode_constrained_number(struct BinaryData *bytes, uint64_t Val, uint32_t Lb, uint64_t Ub);

extern void encode_unconstrained_number(struct BinaryData *bytes, uint32_t val);

extern void encode_open_type(struct BinaryData *bytes, struct BinaryData *data);

extern void encode_octet_string(struct BinaryData *bytes, uint8_t *str, uint32_t size);

extern void encode_known_multiplier_string_PrintableString_withExt(struct BinaryData *bytes, uint32_t Lb, uint32_t Ub, uint8_t *str, uint32_t len, uint8_t ext);

extern void encode_semi_constrained_number(struct BinaryData *bytes, uint32_t Lb, uint32_t val);

extern void encode_small_number(struct BinaryData *bytes, uint32_t val);

#endif  /* !_RT_PER_BIN_H */
