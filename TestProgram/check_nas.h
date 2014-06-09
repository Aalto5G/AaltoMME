/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**@file   check_libgtp.h
 * @author Vicent Ferrer
 * @date   March, 2013
 * @brief  Definition of Check tests for libgtp
 *
 * Definition and implementation of test cases using check testing framework */

#include <check.h>

/* event_get_version function*/
#include <event2/event.h>

Suite *nas_suite (void);
