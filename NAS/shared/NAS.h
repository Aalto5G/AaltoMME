/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   NAS.h
 * @Author Vicent Ferrer
 * @date   May, 2013
 * @brief  NAS main header
 */

#ifndef _NAS_H
#define _NAS_H

#include "NASMessages.h"
#include "NASConstants.h"

typedef void* NAS;

typedef enum{
	NAS_EIA0, /**< EPS integrity algorithm EIA0 (null integrity protection algorithm)*/
	NAS_EIA1, /**< EPS integrity algorithm 128-EIA1 */
	NAS_EIA2, /**< EPS integrity algorithm 128-EIA2 */
	NAS_EIA3, /**< EPS integrity algorithm 128-EIA3 */
	NAS_EIA4, /**< EPS integrity algorithm EIA4 */
	NAS_EIA5, /**< EPS integrity algorithm EIA5 */
	NAS_EIA6, /**< EPS integrity algorithm EIA6 */
	NAS_EIA7, /**< EPS integrity algorithm EIA7 */
}NAS_EIA;

typedef enum{
	NAS_EEA0, /**< EPS encription algorithm EEA0 (null ciphering algorithm)*/
	NAS_EEA1, /**< EPS encription algorithm 128-EEA1*/
	NAS_EEA2, /**< EPS encription algorithm 128-EEA2*/
	NAS_EEA3, /**< EPS encription algorithm 128-EEA3*/
	NAS_EEA4, /**< EPS encription algorithm EEA4*/
	NAS_EEA5, /**< EPS encription algorithm EEA5*/
	NAS_EEA6, /**< EPS encription algorithm EEA6*/
	NAS_EEA7, /**< EPS encription algorithm EEA7*/
}NAS_EEA;

NAS nas_newHandler();

void nas_freeHandler(NAS h);

void nas_setSecurity(NAS h, NAS_EIA i, NAS_EEA e, uint8_t *key);

SecurityHeaderType_t dec_secNAS(const NAS h,
                                GenericNASMsg_t *msg,
                                const uint8_t *buf,
                                const uint32_t size);


void dec_NAS(GenericNASMsg_t *msg,
             uint8_t *buf,
             uint32_t size);

void dec_ESM(ESM_Message_t *msg, uint8_t *buf, uint32_t size);

void newNASMsg_EMM(uint8_t **curpos,
                   ProtocolDiscriminator_t protocolDiscriminator,
                   SecurityHeaderType_t securityHeaderType);

void newNASMsg_ESM(uint8_t **curpos,
                   ProtocolDiscriminator_t protocolDiscriminator,
                   uint8_t ePSBearerId);

void encaps_ESM(uint8_t **curpos,
                ProcedureTransactionId_t procedureTransactionIdentity,
                NASMessageType_t messageType);

void encaps_EMM(uint8_t **curpos,
                NASMessageType_t messageType);

/* Tool Functions*/
uint32_t encapPLMN(uint16_t mcc, uint16_t mnc);

#endif  /* !_NAS_H */
