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

// MME_test.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <event2/event.h>

#include "signals.h"
#include "logmgr.h"
#include "commands.h"
#include "storagesys.h"
#include "MME_engine.h"
#include "MME.h"
#include "MME_S1.h"
#include "MME_S11.h"
#include "nodemgr.h"


#define FALSE (0)
#define TRUE (!FALSE)


void freedatadummy(void *data){}

/**************************************************
Struct to store command callback arguments
**************************************************/
/*struct comm_cb_arg{
  struct t_signal       *signal;
  struct event_base     *evbase;
};*/


/***********************************************/

static void engine_run(struct t_engine_data *self);
static void engine_signal_free(struct t_signal *signal);
static void process_run(struct t_signal *signal);



struct t_signal *new_signal(struct t_process *self){

    struct t_signal *output = NULL;
    output = (struct t_signal *)calloc(1, sizeof(struct t_signal));

    output->processTo = self;
    output->processFrom = self;
    output->freedataFunc = NULL;
    output->data = NULL;

    return output;

}

/****************************************************/
void signal_send(struct t_signal *signal){

    int		i=0;
    struct t_signal_queue	*newItem = NULL;
    struct t_signal_queue	*currentItem = NULL;
    int		slot = 0;
    struct t_engine_data *self = signal->processTo->engine;

    if(signal){
        log_msg(LOG_DEBUG, 0, "New signal send to engine: %d", signal->name);
        newItem = (struct t_signal_queue *)malloc(sizeof(struct t_signal_queue));

        if(!newItem){
            engine_signal_free(signal);
            return;
        }

        newItem->signal = signal;
        slot = ((PRIORITY_LEVELS_NUM * signal->priority) / MAXIMUM_PRIORITY);
        if (slot >= PRIORITY_LEVELS_NUM)
            slot = PRIORITY_LEVELS_NUM - 1;

        /* insert the item in the right priority on the queue */
        newItem->next = self->sig_last[slot]->next;
        self->sig_last[slot]->next = newItem;
        currentItem = self->sig_last[slot];

        /* reorganize the signals in the queue based on priorities*/
        for(i=0; i<=slot; i++){
            if(currentItem == self->sig_last[i]){
                self->sig_last[i] = newItem;
            }
        }
    }
}

/***********************************************
TIMEOUT FUNCTIONS
***********************************************/
static void cb_timer(evutil_socket_t fd, short what, void *arg)
{
    struct t_signal *signal = (struct t_signal *)arg;
    signal_send(signal);
    event_del(signal->timerEv);
    signal->timerEv = NULL;
}

void set_timeout(struct t_signal *signal, uint8_t sec, uint32_t usec){

    struct timeval time;
    time.tv_sec = sec;
    time.tv_usec = usec;
    struct event *ev;
    /* We're going to set up a repeating timer to get called called 100
       times. */
    ev = event_new(signal->processTo->engine->evbase, -1, 0, cb_timer, signal);
    signal->timerEv = ev;
    event_add(ev, &time);
}

void stop_timeout(struct t_signal *signal){
    event_del(signal->timerEv);
    signal->timerEv = NULL;
}

/****************************************************/

void save_signal(struct t_signal *signal){
    struct t_signal_queue   *queueItem = NULL;
    struct t_signal_queue   *queueItem2 = NULL;
    struct t_process        *process = signal->processTo;
    log_msg(LOG_DEBUG, 0, "ENTER");

    if(process==NULL){
        log_msg(LOG_ERR, 0, "No process to store the signal");
        return;
    }

    queueItem = (struct t_signal_queue *)malloc(sizeof(struct t_signal_queue));
    if(!queueItem){
        engine_signal_free(signal);
        return;
    }
    queueItem->signal = signal;
    queueItem->next = NULL;

    if(process->lastSignal)
        process->lastSignal->next = queueItem;

    process->lastSignal = queueItem;
    if(!process->firstSignal){
        process->firstSignal = queueItem;
    }
    log_msg(LOG_DEBUG, 0, "Stored signal %d on process %p", signal->name, process);
}

/********************************************************/
static void engine_signal_free(struct t_signal *signal){
    if(!signal)
        return;

    if(signal->data){
        if(signal->freedataFunc){
            signal->freedataFunc(signal->data);
            signal->data=NULL;
        }
    }
}


