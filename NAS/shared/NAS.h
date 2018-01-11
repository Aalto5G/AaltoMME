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
 * @file   NAS.h
 * @Author Vicent Ferrer
 * @date   May, 2013
 * @brief  NAS main header
 */

#ifndef _NAS_H
#define _NAS_H

#include <stddef.h>
#include <stdint.h>
#include "NASMessages.h"
#include "NASConstants.h"

#define NAS_COUNT_THRESHOLD (10)

typedef void* NAS;

typedef enum{
    NAS_EIA0, /**< EPS integrity algorithm EIA0
                   (null integrity protection algorithm)*/
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

typedef enum{
    NAS_UpLink,   /**< */
    NAS_DownLink, /**< */
}NAS_Direction;


/**
 * @brief  Create new NAS parser structure
 * @return NAS handler for encoding and decoding
 *
 * The returned structure has to be freed using nas_freeHandler
 */
NAS nas_newHandler();


/**
 * @brief Deallocate the NAS handler
 * @param [in] h NAS handler
 *
 * The NAS handler structure is freed.
 */
void nas_freeHandler(NAS h);


/**
 * @brief Reset NAS handler information
 * @param [inout] h           NAS handler
 * @param [in]    i           Integrity algorithm
 * @param [in]    e           Encryption algorithm
 * @param [in]    kasme       K ASME
 *
 * This function allows NULL parameters to maintain them unchanged, thus
 * allowing the to change some of them, except the integrity and encryption
 * algorithms that are always required.
 */
void nas_setSecurity(NAS h, const NAS_EIA i, const NAS_EEA e,
                     const uint8_t *kasme);


/**
 * @brief Get NAS message header
 * @param [in]  buf         Buffer with the NAS message to be parsed
 * @param [in]  size        Size of the NAS message buffer
 * @param [out] s           Security Header type
 * @param [out] p           Protocol Discriminator
 * @return 1 on success, 0 if any error
 *
 * It parses the first byte of the NAS message, the header. Use this function
 * to decide if the packet should be validated or deciphered
 */
int nas_getHeader(const uint8_t *buf, const uint32_t size,
                  SecurityHeaderType_t *s, ProtocolDiscriminator_t *p);

/**
 * @brief Get NAS Count
 * @param [in] h           NAS handler
 * @param [in] direction   0 for uplink, 1 for downlink
 * @return full NAS Count (24 bits)
 *
 * This function allows to access the NAS COUNT values to derive other keys
 */
const uint32_t nas_getLastCount(const NAS h, const NAS_Direction direction);


/**
 * @brief Check NAS integrity of a message
 * @param [in]  h           NAS handler
 * @param [in]  buf         Received NAS message
 * @param [in]  size        Received NAS message length
 * @param [in]  direction   0 for uplink, 1 for downlink
 * @param [out] isAuth      set to 1 if authentication is valid, 0 otherwise
 * @return 1 on success, 2 if NAS COUNT mismatch, 0 if any error
 *
 * Function to authenticate the NAS message. It validates the received MAC with
 * the calculated value.
 *
 * Don't use it for Plain NAS, the user is suposed to check the security context
 * beforehand using the function nas_getHeader.
 *
 * isAuth is only valid if the function returned 1.
 */
int nas_authenticateMsg(const NAS h,
                        const uint8_t *buf, const uint32_t size,
                        const NAS_Direction direction,
                        uint8_t *isAuth);


/**
 * @brief Decode a NAS message with security context.
 * @param [in]  h           NAS handler
 * @param [out] msg         Parsed NAS message
 * @param [in]  direction   0 for uplink, 1 for downlink
 * @param [in]  buf         Buffer with the NAS message to be parsed
 * @param [in]  size        Size of the NAS message buffer
 * @return 1 on success, 0 if any error
 *
 * Function to parse a NAS message with a security context. This functions
 * doesn't authenticate the message, use nas_authenticateMsg for that purpose
 *
 * Don't use it for Plain NAS, the user is suposed to check the security
 * context beforehand using the function nas_getHeader
 *
 * The decoded and deciphered  message is returned in the msg structure,
 * the security header is not provided, just the PlainNAS (NAS message from
 * octet 7 only).
 */
int dec_secNAS(const NAS h,
               GenericNASMsg_t *msg, const NAS_Direction direction,
               const uint8_t *buf, const size_t size);

/**
 * @brief Decode a NAS message without security context.
 * @param [out] msg         Parsed NAS message
 * @param [in]  buf         Buffer with the NAS message to be parsed
 * @param [in]  size        Size of the NAS message buffer
 * @return 1 on success, 0 if any error
 *
 * Function to parse a NAS message without a security context.
 *
 * Use  it only for Plain NAS, the user is suposed to check the security
 * context beforehand using the function nas_getHeader
 */
int dec_NAS(GenericNASMsg_t *msg, const uint8_t *buf, const size_t size);


void dec_ESM(ESM_Message_t *msg, const uint8_t *buf, const size_t size);


void newNASMsg_EMM(uint8_t **curpos,
                   ProtocolDiscriminator_t protocolDiscriminator,
                   SecurityHeaderType_t securityHeaderType);


void newNASMsg_ESM(uint8_t **curpos,
                   ProtocolDiscriminator_t protocolDiscriminator,
                   uint8_t ePSBearerId);


/**
 * @brief Encode a NAS message with security header
 * @param [in]  h           NAS handler
 * @param [out] out         Buffer to place the encoded NAS message
 * @param [out] len         Size of the NAS message buffer
 * @param [in]  p           Protocol Discriminator
 * @param [in]  s           Security Header type
 * @param [in]  direction   0 for uplink, 1 for downlink
 * @param [in]  plain       Encoded plain NAS message
 * @param [in]  pLen        Size of the NAS plain message buffer
 * @return 1 on success, 0 if any error
 *
 * Function to encode a NAS message with a security context. The function
 * is intelligent enough to know when cyphering is required depending on the
 * security header type provided.
 *
 * The out buffer needs to have enough space to store the resulting
 * message.
 */
int newNASMsg_sec(const NAS h,
                  uint8_t *out, size_t *len,
                  const ProtocolDiscriminator_t p,
                  const SecurityHeaderType_t s,
                  const NAS_Direction direction,
                  const uint8_t *plain, const size_t pLen);


void encaps_ESM(uint8_t **curpos,
                ProcedureTransactionId_t procedureTransactionIdentity,
                NASMessageType_t messageType);


void encaps_EMM(uint8_t **curpos,
                NASMessageType_t messageType);


/* Tool Functions*/

/**
 * @brief Check NAS integrity of a message
 * @param [in]  messageType   NAS message type
 * @return 1 if integrity check is required to process the message, else 0
 *
 * This function returns 1 with the message integrity check is mandatory to
 * process the message. If the message has to be processed even if the
 * integrity fails returns 0.
 *
 * The rules that are applied are the ones described in clause 4.4.4.3 of
 * TS 24.301. The exceptional cases are not contemplated and the developer
 * is required to check them:
 * - The Identity response has to be processed only if the parameter is IMSI
 * - The Detach Request if sent before security has been activated.
 */
uint8_t nas_isAuthRequired(const NASMessageType_t messageType);

/**
 * @brief get Optional IE
 * @param [in]  optionals   NAS optional IE array
 * @param [in]  maxOpts     Maximum number of optional IE in the array
 * @param [in]  iei         IE Identifier
 * @param [out] ie          IE found
 *
 * This function points the ie to the optional IE found in the optionals list.
 * If an IE with the specified IE ID is not found, NULL is returned
 *
 * The function returns on the first IEI 0 or if the maxOpts is reached
 */
void nas_NASOpt_lookup(const union nAS_ie_member *optionals,
                       const uint8_t maxOpts,
                       const uint8_t iei,
                       union nAS_ie_member const **ie);

#endif  /* !_NAS_H */
