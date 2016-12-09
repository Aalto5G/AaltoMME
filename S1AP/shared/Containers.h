/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   Containers.h
 * @author Vicent Ferrer
 * @date   April, 2013
 * @brief  S1AP Containers Definitions
 *
 *Interesting Comments: http://www.obj-sys.com/docs/ASN1CInfoObjectsWhitePaper.pdf
 */

#ifndef CONTAINERS_H_
#define CONTAINERS_H_

#include "CommonDataTypes.h"
#include "Constants.h"

/* **************************************************************
--
-- Class Definition for Protocol IEs
--
-- *************************************************************/
/*
S1AP-PROTOCOL-IES ::= CLASS {
  &id               ProtocolIE-ID                   UNIQUE,
  &criticality      Criticality,
  &Value,
  &presence         Presence
}
WITH SYNTAX {
  ID                &id
  CRITICALITY       &criticality
  TYPE              &Value
  PRESENCE          &presence
}
*/
typedef struct S1AP_PROTOCOL_IES_c{
    ProtocolIE_ID_t id;
    Criticality_e   criticality;
    void*           value;
    Presence_e      presence;
    void            (*freeIE)(struct S1AP_PROTOCOL_IES_c *self);    /*< method to remove the S1AP_PROTOCOL_IES_t structure*/
    void            (*freeValue)(void *);                           /*< method to remove the value structure. If value type is not known, the free method is stored here*/
    void            (*showIE)(struct S1AP_PROTOCOL_IES_c *self);
    void            (*showValue)(void *);

}S1AP_PROTOCOL_IES_t;

/** S1AP_PROTOCOL_IES_t Constructor*/
extern S1AP_PROTOCOL_IES_t * newProtocolIE();

/* **************************************************************
--
-- Class Definition for Protocol IEs
--
-- *************************************************************/
/*
S1AP-PROTOCOL-IES-PAIR ::= CLASS {
    &id             ProtocolIE-ID                   UNIQUE,
    &firstCriticality       Criticality,
    &FirstValue,
    &secondCriticality      Criticality,
    &SecondValue,
    &presence           Presence
}
WITH SYNTAX {
    ID              &id
    FIRST CRITICALITY       &firstCriticality
    FIRST TYPE          &FirstValue
    SECOND CRITICALITY      &secondCriticality
    SECOND TYPE         &SecondValue
    PRESENCE            &presence
}
*/

typedef struct S1AP_PROTOCOL_IES_PAIR_c{
    ProtocolIE_ID_t id;
    Criticality_e   firstCriticality;
    void*           firstValue;
    Criticality_e   secondCriticality;
    void*           secondValue;
    Presence_e      presence;
    void            (*freeIE)(struct S1AP_PROTOCOL_IES_PAIR_c *self);
    void            (*freeValue1)(struct S1AP_PROTOCOL_IES_PAIR_c *self);
    void            (*freeValue2)(struct S1AP_PROTOCOL_IES_PAIR_c *self);
}S1AP_PROTOCOL_IES_PAIR_t;

/** S1AP_PROTOCOL_IES_PAIR_t Constructor*/
extern S1AP_PROTOCOL_IES_PAIR_t * newProtocolIEPair();

/* **************************************************************
--
-- Class Definition for Protocol Extensions
--
-- **************************************************************
*//*
S1AP-PROTOCOL-EXTENSION ::= CLASS {
    &id             ProtocolExtensionID                 UNIQUE,
    &criticality            Criticality,
    &Extension,
    &presence       Presence
}
WITH SYNTAX {
    ID              &id
    CRITICALITY         &criticality
    EXTENSION           &Extension
    PRESENCE        &presence
}
*/
typedef struct S1AP_PROTOCOL_EXTENSION_c{
    ProtocolExtensionID_t   id;
    Criticality_e           criticality;
    void*                   extension;
    Presence_e              presence;
    void                    (*freeProtExt)(struct S1AP_PROTOCOL_EXTENSION_c *self);
    void                    (*freeExtension)(struct S1AP_PROTOCOL_EXTENSION_c *self);
}S1AP_PROTOCOL_EXTENSION_t;

/** S1AP_PROTOCOL_EXTENSION_t Constructor*/
extern S1AP_PROTOCOL_EXTENSION_t * newProtocolExtension();

