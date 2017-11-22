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

#ifndef S1AP_IEDEC_H_
#define S1AP_IEDEC_H_

#include "rt_per_bin.h"
#include "CommonDataTypes.h"
#include "Constants.h"
#include "Containers.h"


/**@brief IE encoder
 * @param [in] bytes Byte stream structure input to be decode.
 * @return ie resulting IE structure.
 *
 * This function decodes the Information Element provided.
 * The returned structure should be deallocated once used with its function pointer field S1AP_PROTOCOL_IES_t.freeIE()
 * */
extern S1AP_PROTOCOL_IES_t *dec_protocolIEs(struct BinaryData *bytes);


/**@brief Decode IE prototype
 * @param [out] ie decoded IE structure.
 * @param [in] bytes Byte stream structure to be decoded
 *
 * Used to define an array with the decoding functions for each IE available.
 * The destructor of the allocated memory is stored on the freeValue function pointer field.
 * */
typedef void (*getDecS1AP_IE)(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes);

/**@brief Decoding function Array
 * Function pointers array, use ProtocolIE_ID_t (S1AP_PROTOCOL_IES_t.id) as a vector index
 *
 * getDecS1AP_IE encfunction;
 * encfunction = getDecS1AP_IE[1];
 * encfunction(ie, bytes);
 * */
/*  Vector with the function pointers, use ProtocolIE_ID_t (S1AP_PROTOCOL_IES_t.id) as a vector index*/
extern const getDecS1AP_IE getdec_S1AP_IE[];

/*  IE Decoder Functions*/
/*  These are intended to be private functions, but in the future can be uncommented to become part of the API
extern void dec_Global_ENB_ID(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes);
extern void dec_ENBname(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes);

extern void dec_SupportedTAs(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes);

extern void dec_PagingDRX(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes);
*/



#endif /* S1AP_IEDEC_H_ */
