/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and funded by EIT ICT labs.
 */

/**
 * @file   MME_Controller.h
 * @Author Vicent Ferrer Guasch
 * @date July 2015
 * @brief MME Controller interface protocol state machine.
 */

#ifndef MME_CONTROLLER_HFILE
#define MME_CONTROLLER_HFILE

#include "MME.h"


/* ======================================================================
 * MME Controller State Machine API
 * ====================================================================== */

/**@brief Create SDN interface
 * @param [in]  mme reference to the mme
 * @return the reference to the sdn ctrl interface
 *
 * The returned reference will be used in the calls to this API. Remember to
 * free the reference after use with sdnCtrl_free
 */
gpointer sdnCtrl_init(gpointer mme);


/**@brief Delete SDN interface
 * @param [in]  ctrl_h reference to the interface
 *
 * Function to delete the ctrl interface structure
 */
void sdnCtrl_free(gpointer ctrl_h);


/**@brief Attach new user event
 * @param [in]  ctrl pointer to the SDN controller interface
 * @param [in]  user User information
 *
 * Used to pass the session to the Controller State machine.
 */
void Controller_newAttach(gpointer ctrl, gpointer emm);

/**@brief Detach UE event
 * @param [in]  ctrl pointer to the SDN controller interface
 * @param [in]  user User information
 *
 * Used to pass the session to the Controller State machine.
 */
void Controller_newDetach(gpointer ctrl, gpointer emm);

/**@brief Handover UE from one src eNB to dest eNB event
 * @param [in]  ctrl pointer to the SDN controller interface
 * @param [in]  user User information
 *
 * Used to pass the session to the Controller State machine.
 */
void Controller_newHandover(gpointer ctrl, gpointer emm);

#endif /* MME_CONTROLLER_HFILE */