/* **************************************************************
--
-- Class Definition for Private IEs
--
-- **************************************************************

S1AP-PRIVATE-IES ::= CLASS {
    &id             PrivateIE-ID,
    &criticality            Criticality,
    &Value,
    &presence       Presence
}
WITH SYNTAX {
    ID              &id
    CRITICALITY         &criticality
    TYPE            &Value
    PRESENCE        &presence
}
*/

/* **************************************************************
--
-- Container for Protocol IEs
--
-- *************************************************************/
/*
ProtocolIE-Container {S1AP-PROTOCOL-IES : IEsSetParam} ::=
    SEQUENCE (SIZE (0..maxProtocolIEs)) OF
    ProtocolIE-Field {{IEsSetParam}}

ProtocolIE-SingleContainer {S1AP-PROTOCOL-IES : IEsSetParam} ::=
    ProtocolIE-Field {{IEsSetParam}}

ProtocolIE-Field {S1AP-PROTOCOL-IES : IEsSetParam} ::= SEQUENCE {
    id              S1AP-PROTOCOL-IES.&id               ({IEsSetParam}),
    criticality         S1AP-PROTOCOL-IES.&criticality          ({IEsSetParam}{@id}),
    value               S1AP-PROTOCOL-IES.&Value                ({IEsSetParam}{@id})
}
*/
typedef struct ProtocolIE_Field_c{
    ProtocolIE_ID_t id;
    Criticality_e   criticality;
    void*           value;
}ProtocolIE_Field_t;

typedef struct ProtocolIE_Container_c {
/*   uint16_t             n;      *//*< Number of current IE on the containter. The maximum number is maxProtocolIEs = 65535*/
   uint16_t             size;   /*< Number of IE expected*/
   S1AP_PROTOCOL_IES_t  **elem;
   /*ProtocolIE_Field elem[maxProtocolIEs];*/
   void     (*freeContainer)(struct ProtocolIE_Container_c*);
   void     (*showIEs)(struct ProtocolIE_Container_c*);
   void     (*addIe)(struct ProtocolIE_Container_c*, S1AP_PROTOCOL_IES_t* ie);
} ProtocolIE_Container_t;

ProtocolIE_Container_t *new_ProtocolIE_Container();

typedef S1AP_PROTOCOL_IES_t ProtocolIE_SingleContainer_t;

/* **************************************************************
--
-- Container for Protocol IE Pairs
--
-- **************************************************************

ProtocolIE-ContainerPair {S1AP-PROTOCOL-IES-PAIR : IEsSetParam} ::=
    SEQUENCE (SIZE (0..maxProtocolIEs)) OF
    ProtocolIE-FieldPair {{IEsSetParam}}

ProtocolIE-FieldPair {S1AP-PROTOCOL-IES-PAIR : IEsSetParam} ::= SEQUENCE {
    id              S1AP-PROTOCOL-IES-PAIR.&id          ({IEsSetParam}),
    firstCriticality        S1AP-PROTOCOL-IES-PAIR.&firstCriticality    ({IEsSetParam}{@id}),
    firstValue          S1AP-PROTOCOL-IES-PAIR.&FirstValue          ({IEsSetParam}{@id}),
    secondCriticality       S1AP-PROTOCOL-IES-PAIR.&secondCriticality   ({IEsSetParam}{@id}),
    secondValue         S1AP-PROTOCOL-IES-PAIR.&SecondValue         ({IEsSetParam}{@id})
}
*/
typedef struct ProtocolIE_FieldPair_c{
    ProtocolIE_ID_t id;
    Criticality_e   firstCriticality;
    void*           firstValue;
    Criticality_e   secondCriticality;
    void*           secondValue;
}ProtocolIE_FieldPair_t;

typedef struct ProtocolIE_ContainerPair_c {
   uint16_t                     n;      /*< The maximum number is maxProtocolIEs = 65535*/
   ProtocolIE_FieldPair_t     *elem;
   /*S1AP_PROTOCOL_IES_PAIR_t elem[maxProtocolIEs];*/
} ProtocolIE_ContainerPair_t;

/* **************************************************************
--
-- Container Lists for Protocol IE Containers
--
-- **************************************************************

ProtocolIE-ContainerList {INTEGER : lowerBound, INTEGER : upperBound, S1AP-PROTOCOL-IES : IEsSetParam} ::=
    SEQUENCE (SIZE (lowerBound..upperBound)) OF
    ProtocolIE-SingleContainer {{IEsSetParam}}

ProtocolIE-ContainerPairList {INTEGER : lowerBound, INTEGER : upperBound, S1AP-PROTOCOL-IES-PAIR : IEsSetParam} ::=
    SEQUENCE (SIZE (lowerBound..upperBound)) OF
    ProtocolIE-ContainerPair {{IEsSetParam}}
*/

