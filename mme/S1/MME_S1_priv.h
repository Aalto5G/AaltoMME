/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   MME_S1.h
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  MME S1 interface protocol state machine.
 *
 * This module implements the S1 interface state machine.
 */

#ifndef MME_S1_PRIV_HFILE
#define MME_S1_PRIVHFILE

#include <netinet/sctp.h>
#include <glib.h>

#include "MME.h"
#include "S1AP.h"

/* Interfaces */
#include "S1Assoc.h"

typedef struct{
    gpointer    mme;   /**< mme handler*/
    int         fd;    /**< file descriptor of the s11 server*/
    GHashTable  *assocs; /**< SCTP endpoint connections*/
}S1_t;

/* ======================================================================
 * S1 API to S1Assoc
 * ====================================================================== */

/**@brief S1 Register eNB
 * @param [in] s1_h    Destination EndPoint information
 * @param [in] assoc   Association to be registered in S1
 *
 * This function registers a new S1 association to the S1 interface
 * */
void s1_registerAssoc(S1 s1_h, gpointer assoc);

struct mme_t *s1_getMME(S1_t *self);

#endif /* MME_S1_PRIV_HFILE */
