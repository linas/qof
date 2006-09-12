/********************************************************************
 *            qoferror.h
 *
 *  Sun Sep 10 19:55:48 2006
 *  Copyright  2006  Neil Williams
 *  linux@codehelp.co.uk
 *******************************************************************/
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
 
#ifndef _QOFERROR_H
#define _QOFERROR_H

#include "qofsession.h"

/** @addtogroup Error

QofError supports the creation of new error codes (complete with
error strings) along the lines of GdaError. Applications and
backends can generate their own QofError values and register them
with QofError. Any function can then set this error value and
retrieve the error with the deprecated ::qof_session_get_error and 
::qof_session_get_error_message or the new style ::qof_error_get_id
and ::qof_error_get_message. The main advantage is that
applications can set error states that are unrelated to the old
QofBackendError values but retrieve all errors in the same manner.

- Use QofLog to record information for programmers and maintainers.
- Use QofError to communicate descriptive error messages to the user.

Each error stack is specific to one QofSession.

Applications can register new error values with ::qof_error_register 
passing the error message string, already marked for translation -
a new QofErrorId will be returned. 

Each backend can also generate specific QofError values, in
which case the translation is done within QOF.

Set an error by passing the QofErrorId (or the deprecated
QofBackendError) to ::qof_error_set.

To check the error condition use ::qof_error_check - if an error
has been set, qof_error_check returns the QofErrorId of that error
without clearing the error from the stack.

To retrieve an error and clear it from the stack, use
::qof_error_get_id or ::qof_error_get_message.

Precise values of QofErrorId are \b not to be stored in applications
as values (other than deprecated values) may change at any time. 

There are no default errors - previous QofBackendError values
are retained only as deprecated macros. Until libqof2, QofErrorId
is guaranteed not to overlap a previous QofBackendError value but
once deprecated code is removed in libqof2, any value can be used.

This deliberately makes it harder to re-use the same error time
after time. The purpose is to encourage more detailed error
reporting by supporting an unlimited number of error values.

Applications and backends can store the QofErrorId in a context
or static values if the error must be set from multiple locations,
otherwise an error can be registered and set locally.

If a subsystem or dependency generates an error message of it's own,
this can also be passed to qof_error_register to generate a new
error within the session, complete with the (translated) message
direct from the subsystem. This increases the detail and clarity
of the messages presented to the user. Programming errors and 
complex errors should still be logged using QofLog - QofError
is for messages destined for the end user of the application using
QOF.

Many applications already include message strings for the previous
QofBackendError values but all are welcome to move to the new
QofError strings. 

QofError strings remain the property of QofError and should not
be freed.

@since 0.8.0

@{
*/

/** @file qoferror.h
    @brief Extensible error handling
    @author Copyright 2006 Neil Williams <linux@codehelp.co.uk>
*/

/** opaque QofError type. */
typedef struct QofError_s QofError;

/** success value */
#define QOF_SUCCESS 0

/** \brief general error value

Can be returned by any function handling QofErrorId
to indicate a fatal error, e.g. g_return_val_if_fail
*/
#define QOF_FATAL -1

/** \brief Generate and register a new error 

@param err_message The user-friendly string to add as
	an error, already marked for translation.

@return The QofErrorId of this error.
*/
QofErrorId
qof_error_register (const gchar * err_message);

/** \brief Add an error to the stack for this session.

@param session The session that raised the error.
@param error  The QofErrorId of the error to be recorded.
*/
void
qof_error_set (QofSession * session, QofErrorId error);

void
qof_error_set_be (QofBackend * be, QofErrorId error);

/** \brief clear the error stack for the session.

Applications should clear the stack once errors have been
presented to the user.
*/
void
qof_error_clear (QofSession * session);


/** \brief Check a session for errors

@param session The session to check.

@return QOF_SUCCESS if no errors have been set, otherwise
	the QofErrorId of the most recently set error.
*/
QofErrorId
qof_error_check (QofSession * session);

/** \brief Get the time of the most recent error

All QofError values are timestamped at the moment
that the error is set.

@param session The session where the error was set.

@return NULL if no error exists, otherwise the QofTime
	that the error was set.
*/
QofTime *
qof_error_get_time_be (QofBackend * be);

/** \brief Alternative for applications */
QofTime *
qof_error_get_time (QofSession * session);

/** @brief Pop the most recent error from the session stack

Returns and clears the most recently set error for this
session, if any.

@param session The session that recorded the error.

@return QOF_SUCCESS if no errors have been set, otherwise
	the QofErrorId of the most recently set error.
*/
QofErrorId
qof_error_get_id_be (QofBackend * be);

/** \brief Alternative for applications */
QofErrorId
qof_error_get_id (QofSession * session);

/** @brief Pop the most recent error and get the message.

Clears the most recently set error for this session and
returns the error message, if any.

@param session The session that recorded the error.

@return NULL if no errors have been set, otherwise
	the translated message for the most recently set error.
*/
const gchar *
qof_error_get_message_be (QofBackend * be);

/** \brief Alternative for applications. */
const gchar *
qof_error_get_message (QofSession * session);

/** @} */
#endif /* _QOFERROR_H */