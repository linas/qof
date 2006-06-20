/********************************************************************
 *       qofdate-p.h - QofDate private header.
 *
 *  Tue Jun 13 16:19:13 2006
 *  Copyright (C) 2006 Free Software Foundation, Inc.
 ********************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
 
#ifndef _QOFDATE_P_H
#define _QOFDATE_P_H

#define ENUM_ERR_LIST(_)	\
	_(ERR_NO_ERROR, = 0)	\
	_(ERR_WEEKDAY_NAME,)	\
	_(ERR_MONTH_NAME,)		\
	_(ERR_LOCALE_DATE_TIME,) \
	_(ERR_STANDARD_DAY,)	\
	_(ERR_RECURSIVE_F,)		\
	_(ERR_LOCALE_AMPM,)		\
	_(ERR_TIME_AMPM,)		\
	_(ERR_RECURSIVE_R,)		\
	_(ERR_SECS_NO_DIGITS,)	\
	_(ERR_RECURSIVE_T,)		\
	_(ERR_G_INCOMPLETE,)	\
	_(ERR_INVALID_Z,)		\
	_(ERR_YEAR_DIGITS,)		\
	_(ERR_MIN_TO_DECIMAL,)	\
	_(ERR_GMTOFF,)			\
	_(ERR_INVALID_FORMAT,)	\
	_(ERR_OUT_OF_RANGE,)	\
	_(ERR_INVALID_DELIMITER,) \
	_(ERR_INVALID_ERA,)		\
	_(ERR_UNKNOWN_ERR,)

DEFINE_ENUM (QofDateError, ENUM_ERR_LIST)

AS_STRING_DEC (QofDateError, ENUM_ERR_LIST)

/* \brief QofDate private replacement for strftime

(recursive).

 \param upcase used recursively to handle format specifiers
 that alter the case of the character to be generated.
 \param s  The buffer to hold the string being created.
 \param maxsize == MAX_DATE_BUFFER. Retained because of
 internal recursion.
 \param format The QofDateFormat string.
 \param qd The QofDate to parse.
 \param ut Use UTC if non-zero.
 \param ns Nanoseconds - for GNU %N extension.

 \return NULL on error or if the date is out of range
 of the specified format, otherwise the formatted string.
*/
size_t
strftime_case (gboolean upcase, gchar * s, size_t maxsize, 
	const gchar *format, const QofDate *qd, gint ut, glong ns);

/* \brief QofDate replacement for strptime

Returns a new QofDate from a string according to the 
QofDateFormat specified. The QofDate becomes the 
property of the caller and needs to be freed with
qof_date_free when done.

 \note Locale-specific formats are not available for the full
range of QofDate dates because the locale data for these
formats is only available via the underlying strftime implementation.
The formats affected are those involving the %E and %O modifiers 
and other format specifiers that use the current locale. 
e.g. Japanese Emperor reigns, local numeric specifiers etc. 
If these are used, qofstrptime cannot support the full range
because these implementations are not available to be extended.

The full list of affected format specifiers is:

 \verbatim
 'a', 'A', 'b', 'h', 'B', 'c', 'C', 'x', 'p', 'P',
 'r', 'X', 'E' and 'O'.
 \endverbatim

 \param rp The string to parse.
 \param fmt The QofDateFormat format string.
 \param qd The empty QofDate structure to use. Any
 existing values are overwritten; the QofDate does \b not
 have to be valid.
 \param error Pointer to a QofDateError value to store
 any errors encountered during processing. Uses
 QofDateErrorasString to convert the code to a string for
 logging with QofLogLevel.

 \return If an error occurs, returns the remainder of the 
 string to parse and sets error. On success, returns NULL
 and sets error to ERR_NO_ERROR.
*/
gchar *
strptime_internal (const gchar * rp, const gchar * fmt, 
	QofDate * qd, QofDateError * error);

#endif /* _QOFDATE_P_H */ 