typedef ProtocolIE_Container_t      ProtocolIE_ContainerList[];
typedef ProtocolIE_ContainerPair_t  ProtocolIE_ContainerPairList[];


/* **************************************************************
--
-- Container for Protocol Extensions
--
-- **************************************************************

ProtocolExtensionContainer {S1AP-PROTOCOL-EXTENSION : ExtensionSetParam} ::=
    SEQUENCE (SIZE (1..maxProtocolExtensions)) OF
    ProtocolExtensionField {{ExtensionSetParam}}

ProtocolExtensionField {S1AP-PROTOCOL-EXTENSION : ExtensionSetParam} ::= SEQUENCE {
    id              S1AP-PROTOCOL-EXTENSION.&id         ({ExtensionSetParam}),
    criticality         S1AP-PROTOCOL-EXTENSION.&criticality        ({ExtensionSetParam}{@id}),
    extensionValue          S1AP-PROTOCOL-EXTENSION.&Extension      ({ExtensionSetParam}{@id})
}
*/
typedef struct ProtocolExtensionField_c{
    ProtocolIE_ID_t id;
    Criticality_e   firstCriticality;
    void*           extensionValue;
}ProtocolExtensionField_t;

typedef struct ProtocolExtensionContainer_c{
   uint16_t                     size;      /*< The maximum number is maxProtocolExtensions = 65535*/
   S1AP_PROTOCOL_IES_t          **elem;
   /*ProtocolExtensionField_t     elem[maxProtocolExtensions];*/
   void     (*freeExtensionContainer)(struct ProtocolExtensionContainer_c*);
   void     (*showExtensionContainer)(struct ProtocolExtensionContainer_c*);
   void     (*addIe)(struct ProtocolExtensionContainer_c*, S1AP_PROTOCOL_IES_t* ie);
} ProtocolExtensionContainer_t;

ProtocolExtensionContainer_t *new_ProtocolExtensionContainer();

/* **************************************************************
--
-- Container for Private IEs
--
-- **************************************************************

PrivateIE-Container {S1AP-PRIVATE-IES : IEsSetParam } ::=
    SEQUENCE (SIZE (1.. maxPrivateIEs)) OF
    PrivateIE-Field {{IEsSetParam}}

PrivateIE-Field {S1AP-PRIVATE-IES : IEsSetParam} ::= SEQUENCE {
    id              S1AP-PRIVATE-IES.&id            ({IEsSetParam}),
    criticality         S1AP-PRIVATE-IES.&criticality       ({IEsSetParam}{@id}),
    value           S1AP-PRIVATE-IES.&Value     ({IEsSetParam}{@id})
}

 */

/*
-- **************************************************************
--
-- Common Container Lists
--
-- **************************************************************

E-RAB-IE-ContainerList          { S1AP-PROTOCOL-IES      : IEsSetParam }    ::= ProtocolIE-ContainerList     { 1, maxNrOfE-RABs,   {IEsSetParam} }
E-RAB-IE-ContainerPairList      { S1AP-PROTOCOL-IES-PAIR : IEsSetParam }    ::= ProtocolIE-ContainerPairList { 1, maxNrOfE-RABs,   {IEsSetParam} }
ProtocolError-IE-ContainerList  { S1AP-PROTOCOL-IES      : IEsSetParam }    ::= ProtocolIE-ContainerList     { 1, maxNrOfE-RABs,   {IEsSetParam} }
*/

typedef ProtocolIE_Container_t E_RAB_IE_ContainerList_t;

