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
retrieve the error with ::qof_error_get_id or 
::qof_error_get_message. The main advantage is that
applications can set error states that are unrelated to the old
QofBackendError values but retrieve all errors in the same manner.

- Use QofLog to record information for programmers and maintainers.
- Use QofError to communicate descriptive error messages to the user.

An error must be registered to be set. Registered errors can be
set repeatedly into an error stack for the relevant session.
Setting an error copies the registered error to the error stack
and sets a time index in the copy.

Once an error has been unregistered, it cannot be set later.
If the error has already been set on the error stack, the
stack is \b not changed and the error remains readable.

Each error stack is specific to one QofSession.

Registered errors can be set in any session (if the 
QofErrorId is known) but most errors are specific to one session.

Applications can register new error values with ::qof_error_register 
passing the error message string, already marked for translation -
a new QofErrorId will be returned. Error values are unregistered
when the session ends or can be unregistered manually.

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

/** QofError log_module name. */
#define QOF_MOD_ERROR "qof-error-module"

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
@param use_file  TRUE if the session filename should be
	substituted in the string - err_message \b must
	contain a bare string format specifier: %s. Note that 
	flags, width, precision or size specifiers are \b not
	accepted and the filename is output in full,
	complete with the access_method.
	e.g. file:/home/user/app/data.xml

To use a different presentation of the filename or other
customised strings, prepare the error message before
registering it with QofError.

Registered errors are cleared when the session is destroyed.

Applications need to plan the use of locally registered
error codes so that the same errors are not repeatedly
registered.

@return The QofErrorId of this error.
*/
QofErrorId
qof_error_register (const gchar * err_message, gboolean use_file);

/** \brief Unregister an error

Registered errors are normally freed when the session ends.
Errors can also be unregistered (and freed) directly.

An unregistered error can not be set later. 
*/
void
qof_error_unregister (QofErrorId id);

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

/** \brief Check for errors

@param be The backend to check.

@return QOF_SUCCESS if no errors have been set, otherwise
	the QofErrorId of the most recently set error.
*/
QofErrorId
qof_error_check_be (QofBackend * be);

/** alternative for applications. */
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
