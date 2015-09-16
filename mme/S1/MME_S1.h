/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   MME_S1.h
 * @Author Vicent Ferrer
 * @date   May, 2013
 * @brief  MME S1 interface protocol state machine.
 *
 * This module implements the S1 interface state machine.
 */

#ifndef MME_S1_HFILE
#define MME_S1_HFILE

#include <netinet/sctp.h>
#include <glib.h>

#include "MME.h"
#include "S1AP.h"

#define S1AP_NONUESIGNALING_STREAM0 0


/* ======================================================================
 * S1 Type definitions
 * ====================================================================== */


typedef struct S1_EndPoint_Info_c{
    ENBname_t               *eNBname;
    Global_ENB_ID_t         *global_eNB_ID;
    SupportedTAs_t          *supportedTAs;
    CSG_IdList_t            *cSG_IdList;
    struct sctp_sndrcvinfo  sndrcvinfo;
}S1_EndPoint_Info_t;

/**@brief S1_EndPoint_Info_t destructor
 * @param [in] epInfo   Structure to be deallocated
 *
 * This function send the S1 message using the SCTP protocol
 * */
//void free_S1_EndPoint_Info(S1_EndPoint_Info_t* epInfo);


/* ======================================================================
 * S1 Tool API
 * ====================================================================== */

typedef gpointer S1;

/**@brief S1 Interface constructor
 * @param [in] mme pointer to MME structure
 * @returns interface handler
 * 
 * This function creates the interface S1, use s1_free to deallocate it.
 * */
gpointer s1_init(gpointer mme);

/**@brief S1 Interface destructor
 * @param [in] s1_h S1 interface handler
 * 
 * This function deallocates the interface S1 created using s1_init.
 * */
void s1_free(S1 s1_h);


/* ======================================================================
 * S1 MME State Machine API
 * ====================================================================== */


/**@brief S1 Setup Procedure function
 * @param [in] engine   Engine reference
 * @param [in] owner    parent process
 * @param [in] ep       Endpoint information of the eNB to be setup
 *
 * This function Allocates a session and a process for the S1 Setup Procedure
 * */
struct t_process *S1_Setup(struct t_engine_data *self, struct t_process *owner, struct EndpointStruct_t *ep_S1);
#endif /* MME_S1_HFILE */