#define SEQ_OF_HEADER(list_name, item_name) \
typedef struct list_name##_c{ \
    void                                (*freeIE)(void *); \
    void                                (*showIE)(void *); \
    void                                (*additem)(struct list_name##_c*, item_name##_t* ie); \
    void                                *(*newItem)(struct list_name##_c*); \
    uint8_t                             size;               /*<Number of items*/ \
    item_name##_t                       **item;             /*<Item Array */ \
}list_name##_t; \
 \
list_name##_t *new_##list_name()


#define SEQ_OF_CONTAINER_HEADER(list_name, item_name)                       \
typedef struct list_name##_c{                                               \
    void                                (*freeIE)(void *);                  \
    void                                (*showIE)(void *);                  \
    void                                (*additem)(struct list_name##_c*,   \
                                                   ProtocolIE_SingleContainer_t* ie); \
    void                                *(*newItem)(struct list_name##_c*); \
    uint8_t                             size;          /*<Number of items*/ \
    ProtocolIE_SingleContainer_t        **item;        /*<Item Array */     \
}list_name##_t;                                                             \
                                                                            \
list_name##_t *new_##list_name()


#define SEQ_OF_FUNC(list_name, item_name, max_items)                        \
void free_##list_name(void * data){                                         \
    uint16_t i;                                                             \
    list_name##_t *self = (list_name##_t*)data;                             \
    if(!self){                                                              \
        return;                                                             \
    }                                                                       \
                                                                            \
    for(i=0; i<self->size;i++){                                             \
        if(self->item[i]->freeItem){                                        \
            self->item[i]->freeItem(self->item[i]);                         \
        }                                                                   \
    }                                                                       \
                                                                            \
    free(self->item);                                                       \
    free(self);                                                             \
}                                                                           \
                                                                            \
