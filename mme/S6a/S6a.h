/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S6a.h
 * @Author Vicent Ferrer
 * @date   June, 2013
 * @brief  S6a definitions.
 *
 * This include should be replaced with the proper S6a diameter coder/encoder
 */

#ifndef S6A_H_
#define S6A_H_

typedef struct SecurityCtx_c{
    /*Auth Vector*/
    uint8_t eKSI;
    uint8_t rAND[16];
    uint8_t xRES[8];
    uint8_t kASME[32];
    uint8_t aUTN[16];

    /*Derived parameters*/
    uint8_t keNB[32];
    uint8_t nh[32];

    /*Counters*/
    uint32_t ulNAScnt;
    uint32_t dlNAScnt;
    uint8_t ncc; /*< Next Hop Chaining Count*/
}SecurityCtx_t;


#endif /* S6A_H_ */
