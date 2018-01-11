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
