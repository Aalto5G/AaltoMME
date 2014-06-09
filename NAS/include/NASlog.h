/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   NASlog.h
 * @author Vicent Ferrer
 * @date   April, 2013
 * @brief  Error managment functions.
 *
 */

#ifndef NASLOG_H
#define NASLOG_H

/** Enum Used to classify the message level*/
enum nas_prioritylevel_c{
    NAS_EMERG,
    NAS_ALERT,
    NAS_CRIT,
    NAS_ERROR,
    NAS_WARN,
    NAS_NOTICE,
    NAS_INFO,
    NAS_DEBUG
};

#ifndef NAS_LOG_LVL
#define NAS_LOG_LVL 3
#endif /* NAS_LOG_LVL */


/** Use this macro to get the file, function and line values.*/
#define nas_msg(p, en, ...) nas_msg_(p, __FILE__, __func__, __LINE__, en, __VA_ARGS__)

/** Function Used to show the error message*/
void nas_msg_(enum nas_prioritylevel_c pri, const char *fn, const char *func, int ln, int en, char *fmt, ...);

#endif /* S1APLOG_H */
