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
int init_hss(const char *host, const char *db, const char *usr, const char *pw);

void disconnect_hss();

void HSS_getAuthVec(EMMCtx emm, GError **err);

void HSS_UpdateLocation(EMMCtx emm, const ServedGUMMEIs_t * sGUMMEIs);

void HSS_syncAuthVec(EMMCtx emm, uint8_t * auts, GError **err);

#endif /* HSS_HFILE */
