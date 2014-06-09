/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
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
