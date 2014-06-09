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
#include "signals.h"

/** Server port definition*/
#define COMMAND_PORT 5000

/** Command Structure, the data is organized as a TLV */
struct comm_tlv {
  uint8_t t;                    /**< Type */
  uint8_t l;                    /**< Length : Total length of the packet, not only the value.*/
  uint8_t v[254];               /**< Value */
  uint64_t imsi;
} __attribute__((packed));


/** Command type enum*/
enum t_comm_name{
        engine_signal = 0,          /**<The command is a signal*/
        engine_signal_with_args,    /**<The command is a signal with arguments*/
        debug_level,                /**<The command is a debug level*/
};

/*Public functions*/

/**@brief Initializes a UDP server
 * @param [in] servPort Server port number
 * @returns socket file descriptor
 *
 * Internally can socket and bind system calls */
extern int servcommand_init(const int servPort);

/**@brief Accept callback used to read the commands
 *
 * LibEvent defined callback. Used to receive the command when the file descriptor is set as active */
extern void cmd_accept(evutil_socket_t listener, short event, void *arg);

#endif /*COMM_HFILE*/
