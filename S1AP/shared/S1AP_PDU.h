/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S1AP_PDU.h
 * @author Vicent Ferrer
 * @date   April, 2013
 * @brief  Protocal Data Unit (PDU) definitions for S1AP.
 *
 */

#ifndef PDU_H_
#define PDU_H_

#include "Constants.h"
#include "CommonDataTypes.h"
#include "Containers.h"

/* **************************************************************
--
-- Interface Elementary Procedure Class
--
-- **************************************************************

S1AP-ELEMENTARY-PROCEDURE ::= CLASS {
    &InitiatingMessage              ,
    &SuccessfulOutcome              OPTIONAL,
    &UnsuccessfulOutcome                OPTIONAL,
    &procedureCode          ProcedureCode   UNIQUE,
    &criticality            Criticality     DEFAULT ignore
}
WITH SYNTAX {
    INITIATING MESSAGE          &InitiatingMessage
    [SUCCESSFUL OUTCOME         &SuccessfulOutcome]
    [UNSUCCESSFUL OUTCOME       &UnsuccessfulOutcome]
    PROCEDURE CODE              &procedureCode
    [CRITICALITY                &criticality]
}*/

typedef struct S1AP_ELEMENTARY_PROCEDURE_c{
    void            *InitiatingMessage;
    void            *SuccessfulOutcome;
    void            *UnsuccessfulOutcome;
    ProcedureCode_t procedureCode;
    Criticality_e   criticality;
}S1AP_ELEMENTARY_PROCEDURE_t;

/* **************************************************************
--
-- Interface PDU Definition
--
-- **************************************************************

S1AP-PDU ::= CHOICE {
    initiatingMessage   InitiatingMessage,
    successfulOutcome   SuccessfulOutcome,
    unsuccessfulOutcome UnsuccessfulOutcome,
    ...
}

InitiatingMessage ::= SEQUENCE {
    procedureCode   S1AP-ELEMENTARY-PROCEDURE.&procedureCode    ({S1AP-ELEMENTARY-PROCEDURES}),
    criticality S1AP-ELEMENTARY-PROCEDURE.&criticality          ({S1AP-ELEMENTARY-PROCEDURES}{@procedureCode}),
    value       S1AP-ELEMENTARY-PROCEDURE.&InitiatingMessage    ({S1AP-ELEMENTARY-PROCEDURES}{@procedureCode})
}

SuccessfulOutcome ::= SEQUENCE {
    procedureCode   S1AP-ELEMENTARY-PROCEDURE.&procedureCode    ({S1AP-ELEMENTARY-PROCEDURES}),
    criticality S1AP-ELEMENTARY-PROCEDURE.&criticality          ({S1AP-ELEMENTARY-PROCEDURES}{@procedureCode}),
    value       S1AP-ELEMENTARY-PROCEDURE.&SuccessfulOutcome    ({S1AP-ELEMENTARY-PROCEDURES}{@procedureCode})
}

UnsuccessfulOutcome ::= SEQUENCE {
    procedureCode   S1AP-ELEMENTARY-PROCEDURE.&procedureCode    ({S1AP-ELEMENTARY-PROCEDURES}),
    criticality S1AP-ELEMENTARY-PROCEDURE.&criticality          ({S1AP-ELEMENTARY-PROCEDURES}{@procedureCode}),
    value       S1AP-ELEMENTARY-PROCEDURE.&UnsuccessfulOutcome  ({S1AP-ELEMENTARY-PROCEDURES}{@procedureCode})
}
 */

/* This type correspond to InitiatingMessage, SuccessfulOutcome, UnsuccessfulOutcome types of the standard*/
typedef struct S1AP_PDU_c{
    ProcedureCode_t         procedureCode;
    Criticality_e           criticality;
    uint8_t                 ext;
    ProtocolIE_Container_t  *value;
    void                    *extensionValue;
}S1AP_PDU_t;

/*
typedef struct InitiatingMessage_c{
    ProcedureCode_t procedureCode;
    Criticality_e   criticality;
    void            *value;
}InitiatingMessage_t;

typedef struct SuccessfulOutcome_c{
    ProcedureCode_t procedureCode;
    Criticality_e   criticality;
    void            *value;
}SuccessfulOutcome_t;

typedef struct UnsuccessfulOutcome_c{
    ProcedureCode_t procedureCode;
    Criticality_e   criticality;
    void            *value;
}UnsuccessfulOutcome_t;
*/

/** This type correspond to S1AP_PDU type in the standard*/
typedef struct Message_c{
    uint8_t                     extension;
    enum TriggeringMessage_c    choice;
    S1AP_PDU_t                  *pdu;
    void                        (*freemsg)(struct Message_c*);
    void                        (*showmsg)(struct Message_c*);
}S1AP_Message_t;

S1AP_Message_t *S1AP_newMsg();

#endif /* PDU_H_ */
