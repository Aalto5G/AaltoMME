/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**@file   commands_msg.h
 * @author Vicent Ferrer
 * @date   June, 2015
 * @brief  Definition of command messages
 * Definition of command messages
 * */

#ifndef COMM_MSG_HFILE
#define COMM_MSG_HFILE

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
        debug_level,                /**<The command is a debug level*/
};

#endif /*COMM_MSG_HFILE*/
