/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   NASlog.c
 * @author Vicent Ferrer
 * @date   April, 2013
 * @brief  Error managment functions.
 *
 */

#include "NASlog.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

/** Max size allowed on the argument string*/
#define NASLOG_MSGSIZE 256

/** Log Level dictionary to convert the priority to a string*/
static char* logLevelStr[] = {"EMERG", "ALERT", "CRIT ", "ERROR", "WARN ", "NOTICE", "INFO ", "DEBUG"};

void nas_msg_(enum nas_prioritylevel_c pri, const char *fn, const char *func, int ln, int en, char *fmt, ...){
    va_list args;
    char buf[NASLOG_MSGSIZE];

    char timeStr[128];
    struct timeval tv;
    struct tm *pTm;
    if(pri<NAS_LOG_LVL){
        gettimeofday( &tv, NULL );
        pTm = localtime ( (const time_t*)&tv.tv_sec );
        strftime ((char*) timeStr, 128, "%d/%m/%Y %T", pTm );
        sprintf((char*)(timeStr + strlen((char*)timeStr)), ".%06u", (unsigned int)tv.tv_usec);

        va_start(args, fmt);
        vsnprintf(buf, NASLOG_MSGSIZE, fmt, args);
        va_end(args);
        buf[NASLOG_MSGSIZE-1] = 0; /* Make sure it is null terminated */
        if (en)
          printf("[%s]%s : %d (%s) %s <%s, %s, %d>\n", timeStr, logLevelStr[pri], en, strerror(en), buf, func, fn, ln);
        else
          printf("[%s]%s : %s <%s(), %s, %d>\n", timeStr, logLevelStr[pri], buf, func, fn, ln);
    }
}
