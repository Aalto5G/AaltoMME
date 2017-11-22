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
 * @file   CommonDataTypes.h
 * @Author Vicent Ferrer
 * @date   April, 2013
 * @brief  S1AP Common data types definition
 *
 */

#ifndef _S1AP_COMM_DATA_TYPES_H
#define _S1AP_COMM_DATA_TYPES_H
#include <stdint.h>

/*Criticality     ::= ENUMERATED { reject, ignore, notify }*/
typedef enum Criticality_c{
    reject,
    ignore,
    notify
}Criticality_e;

extern const char * CriticalityName [];

/*Presence        ::= ENUMERATED { optional, conditional, mandatory }*/
typedef enum Presence_c{
    optional,
    conditional,
    mandatory
}Presence_e;

extern const char * PresenceName [];

/*PrivateIE-ID    ::= CHOICE {
    local               INTEGER (0..65535),
    global              OBJECT IDENTIFIER
}*/
/* Dependencies */
typedef enum PrivateIE_ID_PR_c{
    PrivateIE_ID_PR_NOTHING,    /* No components present */
    PrivateIE_ID_PR_local,
    PrivateIE_ID_PR_global
} PrivateIE_ID_PR;

/* PrivateIE-ID */
typedef struct PrivateIE_ID_c{
    PrivateIE_ID_PR present;
    union PrivateIE_ID_u {
        long     local;
        /*OBJECT_IDENTIFIER_t  global;*/
    }choice;
} PrivateIE_ID_t;


/* ProcedureCode       ::= INTEGER (0..255)*/
typedef uint8_t ProcedureCode_t;

/* ProtocolExtensionID ::= INTEGER (0..65535)*/
typedef uint16_t ProtocolExtensionID_t;

/* ProtocolIE-ID       ::= INTEGER (0..65535)*/
typedef uint16_t ProtocolIE_ID_t;

/* TriggeringMessage   ::= ENUMERATED { initiating-message, successful-outcome, unsuccessfull-outcome }  */
typedef enum TriggeringMessage_c{
    initiating_message,
    successful_outcome,
    unsuccessful_outcome,
    ext_not_implemented,
}TriggeringMessage_e;

extern const char * MessageName [];

#endif  /* !_S1AP_COMM_DATA_TYPES_H */
