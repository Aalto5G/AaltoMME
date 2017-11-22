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
 * @file   Containers.c
 * @author Vicent Ferrer
 * @date   April, 2013
 * @brief  S1AP Containers Definitions
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Containers.h"
#include "S1APlog.h"

/* **************************************************************
--
-- Class Definition for Protocol IEs
--
-- *************************************************************/
/*Functions to deallocate the S1AP_PROTOCOL_IES_t structures*/
void dealocateProtocolIEs (S1AP_PROTOCOL_IES_t *self){

    if(!self){
        return;
    }

    /*Dealocate Value*/
    if(self->freeValue!=NULL){
        self->freeValue(self->value);
    }
    /*Dealocate IE structure*/
    free(self);
}

void show_ProtocolIEs (S1AP_PROTOCOL_IES_t *self){

    printf("\t\tID=%s(%u)\tCritically=%s(%u)\tPresence=%s(%u)\n",
            IEName[self->id], self->id,
            CriticalityName[self->criticality], self->criticality,
            PresenceName[self->presence], self->presence);

    if(self->showValue){
        self->showValue(self->value);
    }

}
/* ******************** IE constructor ******************** */
S1AP_PROTOCOL_IES_t * newProtocolIE(){
    S1AP_PROTOCOL_IES_t * self;
    self = (S1AP_PROTOCOL_IES_t *)malloc(sizeof(struct S1AP_PROTOCOL_IES_c));
    if(!self){
        s1ap_msg(ERROR, 0, "IE not allocated");
    }

    memset(self, 0, sizeof(S1AP_PROTOCOL_IES_t));
    self->freeIE = dealocateProtocolIEs;
    self->freeValue = NULL;
    self->showIE = show_ProtocolIEs;
    self->showValue = NULL;
    return self;
}

/* **************************************************************
--
-- Class Definition for Protocol IEs
--
-- *************************************************************/
/*Functions to deallocate the S1AP_PROTOCOL_IES_t structures*/
void dealocateProtocolIEsPair (S1AP_PROTOCOL_IES_PAIR_t *self){

    /*Dealocate Values*/
    if(self->freeValue1!=NULL){
        self->freeValue1(self);
    }
    if(self->freeValue2!=NULL){
        self->freeValue2(self);
    }
    /*Dealocate IE structure*/
    free(self);
}

/* ******************** IE constructor ******************** */
S1AP_PROTOCOL_IES_PAIR_t * newProtocolIEPair(){
    S1AP_PROTOCOL_IES_PAIR_t * self;
    self = (S1AP_PROTOCOL_IES_PAIR_t *)malloc(sizeof(S1AP_PROTOCOL_IES_PAIR_t));
    if(!self){
        s1ap_msg(ERROR, 0, "IE not allocated");
    }

    memset(self, 0, sizeof(S1AP_PROTOCOL_IES_PAIR_t));
    self->freeIE = dealocateProtocolIEsPair;
    self->freeValue1 = NULL;
    self->freeValue2 = NULL;
    return self;
}

/* **************************************************************
--
-- Class Definition for Protocol Extensions
--
-- **************************************************************
*/
/*Functions to deallocate the S1AP_PROTOCOL_IES_t structures*/
void dealocateProtocolExtension (S1AP_PROTOCOL_EXTENSION_t *self){

    /*Dealocate Values*/
    if(self->freeExtension!=NULL){
        self->freeExtension(self);
    }
    /*Dealocate IE structure*/
    free(self);
}

/* ******************** IE constructor ******************** */
S1AP_PROTOCOL_EXTENSION_t * newProtocolExtension(){
    S1AP_PROTOCOL_EXTENSION_t * self;
    self = (S1AP_PROTOCOL_EXTENSION_t *)malloc(sizeof(S1AP_PROTOCOL_EXTENSION_t));
    if(!self){
        s1ap_msg(ERROR, 0, "Extension not allocated");
    }

    memset(self, 0, sizeof(S1AP_PROTOCOL_EXTENSION_t));
    self->freeProtExt = dealocateProtocolExtension;
    self->freeExtension = NULL;
    return self;
}


/* ******************** protocolIE_Container Methods ******************** */
void free_ProtocolIE_Container(ProtocolIE_Container_t *self){
    uint16_t i;

    if(!self){
        return;
    }
    for(i=0; i<self->size;i++){
        if(self->elem[i]){
            self->elem[i]->freeIE(self->elem[i]);
        }
    }

    free(self->elem);
    free(self);
}

