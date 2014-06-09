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
