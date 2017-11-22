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
 * @file   S1AP_IEdec.h
 * @Author Vicent Ferrer
 * @date   April, 2013
 * @brief
 *
 */

#ifndef S1AP_IEENC_H_
#define S1AP_IEENC_H_

#include "rt_per_bin.h"
#include "CommonDataTypes.h"
#include "Constants.h"
#include "Containers.h"


/**@brief IE decoder
 * @param [out] bytes Byte stream structure
 * @param [in] ie IE structure to be encoded.
 *
 * This function encodes the Information Element on the bytes structure.
 * The bytes structure should have enough memory to write.
 * */
extern void enc_protocolIEs(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t *ie);


/**@brief Encode IE prototype
 * @param [out] bytes Byte stream structure
 * @param [in] ie IE structure to be encoded.
 *
 * Used to define an array with the encoding functions for each IE available.
 * */
typedef void (*getEncS1AP_IE)( struct BinaryData *bytes, S1AP_PROTOCOL_IES_t *ie);

/**@brief Encoding function Array
 * Function pointers array, use ProtocolIE_ID_t (S1AP_PROTOCOL_IES_t.id) as a vector index
 *
 * getEncS1AP_IE decfunction;
 * decfunction = getenc_S1AP_IE[1];
 * decfunction(bytes, ie);
 * */
extern const getEncS1AP_IE getenc_S1AP_IE[];

/*  IE Decoder Functions*/
/*  These are intended to be private functions, but in the future can be uncommented to become part of the API
extern void dec_Global_ENB_ID(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes);
extern void dec_ENBname(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes);

extern void dec_SupportedTAs(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes);

extern void dec_PagingDRX(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes);
*/

#endif /* S1AP_IEENC_H_ */
