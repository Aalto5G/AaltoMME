/* AaltoMME - Mobility Management Entity for LTE networks
 * Copyright (C) 2013 Vicent Ferrer Guash & Jesus Llorente Santos
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
