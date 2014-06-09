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

void dec_NAS(GenericNASMsg_t *msg, uint8_t *buf, uint32_t size);

void newNASMsg_EMM(uint8_t **curpos, ProtocolDiscriminator_t protocolDiscriminator, SecurityHeaderType_t securityHeaderType);
void newNASMsg_ESM(uint8_t **curpos, ProtocolDiscriminator_t protocolDiscriminator, uint8_t ePSBearerId);
void encaps_ESM(uint8_t **curpos, ProcedureTransactionId_t procedureTransactionIdentity, NASMessageType_t messageType);
void encaps_EMM(uint8_t **curpos, NASMessageType_t messageType);

/* Tool Functions*/
uint32_t encapPLMN(uint16_t mcc, uint16_t mnc);

#endif  /* !_NAS_H */
