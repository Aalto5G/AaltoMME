/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   EPS_Session.h
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  EPS_Session Information
 *
 * This module implements the EPS_Session. This header is private
 */

#ifndef EPS_SESSION_H
#define EPS_SESSION_H

#include <glib.h>

#include "EPS_Session_iface.h"
#include "Subscription.h"

typedef struct{
	Subscription      subs;  /**< Subscription information*/
    GString           *APN;
    /* apn_restriction; */
    GString           *subscribedAPN;
    guint8            pdn_addr_type;
    guint8            pdn_addr[20];
    guint16           charging_characteristics;
    GString           *apn_io_replacement;


    struct qos_t      qos;
    guint64           apn_ambr_dl;
    guint64           apn_ambr_ul;
    guint8            default_ebi;

    uint8_t           pco[0xff+2];   /* TLV Protocol Configuration Options*/

    GHashTable        *bearers;

}EPS_Session_t;


#endif /* EPS_SESSION_H*/
