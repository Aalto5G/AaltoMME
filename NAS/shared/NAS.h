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
 * @param [in]    ikey        NAS Integrity Key (16 bytes)
 * @param [in]    e           Encryption algorithm
 * @param [in]    ekey        NAS Encryption Key (16 bytes)
 * @param [in]    cb          NAS Count Overflow event callback
 * @param [in]    user_data   User data to call the overflow callback
 *
 * This function allows NULL parameters to maintain them unchanged, thus
 * allowing the to change some of them, except the integrity and encryption
 * algorithms that are always required.
 */
void nas_setSecurity(NAS h,
                     NAS_EIA i, const uint8_t *ikey,
                     NAS_EEA e, const uint8_t *ekey,
                     void (*cb)(void*), void* user_data);


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
 * @brief Check NAS integrity of a message
 * @param [in]  h           NAS handler
 * @param [in]  buf         Received NAS message
 * @param [in]  size        Received NAS message length
 * @param [in]  direction   0 for uplink, 1 for downlink
 * @param [out] isAuth      set to 1 if authentication is valid, 0 otherwise
 * @return 1 on success, 0 if any error
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
                        const uint8_t direction,
                        uint8_t *isAuth);


/**
 * @brief Decode a NAS message with security context.
 * @param [in]  h           NAS handler
 * @param [out] msg         Parsed NAS message
 * @param [in]  buf         Buffer with the NAS message to be parsed
 * @param [in]  size        Size of the NAS message buffer
 * @return 1 on success, 0 if any error
 *
 * Function to parse a NAS message with a security context. This functions
 * doesn't authenticate the message, use nas_authenticateMsg for that purpose
 *
 * Don't use it for Plain NAS, the user is suposed to check the security
 * context  beforehand using the function nas_getHeader
 */
int dec_secNAS(const NAS h,
               GenericNASMsg_t *msg,
               const uint8_t *buf, const uint32_t size);


void dec_NAS(GenericNASMsg_t *msg,
             const uint8_t *buf,
             const uint32_t size);


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
