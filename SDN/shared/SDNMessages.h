/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   SDNMessages.h
 * @Author Robin Babujee Jerome
 * @date   December, 2013
 * @brief  SDN messages
 *
 * The structures are a collection of pointers to the data.
 */

#ifndef _SDN_MESSAGES_H
#define _SDN_MESSAGES_H


/* ======================================================================
 * Structure for message sent between MME & SDN Controller
 * ====================================================================== */

typedef struct ip_addr{
	uint8_t     ipv4    :1; /*  1.......    IPv6 flag*/
	uint8_t     ipv6    :1; /*  .1......    IPv6 flag*/
	uint8_t     spare   :6; /*  Will be filled with zeroes */
	union {
		uint32_t    addr_v4;
		uint8_t     addr_v6[16];
		struct{
			uint8_t  ipv6[16];
			uint32_t ipv4;
		}both;

	}ipAddr;
}__attribute__((packed)) ip_addr;

typedef struct sdn_packet{
	uint8_t operation    :5;
	uint8_t version      :3;
	uint64_t msisdn;
	uint32_t uL_gtp_teid;
	uint32_t dL_gtp_teid;
	struct ip_addr UE;
	struct ip_addr eNB;
	struct ip_addr SGW;
}__attribute__((packed)) sdn_msg;


#endif  /* !_SDN_MESSAGES_H */
