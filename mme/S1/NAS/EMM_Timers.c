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
 * @file   EMM_Timers.c
 * @Author Vicent Ferrer
 * @date   October, 2015
 * @brief  NAS EMM timer related function implementations
 *
 */

#include "EMM_Timers.h"
#include "timermgr.h"
#include "logmgr.h"

typedef struct{
    EMMCtx_t      *emm;
    EMM_TimerCode code;
    GByteArray    *msg;
}EMM_Timer;


static void emm_timer_free_cb(Timer tm_t, EMM_Timer *t){
    t->emm->activeTimers[t->code] = NULL;
    g_byte_array_free(t->msg, TRUE);
    g_free(t);
}


static void emm_timer_cb(Timer tm_t, EMM_Timer *t){
    t->emm->state->processTimeout(t->emm,
                                  t->msg->data, t->msg->len,
                                  t->code);
}


static void emm_timer_max_cb(Timer tm_t, EMM_Timer *t){
    t->emm->state->processTimeoutMax(t->emm,
                                     t->msg->data, t->msg->len,
                                     t->code);
}


void emm_setTimer(EMMCtx_t *emm, EMM_TimerCode c, guint8 *msg, gsize len){
    EMM_Timer *t;
    Timer tm_t;

    emm_log(emm, LOG_DEBUG, 0,"Setting EMM timer: %s",
            EMM_TimerStr[c]);

    t = g_new0(EMM_Timer, 1);
    t->emm = emm;
    t->code = c;
    t->msg = g_byte_array_new();
    g_byte_array_append(t->msg, msg, len);

    tm_t = tm_add_timer(emm->tm, &(EMM_tv[t->code]), EMM_rtx[t->code],
                        (Timer_cb)emm_timer_cb,
                        (Timer_cb)emm_timer_max_cb,
                        (Timer_cb)emm_timer_free_cb,
                        (void*)t);
    if(!tm_t){
        g_error("Failed to create timer.");
    }
    if(emm->activeTimers[t->code] != NULL){
        emm_log(emm, LOG_WARNING, 0,"Failed to add Timer %s: Already exists. Replacing",
                EMM_TimerStr[t->code]);
        tm_stop_timer(emm->activeTimers[t->code]);
    }
    emm->activeTimers[t->code] = tm_t;
}

void emm_stopTimer(EMMCtx_t *emm, EMM_TimerCode c){
    Timer tm_t = emm->activeTimers[c];

    if(!tm_t){
        emm_log(emm, LOG_DEBUG, 0,"Failed to Stop Timer %s: not found",
                EMM_TimerStr[c]);
        return;
    }
    tm_stop_timer(tm_t);
    emm->activeTimers[c] = NULL;
}

void emm_stopAllTimers(EMMCtx_t *emm){
    Timer tm_t;
    guint i;

    for(i=0; i< EMM_NUM_TIMERS ; i++){
        tm_t = emm->activeTimers[i];
        if(tm_t){
            tm_stop_timer(tm_t);
            emm->activeTimers[i] = NULL;
        }
    }
}
