/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
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

#define EMM_NUM_TIMERS (15)

typedef enum{
    TNULL,
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
    T3442
}EMM_TimerCode;

static const char* EMM_TimerStr[] = {
    "TNULL",
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
    "T3442"
};

static const guint EMM_rtx[] = {
    0, /* TNULL */
    5, /* T3402 */
    5, /* T3410 */
    5, /* T3411 */
    5, /* T3412 */
    5, /* T3416 */
    5, /* T3417 */
    5, /* T3417ext */
    5, /* T3418 */
    5, /* T3420 */
    5, /* T3421 */
    5, /* T3423 */
    5, /* T3430 */
    5, /* T3440 */
    5, /* T3442 */
};

static const struct timeval EMM_tv[] = {
    {0, 0},     /* TNULL */
    {12*60, 0}, /* T3402 */
    {15, 0},    /* T3410 */
    {10, 0},    /* T3411 */
    {54*60, 0}, /* T3412 */ /* UE only, provided by the network*/
    {30, 0},    /* T3416 */
    {5, 0},     /* T3417 */
    {10, 0},    /* T3417ext */
    {20, 0},    /* T3418 */
    {15, 0},    /* T3420 */
    {15, 0},    /* T3421 */
    {54*60, 0}, /* T3423 */ /* UE only, provided by the network*/
    {15, 0},    /* T3430 */
    {10, 0},    /* T3440 */
    {60*60, 0}, /* T3442 */ /* UE only, provided by the network*/
};

void emm_setTimer(EMMCtx_t *emm, EMM_TimerCode t, guint8 *msg, gsize len);

void emm_stopTimer(EMMCtx_t *emm, EMM_TimerCode t);

void emm_stopAllTimers(EMMCtx_t *emm);

#endif /* EMM_TIMERS_H */
