/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/*****************************************************
Description: Header file of signals that are part of 
			the MME state machine
Author: Jose CR
Date: 8 Feb 2013
*****************************************************/


#ifndef _MMESIGNALS_H_
#define _MMESIGNALS_H_


enum t_signal_name{
    no_new_cmd      =   0,
	/*********************************************
	signals related to the engine
	*********************************************/
	engine_stop     =   1,
	engine_start    =   2,

    /**********************************************
    other signals related to S11 interface are here
    ***********************************************/

	S11_new_user_test, /*Test Signal*/
	S11_attach, /*4*/
	S11_ModifyBearer,
	S11_detach,
	S11_createIndirectDataForwardingTunnel,
    S11_handler_ready,
    S11_handler_error,
    S11_releaseAccessBearers,


	/**********************************************
	other signals related to S1 interface are here
	***********************************************/

    S1_Setup_Endpoint,
    S1_newUEmessage,/*12*/
    S1_ContinueAttach,/*13*/
    S1_detach,
	S1_handler_ready,/*15*/
	S1_UE_Context_Release,
	S1_HandoverPreparation,
	S1_HandoverCommand,
	S1_PathSwitchRequest,
	S1_PathSwitchACK,

    /**********************************************
    other signals related to NAS interface are here
    ***********************************************/
    NAS_data_available, /*21*/
    NAS_AttachReqCont, /*22*/
    NAS_AuthVectorAvailable,
    NAS_EMM_Continue,
    NAS_ESM_Continue,
    NAS_ContinueDettach,

    /**********************************************
    other signals related to S6a interface are here
    ***********************************************/
    S6a_getAuthVector,
    S6a_SynchAuthVector,

    /**********************************************
    other signals related to SDN Controller interface are here
    ***********************************************/
    Controller_new_user_attach,
    Controller_user_detach,
    Controller_user_handover,
    Controller_handler_ready,

	/**********************************************
	other signals related to other interface are here
	***********************************************/
	others,


};


#endif
