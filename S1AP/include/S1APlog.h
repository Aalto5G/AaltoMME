/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S1APlog.h
 * @author Vicent Ferrer
 * @date   April, 2013
 * @brief  Error managment functions.
 *
 */

#ifndef S1APLOG_H
#define S1APLOG_H

/** Enum Used to classify the message level*/
enum s1ap_prioritylevel_c{
    EMERG,
    ALERT,
    CRIT,
    ERROR,
    WARN,
    NOTICE,
    INFO,
    DEB
};

/** Use this macro to get the file, function and line values.*/
#define s1ap_msg(p, en, ...) s1ap_msg_(p, __FILE__, __func__, __LINE__, en, __VA_ARGS__)

/** Function Used to show the error message*/
void s1ap_msg_(enum s1ap_prioritylevel_c pri, const char *fn, const char *func, int ln, int en, char *fmt, ...);

#endif /* S1APLOG_H */
