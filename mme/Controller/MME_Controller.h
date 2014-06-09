/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and funded by EIT ICT labs.
 */

/**
 * @file   MME_Controller.h
 * @Author Robin Babujee Jerome
 * @date November 2013
 * @brief MME Controller interface protocol state machine.
 */

#ifndef MME_CONTROLLER_HFILE
#define MME_CONTROLLER_HFILE

#define CONTROLLER_PORT 12345

#include "MME.h"


/* ======================================================================
 * MME Controller State Machine API
 * ====================================================================== */

/**@brief Attach new user
 * @param [in]  engine Engine reference
 * @param [in]  session User session structure
 *
 * Used to pass the session to the Controller State machine. The previous state machine shall include the signal to return.
 */
void Controller_newAttach(struct t_engine_data *engine, struct SessionStruct_t *session);

/**@brief Detach UE
 * @param [in]  engine Engine reference
 * @param [in]  session User session structure
 *
 * Used to pass the session to the Controller State machine. The previous state machine shall include the signal to return.
 */
void Controller_newDetach(struct t_engine_data *engine, struct SessionStruct_t *session);

/**@brief Handover UE from one src eNB to dest eNB
 * @param [in]  engine Engine reference
 * @param [in]  session User session structure
 *
 * Used to pass the session to the Controller State machine. The previous state machine shall include the signal to return.
 */
void Controller_newHandover(struct t_engine_data *engine, struct SessionStruct_t *session);

/**@brief Receive a packet from the SDN Controller
 * @param [in]  socket fd
 * @param [in]  sdn_packet pointer
 * @param [in]  length pointer
 * @param [in]  EndpointStruct for peer
 * @param [in]  length of EndpointStruct
 *
 * Used to receive a packet from the SDN Controller.
 */
int ctrlp_recv(int sockfd, struct sdn_packet *packet, size_t *len, struct sockaddr_in *peer, socklen_t *peerlen);

/**@brief Create Controller process structure
 * @param [in]  engine Engine reference
 * @param [in]  owner Parent process.
 *
 * Used to start a state machine once a packet is received from the controller.
 */
struct t_process *Controller_handler_create(struct t_engine_data *engine, struct t_process *owner);

#endif /* MME_CONTROLLER_HFILE */