/********************************************************/
void engine_process_stop(struct t_process *process){

    struct t_signal_queue	*queueItem = NULL;
    struct t_signal_queue	*queueItem2 = NULL;

    if(!process)
        return;

    queueItem = process->firstSignal;
    while(queueItem){
        engine_signal_free(queueItem->signal);
        queueItem2 = queueItem;
        queueItem = queueItem->next;
        free(queueItem2);
        queueItem2 = NULL;
    }
    free(queueItem);
    queueItem = NULL;
    free(process);
    process = NULL;
}


/********************************************************/
static void process_run(struct t_signal *signal){
    int save = FALSE;		/* signal to be saved */
    engine_stateFunc		state = NULL;
    struct t_process		*process = NULL;
    struct t_signal_queue   *queueItem = NULL;
    struct t_signal_queue   *queueItem2 = NULL;


    if(!signal){
            log_msg(LOG_WARNING, 0, "No signal available to process.");
        return;
    }

    log_msg(LOG_DEBUG, 0, "Processing signal %d", signal->name);

    if(!signal->processTo){
            log_msg(LOG_WARNING, 0, "The signal has not any valid processTo field");
        return;
    }

    process = signal->processTo;

    if(process->stop){
        if(signal->name == engine_stop){
            engine_process_stop(process);
        }
        engine_signal_free(signal);
        free(process);
        return;
    }

    state = process->next_state;
    save = state(signal);
    if(save==1){
        save_signal(signal);
        return;
    } /* end of saved loop*/
    engine_signal_free(signal); /*the signal is not saved so it is
                                  free*/
    if(process->next_state != state && process->firstSignal){
        queueItem = process->firstSignal;
        while(queueItem){
            signal_send(queueItem->signal);
            queueItem2 = queueItem;
            queueItem = queueItem->next;
            free(queueItem2);
            queueItem2 = NULL;
        }
        process->firstSignal = NULL;
        process->lastSignal = NULL;
    }
}


/***************************************************************/

struct t_process *process_create(struct t_engine_data *self, engine_stateFunc state, void *data, struct t_process *parent){

    struct t_process	*newProcess = NULL;
    newProcess = (struct t_process *)calloc(1, sizeof(struct t_process));
    if(!newProcess)
        return NULL;

    newProcess->data = data;
    newProcess->next_state = state;
    newProcess->parent = parent;
    newProcess->firstSignal = NULL;
    newProcess->lastSignal = NULL;
    newProcess->engine = self;

    return newProcess;
}


/*****************************************************************/

void process_stop(struct t_process *self){
    struct t_signal *tempsignal = NULL;
    struct t_signal_queue   *queueItem = NULL;
    struct t_signal_queue   *queueItem2 = NULL;

    if(!self)
        return;

    self->stop = TRUE;
    queueItem = self->firstSignal;
    while(queueItem){
        queueItem2 = queueItem;
        queueItem = queueItem->next;
        free(queueItem2);
        queueItem2 = NULL;
    }
    self->firstSignal = NULL;
    self->lastSignal = NULL;

    signal_send(new_signal(self));

}



void sendFirstStoredSignal(struct t_process *self){
    struct t_signal *tempsignal = NULL;
    struct t_signal_queue   *queueItem = NULL;
    struct t_signal_queue   *queueItem2 = NULL;

    if(!self)
        return;

    /*Store first signal*/
    queueItem = self->firstSignal;

    if(queueItem==NULL){
        return;
    }

    log_msg(LOG_DEBUG, 0, "Recovered signal %u, from process %#x", queueItem->signal->name, self);

    /*Remove from queue*/
    self->firstSignal = queueItem->next;

    if(self->lastSignal==queueItem)
        self->lastSignal=NULL;

    signal_send(queueItem->signal);
    free(queueItem);
    queueItem = NULL;
}


void run_parent(Signal *signal){
    struct t_process *old;
    Signal *output;

    if(PROC->parent !=NULL && PROC->parent != PDATA->sessionHandler){
        log_msg(LOG_DEBUG, 0, "Recover parent %x of %x on session %x", PROC->parent, PDATA->sessionHandler, PDATA);
        /*Recover old process*/
        old = PDATA->sessionHandler;
        PDATA->sessionHandler = PROC->parent;

        process_stop(old);
        /*Recover old signal*/
        sendFirstStoredSignal(PDATA->sessionHandler);
    }else{
        log_msg(LOG_WARNING, 0, "Couldn't recover parent process, possible memory leak and break of expected workflow");
    }
    /*process_stop(SELF);*/
}


