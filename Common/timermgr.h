/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   timermgr.h
 * @author Vicent Ferrer
 * @date   October, 2015
 * @brief  Timer Manager functions.
 *
 */

#ifndef _TIMERMGR_H
#define _TIMERMGR_H

#include <event2/event.h>
#include <stdint.h>

/**
 * @typedef Timer manager handler*/
typedef void* TimerMgr;

/**
 * @typedef Timer handler*/
typedef void* Timer;

/**
 * @typedef Timer callback */
typedef void (*Timer_cb)(Timer, void*);


/**
 * @brief  Create new Timer Manager
 * @param [in] ev_base event base
 * @return Timer Manager handler
 *
 * This functions is used to create a timer manager. The returned pointer
 * has to be freed using the function free_timerMgr
 */
extern TimerMgr init_timerMgr(struct event_base *ev_base);


/**
 * @brief  Delete Timer Manager
 * @param [in] h Timer Manager handler
 *
 * This functions is used to free the timer manager handler.
 */
extern void free_timerMgr(TimerMgr h);


/**
 * @brief  Starts a new timer
 * @param [in]  h        Timer manager handler
 * @param [in]  tv       Time until timer expiration
 * @param [in]  max_rtx  Maximum number of retransmitions
 * @param [in]  cb_to    Callback to be run after timer expiration
 * @param [in]  cb_maxTo Callback to be run after calling cb_to max_rtx times
 * @param [in]  cb_free  Callback to free the cb_args
 * @param [in]  cb_args  Callback arguments
 * @return Timer handler
 *
 * Starts a new timer.
 * The parameters cb_maxTO and cb_free can be NULL. The cb_to cannot be NULL.
 * The max_rtx must be greater than 0.
 */
extern Timer tm_add_timer(TimerMgr h,
                          const struct timeval *tv, const uint32_t max_rtx,
                          Timer_cb cb_to, Timer_cb cb_maxTo, Timer_cb cb_free,
                          void* cb_args);


/**
 * @brief Timer stop
 * @param [in]  t        Timer handler
 *
 * Stop the timer. If the cb_free is not empty, it will call it to free the
 * cb_args
 */
extern void tm_stop_timer(Timer t);


#endif /* !_TIMERMGR_H */
