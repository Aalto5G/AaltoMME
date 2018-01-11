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
 * @file   EMM_Timers.h
 * @Author Vicent Ferrer
 * @date   October, 2015
 * @brief  NAS EMM timer related functions and structs
 *
 */

#ifndef EMM_TIMERS_H
#define EMM_TIMERS_H

#include "EMMCtx.h"

#define EMM_NUM_TIMERS (22)

typedef enum{
    TNULL,
    /* UE side*/
    T3402,
    T3410,
    T3411,
    T3412,
    T3416,
    T3417,
    T3417ext,
    T3418,
    T3420,
    T3421,
    T3423,
    T3430,
    T3440,
    T3442,
    /*Network side*/
    T3413,
    T3422,
    T3450,
    T3460,
    T3470,
    TMOBILE_REACHABLE,
    TIMPLICIT_DETACH
}EMM_TimerCode;

static const char* EMM_TimerStr[] = {
    "TNULL",
    /* UE side*/
    "T3402",
    "T3410",
    "T3411",
    "T3412",
    "T3416",
    "T3417",
    "T3417ext",
    "T3418",
    "T3420",
    "T3421",
    "T3423",
    "T3430",
    "T3440",
    "T3442",
    /* Network side */
    "T3413",
    "T3422",
    "T3450",
    "T3460",
    "T3470",
    "TMOBILE_REACHABLE",
    "TIMPLICIT_DETACH",
};

static const guint EMM_rtx[] = {
    0, /* TNULL */
    /* UE side*/
    4, /* T3402 */
    4, /* T3410 */
    4, /* T3411 */
    4, /* T3412 */
    4, /* T3416 */
    4, /* T3417 */
    4, /* T3417ext */
    4, /* T3418 */
    4, /* T3420 */
    4, /* T3421 */
    4, /* T3423 */
    4, /* T3430 */
    4, /* T3440 */
    4, /* T3442 */
    /* Network side */
    4, /* T3413 */
    4, /* T3422 */
    4, /* T3450 */
    4, /* T3460 */
    4, /* T3470 */
    1, /* TMOBILE_REACHABLE */
    1, /* TIMPLICIT_DETACH */
};

static const struct timeval EMM_tv[] = {
    {0, 0},       /* TNULL */
    /* UE side*/
    {12*60, 0},   /* T3402 */
    {15, 0},      /* T3410 */
    {10, 0},      /* T3411 */
    {54*60, 0},   /* T3412 */ /* UE only, provided by the network*/
    {30, 0},      /* T3416 */
    {5, 0},       /* T3417 */
    {10, 0},      /* T3417ext */
    {20, 0},      /* T3418 */
    {15, 0},      /* T3420 */
    {15, 0},      /* T3421 */
    {54*60, 0},   /* T3423 */ /* UE only, provided by the network*/
    {15, 0},      /* T3430 */
    {10, 0},      /* T3440 */
    {60*60, 0},   /* T3442 */ /* UE only, provided by the network*/
    /* Network side */
    {60, 0},      /* T3413 */ /* Network dependent*/
    {6, 0},       /* T3422 */
    {6, 0},       /* T3450 */
    {6, 0},       /* T3460 */
    {6, 0},       /* T3470 */
    {54*60+4*60, 0}, /* TMOBILE_REACHABLE */ /* Network dependent 4min extra than T3412*/
    {1, 0},       /* TIMPLICIT_DETACH */  /* Network dependent ISR? T3423+4min, T3324? T3412+4min*/
};

void emm_setTimer(EMMCtx_t *emm, EMM_TimerCode t, guint8 *msg, gsize len);

void emm_stopTimer(EMMCtx_t *emm, EMM_TimerCode t);

void emm_stopAllTimers(EMMCtx_t *emm);

#endif /* EMM_TIMERS_H */