/***executes one transition in a process*************************/
static void engine_run(struct t_engine_data *self){
    int	i = 0;
    struct	t_signal_queue	*tmpItem  = NULL;
    struct	t_signal	    *signal   = NULL;

    while(TRUE){
        tmpItem = self->sig_first->next;

        if(tmpItem)
            signal = tmpItem->signal;
        else
            return;

        self->sig_first->next = tmpItem->next;

        for(i=0; i<PRIORITY_LEVELS_NUM; i++){
            if(tmpItem == self->sig_last[i])
                self->sig_last[i] = self->sig_first;
                /*****last signal was removed from the queue***/
        }

        free(tmpItem);
        tmpItem = NULL;
        process_run(signal);

    }
}


/*****************************************************************/

static void engine_loop(struct t_engine_data *self){

    int i;
    Signal CommandSignal;
    struct nodeinfo_t node;

    struct  SessionStruct_t *testsession;
    struct user_ctx_t *user;

    /*=================================================
    For testing we preassign the signal name to start S1 */
    /*CommandSignal.name=S11_new_user_test;*/
    CommandSignal.name=no_new_cmd;
    CommandSignal.data = NULL;

    testsession = (struct SessionStruct_t *) malloc(sizeof(struct SessionStruct_t));
    memset(testsession, 0, sizeof(struct SessionStruct_t));
    testsession->user_ctx = (struct user_ctx_t *) malloc(sizeof(struct user_ctx_t));
    user = testsession->user_ctx;
    user->imsi = 123454678901236ULL;

    /*For testing we preassign the imsi to 1 */

    while(*(self->mme_run)){

        engine_run(self);

        /*Event base loop, because of EVLOOP_NONBLOCK flag,
         * it only checks if there is any active file descriptor. If so, run the callbacks*/
        event_base_loop(self->evbase, EVLOOP_ONCE );//| EVLOOP_NONBLOCK);
        /*****wait for messages from application and then run specific function associated to the message****/
        switch(CommandSignal.name){
            case engine_start:
                log_msg(LOG_DEBUG, 0, "Signal command engine_start received");
                CommandSignal.name=no_new_cmd;
                break;
            case engine_stop:
                log_msg(LOG_DEBUG, 0, "Signal command engine_stop received");
                self->mme_run=FALSE;
                CommandSignal.name=no_new_cmd;
                break;

            case S11_new_user_test:
                log_msg(LOG_DEBUG, 0, "Signal command S11_new_user_test, \nSimulating the reception of S1AP Initial UE Message.");
                /*Create session request*/
                if(CommandSignal.data!=NULL){
                    testsession = (struct SessionStruct_t *) malloc(sizeof(struct SessionStruct_t));
                    memset(testsession, 0, sizeof(struct SessionStruct_t));
                    testsession->user_ctx = (struct user_ctx_t *) malloc(sizeof(struct user_ctx_t));
                    memset(testsession->user_ctx, 0, sizeof(struct user_ctx_t));
                    memcpy(&(testsession->user_ctx->imsi), CommandSignal.data, sizeof(uint64_t));
                    testsession->user_ctx->S11MMETeid = newTeid();
                    testsession->user_ctx->s11.teid = 0;
                }
                //testsession->s11 = &(mme->s11);  /*Change endpoint structure before entering to S11 state machine*/
                S11_newUserAttach(self, testsession);
                CommandSignal.name=no_new_cmd;
                break;

            default:
                break;
        }
    }
    free(user);
    free(testsession);

}

/***************************************************************/
int engine_main(struct mme_t *mme){
    int i=0;
    struct t_engine_data engine;

    engine.mme = mme;
    mme->engine = &engine;
    engine.mme_run = mme->run;

    engine.sigdummy.signal = NULL;
    engine.sigdummy.next = NULL;
    engine.sig_first = &engine.sigdummy;
    engine.evbase = mme->evbase;

    for(i=0; i< PRIORITY_LEVELS_NUM; i++){
        engine.sig_last[i] = &engine.sigdummy;
    }

    engine_loop(&engine);

    if(engine.sigdummy.signal){
        free(engine.sigdummy.signal->data);
        engine.sigdummy.signal->data = NULL;
    }

    return 0;
}



