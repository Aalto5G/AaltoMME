/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**@file   commands.h
 * @author Vicent Ferrer
 * @date   March, 2013
 * @brief  Definition of command protocol
 * Definition of command protocol & Declaration of some server functions to recv commands
 * */

#ifndef COMM_HFILE
#define COMM_HFILE

#include <event2/event.h>
#include <glib.h>
#include "signals.h"
#include "commands_msg.h"

/*Public functions*/

/**@brief Initializes a UDP server
 * @param [in] servPort Server port number
 * @returns pointer to command server stack
 *
 * Internally can socket and bind system calls */
extern gpointer servcommand_init(gpointer mme, const int servPort);
extern gpointer servcommand_stop(gpointer serv_h);

/**@brief Accept callback used to read the commands
 *
 * LibEvent defined callback. Used to receive the command when the file descriptor is set as active */
//extern void cmd_accept(evutil_socket_t listener, short event, void *arg);

#endif /*COMM_HFILE*/
