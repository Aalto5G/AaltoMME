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

#define COMMAND_PORT 5000

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
