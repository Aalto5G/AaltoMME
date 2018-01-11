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
 * @file   S6a.h
 * @Author Vicent Ferrer
 * @date   June, 2013
 * @brief  S6a definitions.
 *
 * This include should be replaced with the proper S6a diameter coder/encoder
 */

#ifndef S6A_H_
#define S6A_H_

typedef struct SecurityCtx_c{
    /*Auth Vector*/
    uint8_t eKSI;
    uint8_t rAND[16];
    uint8_t xRES[8];
    uint8_t kASME[32];
    uint8_t aUTN[16];

    /*Derived parameters*/
    uint8_t keNB[32];
    uint8_t nh[32];

    /*Counters*/
    uint32_t ulNAScnt;
    uint32_t dlNAScnt;
    uint8_t ncc; /*< Next Hop Chaining Count*/
}SecurityCtx_t;


#endif /* S6A_H_ */
