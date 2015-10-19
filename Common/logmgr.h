/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   logmgr.h
 * @author Vicent Ferrer
 * @date   March, 2013
 * @brief  Error managment functions.
 *
 * Based on Syslog functions.(2003, 2004 Mondru AB.)
 */

#ifndef _SYSERR_H
#define _SYSERR_H

#include <syslog.h>
#include <errno.h>
#include <netinet/in.h>


#define SYSERR_MSGSIZE 512
#define log_msg(p, en, ...) log_msg_(p, __FILE__, __func__, __LINE__, en, __VA_ARGS__)
#define log_errpack(p, en, ...) log_errpack_(p, __FILE__, __func__, __LINE__, en, __VA_ARGS__)


/**
 * @brief Function used to initialize the log_entity
 * @param [in] app Application name to be shown on logs
 * @param [in] priority priority level acording to syslog levels
 *
 *  level : This determines the importance of the message. The levels are, in order of decreasing importance:
 *  LOG_EMERG   system is unusable
 *  LOG_ALERT   action must be taken immediately
 *  LOG_CRIT    critical conditions
 *  LOG_ERR     error conditions
 *  LOG_WARNING warning conditions
 *  LOG_NOTICE  normal, but significant, condition
 *  LOG_INFO    informational message
 *  LOG_DEBUG   debug-level message
 */
extern void init_logger(const char * app, int priority);


/**
 * @brief Function to close the log_entity
 *
 * Currently is an optional function, but is recommended to use it to avoid
 * compatibility issues in future versions
 */
extern void close_logger();


/**
 * @brief Change the level of the messages
 * @param [in] priority priority level acording to syslog levels
 */
extern void change_logger_lvl(int priority);


/**
 * @brief send log message
 * @param [in] priority priority level acording to syslog levels
 * @param [in] filename
 * @param [in] line
 * @param [in] errno error number
 * @param [in] msg Message to be printed
 *
 * Used by other libraries.
 */
extern void log_msg_s(int pri, char *fn, const char *func, int ln, int en, const char *msg);


/**
 * @brief send log message
 * @param [in] priority priority level acording to syslog levels
 * @param [in] filename
 * @param [in] line
 * @param [in] errno error number
 * @param [in] fmt format, printf equivalent
 *
 * Use the macro log_msg that fills the filename, the function
 * and the line of code.
 */
extern void log_msg_(int pri, char *fn, const char *func, int ln,
                     int en, char *fmt, ...);


/**
 * @brief send log message
 * @param [in] priority priority level acording to syslog levels
 * @param [in] filename
 * @param [in] line
 * @param [in] errno error number, use 0 by default.
 * @param [in] peer source address
 * @param [in] pack packet buffer
 * @param [in] len packet length
 * @param [in] fmt format, printf equivalent
 *
 * Use the macro log_errpack that fills the filename, the function
 * and the line of code.
 */
extern void log_errpack_(int pri, char *fn, const char *func, int ln,
                         int en, struct sockaddr_in *peer,
                         void *pack, unsigned len, char *fmt, ...);

/**
 * @brief simple function to print a buffer
 * @param [in] buf buffer pointer
 * @param [in] size size of the buffer
 */
extern void printfbuffer(uint8_t *buf, uint32_t size);

#endif	/* !_SYSERR_H */
