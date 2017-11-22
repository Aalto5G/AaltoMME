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
 * @file   S1Assoc_Active.h
 * @Author Vicent Ferrer
 * @date   June, 2015
 * @brief  S1Assoc State
 *
 */

#ifndef S1ASSOC_ACTIVE_HFILE
#define S1ASSOC_ACTIVE_HFILE

#include "S1Assoc_State.h"

typedef struct{
    S1STATE;
}S1Assoc_Active;

void linkS1AssocActive(S1Assoc_State* s);

#endif /* S1ASSOC_ACTIVE_HFILE */
