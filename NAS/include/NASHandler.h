/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   NAS.h
 * @Author Vicent Ferrer
 * @date   September, 2015
 * @brief  Internal NAS handler
 */

#ifndef _NAS_HANDLER_H
#define _NAS_HANDLER_H

typedef struct{
    NAS_EIA  i;                        /**< NAS Integrity Algorithm*/
    uint8_t  ikey[16];                 /**< NAS Integrity Key*/
    NAS_EEA  e;                        /**< NAS Encryption Algorithm */
    uint8_t  ekey[16];                 /**< NAS Encryption Key*/
    uint32_t nas_count[2];             /**< NAS COUNT vector,
                                            index: 0 Uplink, 1 Downlink */
    void     (*countOverflow)(void*);  /**< Callback to be called in the
                                            event of the NAS Count Overflow */
    void     *udata;                   /**< User data to be sent to the
                                            previous callback */
}NASHandler;

#endif  /* !_NAS_HANDLER_H */
