/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/*****************************************************
Description: Header file of MME
Author: Jose CR
Date: 8 Feb 2013
*****************************************************/


#ifndef _MME_engine_H_
#define _MME_engine_H_


#include "signals.h"
#include <stdbool.h>
#include <stdint.h>

#include "gtp.h"

#define MAXIMUM_PRIORITY 100
#define PRIORITY_LEVELS_NUM 2

struct t_signal;

typedef int (*engine_stateFunc)(struct t_signal *signal);


struct t_process{
    struct t_engine_data    *engine;
    engine_stateFunc		next_state;		/* state where the execution will continue*/
    struct t_process		*parent;		/* process that created this one */
    void				    *data;			/* data to be used by the process, usually session structure*/
    struct t_signal_queue	*firstSignal;	/* pointing to the first signal saved in the queue*/
    struct t_signal_queue	*lastSignal;	/* pointing to the last signal saved in the queue*/
    bool				    stop;
};


typedef struct t_signal{
    enum   t_signal_name	name;                   /* identifier of the signal */
    int				        priority;		        /* priority assigned to the signal*/
    struct t_process		*processTo;		        /* process receiving the signal*/
    struct t_process		*processFrom;   	    /* process sending the signal*/
    void				    *data;  		        /* data used by the signal */
    void                    (*freedataFunc)(void *);/* function to release the data in the signal*/
    struct event            *timerEv;               /* Event pointer for a timeout*/
}Signal;

/**************************************************
SIGNAL QUEUE
**************************************************/
struct t_signal_queue{
    struct t_signal     *signal;
    struct t_signal_queue   *next;
};

struct t_engine_data{
    struct mme_t            *mme;
    struct t_signal_queue   *sig_first;
    struct t_signal_queue   *sig_last[PRIORITY_LEVELS_NUM];
    struct t_signal_queue   sigdummy;
    int                     *mme_run;
    struct event_base       *evbase;                        /*< libevent base loop*/
};

/*
 *  freedatadummy - if one does not want to free signal data
 *  automatically, set the "freedatafn" to point to this function.
 */ 
extern void freedatadummy(void *data); /* save signal data for a while */

int engine_main();				/* initialize the state machine in the kernel*/

struct t_signal *new_signal(struct t_process *self);

void set_timeout(struct t_signal *signal, uint8_t sec, uint32_t usec);

void stop_timeout(struct t_signal *signal);

void save_signal(struct t_signal *signal);

void signal_send(struct t_signal *signal);

void engine_process_stop(struct t_process *process);

struct t_process *process_create(struct t_engine_data *self, engine_stateFunc state, void *data, struct t_process *parent);

void process_stop(struct t_process *process);

void run_parent(Signal *signal);

void sendFirstStoredSignal(struct t_process *process);


/*int engine_initialize();*/


#endif
