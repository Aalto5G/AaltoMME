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