void show_##list_name(void * data){                                         \
    list_name##_t *self = (list_name##_t*)data;                             \
    uint16_t i;                                                             \
                                                                            \
    for(i=0; i < self->size; i++){                                          \
        if(&(self->item[i]) == NULL){                                       \
            printf("\n" #item_name " #%u not found\n", i);                  \
            continue;                                                       \
        }                                                                   \
        if(self->item[i]->showItem){                                        \
            self->item[i]->showItem(self->item[i]);                         \
        }else{                                                              \
            printf("\t\t" #item_name " #%u: show function not found\n", i); \
        }                                                                   \
    }                                                                       \
                                                                            \
}                                                                           \
                                                                            \
void list_name##_addItem(list_name##_t* c, item_name##_t* item){            \
    item_name##_t** vector;                                                 \
    if(c->size+1 == max_items ){                                            \
        s1ap_msg(ERROR, 0, #max_items "  reached");                         \
        return;                                                             \
    }                                                                       \
                                                                            \
    c->size++;                                                              \
    vector = (item_name##_t**) realloc (c->item,                            \
                                        c->size * sizeof(item_name##_t*));  \
                                                                            \
    /*Error Check*/                                                         \
    if (vector!=NULL) {                                                     \
        c->item=vector;                                                     \
        c->item[c->size-1]=item;                                            \
    }                                                                       \
    else {                                                                  \
      free (c->item);                                                       \
      s1ap_msg(ERROR, 0, "Error (re)allocating memory");                    \
    }                                                                       \
}                                                                           \
                                                                            \
list_name##_t *new_##list_name(){                                           \
    list_name##_t *self;                                                    \
                                                                            \
    self = malloc(sizeof(list_name##_t));                                   \
    if(!self){                                                              \
        s1ap_msg(ERROR, 0, "S1AP " #list_name                               \
                 "_t not allocated correctly");                             \
        return NULL;                                                        \
    }                                                                       \
    memset(self, 0, sizeof(list_name##_t));                                 \
                                                                            \
    self->freeIE=free_##list_name;                                          \
    self->showIE=show_##list_name;                                          \
    self->additem=list_name##_addItem;                                      \
                                                                            \
    return self;                                                            \
}


#define SEQ_OF_CONTAINER_FUNC(list_name, item_name,                         \
                              item_presence, item_criticality, max_items)   \
void free_##list_name(void * data){                                         \
    uint16_t i;                                                             \
    list_name##_t *self = (list_name##_t*)data;                             \
    if(!self){                                                              \
        return;                                                             \
    }                                                                       \
                                                                            \
    for(i=0; i<self->size;i++){                                             \
        if(self->item[i]->freeIE){                                          \
            self->item[i]->freeIE(self->item[i]);                           \
        }                                                                   \
    }                                                                       \
                                                                            \
    free(self->item);                                                       \
    free(self);                                                             \
}                                                                           \
                                                                            \
void show_##list_name(void * data){                                         \
    list_name##_t *self = (list_name##_t*)data;                             \
    S1AP_PROTOCOL_IES_t *item;                                              \
    uint16_t i;                                                             \
                                                                            \
    for(i=0; i < self->size; i++){                                          \
        if(&(self->item[i]) == NULL){                                       \
            printf("\n" #item_name " #%u not found\n", i);                  \
            continue;                                                       \
        }                                                                   \
        item = (S1AP_PROTOCOL_IES_t*) self->item[i];                        \
        if(item->showIE){                                                   \
            item->showIE(item);                                             \
        }else{                                                              \
            printf("\t\t\t" #item_name " #%u: "                             \
                "show function not found\n", i);                            \
        }                                                                   \
    }                                                                       \
                                                                            \
}                                                                           \
                                                                            \
void list_name##_addItem(list_name##_t* c,                                  \
                         ProtocolIE_SingleContainer_t* item){               \
    ProtocolIE_SingleContainer_t** vector;                                  \
    if(c->size+1 == max_items ){                                            \
        s1ap_msg(ERROR, 0, #max_items "  reached");                         \
        return;                                                             \
    }                                                                       \
    s1ap_msg(WARN, 0, "Test");                                              \
                                                                            \
    c->size++;                                                              \
    s1ap_msg(WARN, 0, "Test");                                              \
    vector = (ProtocolIE_SingleContainer_t**) realloc (c->item,             \
                                        c->size * sizeof(ProtocolIE_SingleContainer_t*));  \
    s1ap_msg(WARN, 0, "Test");                                              \
                                                                            \
    /*Error Check*/                                                         \
    if (vector!=NULL) {                                                     \
        c->item=vector;                                                     \
        c->item[c->size-1]=item;                                            \
    }                                                                       \
    else {                                                                  \
      free (c->item);                                                       \
      s1ap_msg(ERROR, 0, "Error (re)allocating memory");                    \
    }                                                                       \
}                                                                           \
                                                                            \
void *list_name##_newItem(struct list_name##_c* list){                      \
    S1AP_PROTOCOL_IES_t* ie = newProtocolIE();                              \
    item_name##_t *item = new_##item_name();                                \
    ie->value = item;                                                       \
    ie->showValue = item->showIE;                                           \
    ie->freeValue = item->freeIE;                                           \
    ie->id = id_##item_name;                                                \
    ie->presence = item_presence;                                           \
    ie->criticality = item_criticality;                                     \
    s1ap_msg(WARN, 0, "Test");                                              \
    list->additem(list, ie);                                                \
    s1ap_msg(WARN, 0, "Test");                                              \
    return item;                                                            \
}                                                                           \
                                                                            \
list_name##_t *new_##list_name(){                                           \
    list_name##_t *self;                                                    \
                                                                            \
    self = malloc(sizeof(list_name##_t));                                   \
    if(!self){                                                              \
        s1ap_msg(ERROR, 0, "S1AP " #list_name                               \
                 "_t not allocated correctly");                             \
        return NULL;                                                        \
    }                                                                       \
    memset(self, 0, sizeof(list_name##_t));                                 \
                                                                            \
    self->freeIE=free_##list_name;                                          \
    self->showIE=show_##list_name;                                          \
    self->additem=list_name##_addItem;                                      \
    self->newItem = list_name##_newItem;                                    \
                                                                            \
    return self;                                                            \
}

#define SEQ_OF_CONTAINER_ENC(list_name, max_items)                          \
void enc_##list_name(struct BinaryData *bytes, S1AP_PROTOCOL_IES_t * ie){   \
    uint8_t i;                                                              \
    list_name##_t *v = (list_name##_t *)ie->value;                          \
    /*Encode length*/                                                       \
    encode_constrained_number(bytes, v->size, 1, max_items);                \
    /*Encode item*/                                                         \
    for(i=0 ; i < v->size ; i++){                                           \
        enc_protocolIEs(bytes, (S1AP_PROTOCOL_IES_t*)v->item[i]);           \
    }                                                                       \
}

#define SEQ_OF_CONTAINER_DEC(list_name, max_items)                          \
void dec_##list_name(S1AP_PROTOCOL_IES_t * ie, struct BinaryData *bytes){   \
    uint8_t i, length;                                                      \
    S1AP_PROTOCOL_IES_t *fakeie;                                            \
    list_name##_t *v = new_##list_name();                                   \
    /*Link functions*/                                                      \
    ie->showValue = v->showIE;                                              \
    ie->freeValue = v->freeIE;                                              \
    ie->value=v;                                                            \
    /*Decode length*/                                                       \
    length = decode_constrained_number(bytes, 1, max_items);                \
    /*Decode item*/                                                         \
    for(i=0 ; i < length ; i++){                                            \
        fakeie = dec_protocolIEs(bytes);                                    \
        v->additem(v, (ProtocolIE_SingleContainer_t*)fakeie);               \
    }                                                                       \
}


#endif /* CONTAINERS_H_ */
