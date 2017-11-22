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
 * @file   S1AP_PDU.c
 * @author Vicent Ferrer
 * @date   April, 2013
 * @brief  Protocal Data Unit (PDU) definitions for S1AP.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "S1AP_PDU.h"
#include "S1APlog.h"

/* **************************************************************
--
-- Interface Elementary Procedure Class
--
-- *************************************************************

typedef struct S1AP_ELEMENTARY_PROCEDURE_c{
    void            *InitiatingMessage;
    void            *SuccessfulOutcome;
    void            *UnsuccessfulOutcome;
    ProcedureCode_t procedureCode;
    Criticality_e   criticality;
}S1AP_ELEMENTARY_PROCEDURE_t;

*/

void S1AP_freeMsg(S1AP_Message_t* self){

    if(!self){
        return;
    }
    if(self->pdu->value){
        if(self->pdu->value->freeContainer){
            self->pdu->value->freeContainer(self->pdu->value);
        }
    }
    free(self->pdu);

    /*Delete callbacks and free S1AP_Message_t*/
    self->freemsg = NULL;
    self->showmsg = NULL;
    free(self);
    self = NULL;
}

void show_Msg(S1AP_Message_t* self){
    /*Level 1: Packet*/
    printf("\nThe S1AP PDU looks like this:\n");
    printf("Message=%s(%u)  \tExtension=%u\n", MessageName[self->choice], self->choice, self->extension);
    printf("Procedure=%s(%u)\tCriticality=%s(%u)\n",
            elementaryProcedureName[self->pdu->procedureCode], self->pdu->procedureCode,
            CriticalityName[self->pdu->criticality], self->pdu->criticality);
    if(self->pdu->value->showIEs){
        self->pdu->value->showIEs(self->pdu->value);
    }
    printf("\n");
}

S1AP_Message_t *S1AP_newMsg(){
    S1AP_Message_t * self;

    /*Message allocation*/
    self = (S1AP_Message_t *)malloc(sizeof(S1AP_Message_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP message not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(S1AP_Message_t));

    /* PDU allocation*/
    self->pdu = malloc(sizeof(S1AP_PDU_t));
    if(!self->pdu){
        s1ap_msg(ERROR, 0, "S1AP PDU not allocated correctly");
        free(self);
        return NULL;
    }
    memset(self->pdu, 0, sizeof(S1AP_PDU_t));

    /*IE Container allocation*/
    self->pdu->value = new_ProtocolIE_Container();
    if(!self->pdu->value){
        free(self->pdu);
        free(self);
        return NULL;
    }

    self->freemsg = S1AP_freeMsg;
    self->showmsg = show_Msg;
    return self;
};

