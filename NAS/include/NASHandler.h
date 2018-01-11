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
 * @file   NAS.h
 * @Author Vicent Ferrer
 * @date   September, 2015
 * @brief  Internal NAS handler
 */

#ifndef _NAS_HANDLER_H
#define _NAS_HANDLER_H

#include "eia0.h"
#include "eia2.h"

#include "eea0.h"

#include "NAS.h"

typedef void (*EIAcb) (const void *k,
                       const void *count, const uint8_t bearer,
                       const uint8_t direction,
                       const void* msg, const size_t mLen,
                       void *digest);

typedef void (*EEAdec_cb) (const void *k,
                            const void *count, const uint8_t bearer,
                            const uint8_t direction,
                            const void* msg, const size_t mLen,
                            void* plain, size_t *pLen);

typedef void (*EEAcyph_cb) (const void *k,
                            const void *count, const uint8_t bearer,
                            const uint8_t direction,
                            void* msg, size_t *mLen,
                            const void* plain, const size_t pLen);

static const EIAcb eia_cb[] = {
    eia0,
    NULL,
    eia2,
    NULL,
    NULL,
    NULL,
    NULL,
};

static const EEAdec_cb eea_dec_cb[] = {
    eea0_dec,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

static const EEAcyph_cb eea_cyph_cb[] = {
    eea0_cyph,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};


typedef struct{
    uint8_t  isValid;                  /**< 1 if the structure is filled,
                                            0 otherwise*/
    NAS_EIA  i;                        /**< NAS Integrity Algorithm*/
    uint8_t  ikey[16];                 /**< NAS Integrity Key*/
    NAS_EEA  e;                        /**< NAS Encryption Algorithm */
    uint8_t  ekey[16];                 /**< NAS Encryption Key*/
    uint32_t nas_count[2];             /**< NAS COUNT vector,
                                            index: 0 Uplink, 1 Downlink */
}NASHandler;

#endif  /* !_NAS_HANDLER_H */
