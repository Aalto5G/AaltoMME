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
 * @file   eia2.h
 * @Author Vicent Ferrer
 * @date   September, 2015
 * @brief  EPS Integrity Algorithm 2
 *
 * EPS Integrity Algorithm 2: CMAC, AES 128 CBC mode
 */

#ifndef EIA2_H
#define EIA2_H

#include <stdlib.h>
#include <stdint.h>

void eia2(const void *k,
          const void *count, const uint8_t bearer, const uint8_t direction,
          const void* msg, const size_t mLen,
          void *digest);


#endif /* EIA2_H */
