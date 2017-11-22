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
 * @file   S1AP.h
 * @Author Vicent Ferrer
 * @date   April, 2013
 * @brief  S1AP definition
 *
 * This module implements the S1AP 3GPP protocol standard.
 */

#ifndef _S1AP_H
#define _S1AP_H
#include "Constants.h"
#include "CommonDataTypes.h"
#include "Containers.h"
#include "S1AP_PDU.h"
#include "S1AP_IE.h"

#define S1AP_PORT   36412

/* S1AP SCTP Payload Protocol Identifier = 18*/
#define SCTP_S1AP_PPID  htonl(18)

/**@brief Test Function
 *
 * Hello World!
 * */
extern void info();

/**@Decoder function
 *
 * The returned structure should be deallocated once used with its function pointer field S1AP_Message_t.freemsg.
 * */
extern S1AP_Message_t *s1ap_decode(void* data, uint32_t size);

/**@Encoder function
 * */
extern void s1ap_encode(uint8_t* data, uint32_t *size, S1AP_Message_t *msg);


/**@brief Find IE from message
 * @param [in] msg  Message to process
 * @param [in] id   Protocol Id to be extracted
 *
 * This function does not remove the IE from the message structure, use s1ap_getIe for this purpose*/
extern void *s1ap_findIe(S1AP_Message_t *msg, ProtocolIE_ID_t id);

/**@brief Get IE from message
 * @param [in] msg  Message to process
 * @param [in] id   Protocol Id to be extracted
 *
 * This function remove the IE from the message structure to preserve the IE after deleting the message*/
extern void *s1ap_getIe(S1AP_Message_t *msg, ProtocolIE_ID_t id);

/* ********************* Generic Structure ******************** */
/** @brief Constructor of generic IE
 *  @param [in] s1msg   s1 message structure pointer
 *  @param [in]
 * Link and allocate all the structures needed to add and IE to the message
 *
 * */
extern void *s1ap_newIE(S1AP_Message_t *s1msg, ProtocolIE_ID_t id, Presence_e p, Criticality_e c);


/*extern void s1ap_copyValueOnNewIE(S1AP_Message_t *s1msg, ProtocolIE_ID_t id, Presence_e p, Criticality_e c, GenericVal_t *val);*/

extern void s1ap_setValueOnNewIE(S1AP_Message_t *s1msg, ProtocolIE_ID_t id, Presence_e p, Criticality_e c, GenericVal_t *val);

#endif  /* !_S1AP_H */
