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

#endif /* CONTAINERS_H_ */