void showIEs_ProtocolIE_Container(ProtocolIE_Container_t *self){
    uint16_t i;

    printf("\tTotal IEs=%u\n", self->size);
    if(self->size!=self->size){
        printf("The expected (%u) and current number (%u) of IE is diferent", self->size, self->size);
    }
    for(i=0; i < self->size; i++){
        if(self->elem[i] == NULL){
            printf("\nIE #%u not found\n", i);
            continue;
        }
        if(self->elem[i]->showIE){
            printf("\tIE #%u 0x%p:\n", i, self->elem[i]);
            self->elem[i]->showIE(self->elem[i]);
        }else{
            printf("\tIE #%u: show function not found\n", i);
        }
    }
}

void protocolIE_Container_addIE(ProtocolIE_Container_t* c, S1AP_PROTOCOL_IES_t* ie){
    S1AP_PROTOCOL_IES_t** vector;

    if(c->size+1==maxProtocolIEs){
        s1ap_msg(ERROR, 0, "maxProtocolIEs reached");
        return;
    }

    c->size++;
    vector = (S1AP_PROTOCOL_IES_t**) realloc (c->elem, c->size * sizeof(S1AP_PROTOCOL_IES_t*));

    /*Error Check*/
    if (vector!=NULL) {
        c->elem=vector;
        c->elem[c->size-1]=ie;
    }
    else {
      free (c->elem);
      s1ap_msg(ERROR, 0, "Error (re)allocating memory");
    }
}

ProtocolIE_Container_t *new_ProtocolIE_Container(){
    ProtocolIE_Container_t *self;

    self = malloc(sizeof(ProtocolIE_Container_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP protocolIE_Container not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(ProtocolIE_Container_t));

    self->freeContainer=free_ProtocolIE_Container;
    self->showIEs=showIEs_ProtocolIE_Container;
    self->addIe = protocolIE_Container_addIE;
    return self;
}


/* ******************** ProtocolExtensionContainer Methods ******************** */
void free_ProtocolExtensionContainer(ProtocolExtensionContainer_t *self){
    uint16_t i;

    if(!self){
        return;
    }
    for(i=0; i<self->size;i++){
        if(self->elem[i]){
            self->elem[i]->freeIE(self->elem[i]);
        }
    }
    free(self);
}

void showIEs_ProtocolExtensionContainer(ProtocolExtensionContainer_t *self){
    uint16_t i;

    printf("\tTotal IEs=%u\n", self->size);
    if(self->size!=self->size){
        printf("The expected (%u) and current number (%u) of IE is diferent", self->size, self->size);
    }
    for(i=0; i < self->size; i++){
        if(self->elem[i] == NULL){
            printf("\nIE #%u not found\n", i);
            continue;
        }
        if(self->elem[i]->showIE){
            printf("\tIE #%u 0x%p:\n", i, self->elem[i]);
            self->elem[i]->showIE(self->elem[i]);
        }else{
            printf("\tIE #%u: show function not found\n", i);
        }
    }
}

void ProtocolExtensionContainer_addIE(ProtocolExtensionContainer_t* c, S1AP_PROTOCOL_IES_t* ie){
    S1AP_PROTOCOL_IES_t** vector;

    if(c->size+1==maxProtocolExtensions){
        s1ap_msg(ERROR, 0, "maxProtocolExtensions reached");
        return;
    }

    c->size++;
    vector = (S1AP_PROTOCOL_IES_t**) realloc (c->elem, c->size * sizeof(S1AP_PROTOCOL_IES_t*));

    /*Error Check*/
    if (vector!=NULL) {
        c->elem=vector;
        c->elem[c->size-1]=ie;
    }
    else {
      free (c->elem);
      s1ap_msg(ERROR, 0, "Error (re)allocating memory");
    }
}

ProtocolExtensionContainer_t *new_ProtocolExtensionContainer(){
    ProtocolExtensionContainer_t *self;

    self = malloc(sizeof(ProtocolIE_Container_t));
    if(!self){
        s1ap_msg(ERROR, 0, "S1AP protocolIE_Container not allocated correctly");
        return NULL;
    }
    memset(self, 0, sizeof(ProtocolIE_Container_t));

    self->freeExtensionContainer=free_ProtocolExtensionContainer;
    self->showExtensionContainer=showIEs_ProtocolExtensionContainer;
    self->addIe = ProtocolExtensionContainer_addIE;
    return self;
}



