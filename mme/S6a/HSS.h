/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   HSS.h
 * @Author Vicent Ferrer
 * @date   June, 2013
 * @brief  Functions to access to HSS database
 *
 * The current database is a MariaDB.
 */


#ifndef HSS_HFILE
#define HSS_HFILE

#include "EMMCtx_iface.h"
#include "S1AP.h"
#include <glib.h>

#define DIAMETER diameter_quark()

GQuark diameter_quark();

typedef enum{
    DIAMETER_AUTHENTICATION_DATA_UNAVAILABLE = 4181,
    DIAMETER_ERROR_USER_UNKNOWN = 5001,
    DIAMETER_ERROR_ROAMING_NOT_ALLOWED = 5004,
    DIAMETER_UNKNOWN_EPS_SUBSCRIPTION = 5420,
    DIAMETER_ERROR_RAT_NOT_ALLOWED = 5421,
    DIAMETER_ERROR_EQUIPMENT_UNKNOWN = 5422,
    DIAMETER_ERROR_UNKOWN_SERVING_NODE = 5423,
}DiameterCause;

/* Functions Called from the MME initialize and destroy methods*/
int init_hss();

void disconnect_hss();

void HSS_getAuthVec(EMMCtx emm, GError **err);

void HSS_UpdateLocation(EMMCtx emm, const ServedGUMMEIs_t * sGUMMEIs);

void HSS_syncAuthVec(EMMCtx emm, uint8_t * auts, GError **err);

#endif /* HSS_HFILE */
