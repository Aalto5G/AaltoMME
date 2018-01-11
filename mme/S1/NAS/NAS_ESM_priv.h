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
 * @file   NAS_ESM_priv.h
 * @Author Vicent Ferrer
 * @date   September, 2015
 * @brief  ESM logic
 *
 */

#ifndef NAS_ESM_PRIV_HFILE
#define NAS_ESM_PRIV_HFILE

#include <glib.h>
#include "NAS_ESM.h"

typedef struct{
    gpointer    emm;
    gpointer    s11_iface;
    GHashTable* bearers;
    GHashTable* sessions;
    uint8_t     next_ebi;
    void        (*cb)(gpointer);
    gpointer    args;
}ESM_t;

/* API to EPS session and bearers */

/**@brief
 * @param [in] em_h     ESM handler
 * @return
 *
 * */
gpointer esm_getS11iface(ESM esm_h);


#endif /* NAS_ESM_PRIV_HFILE */
