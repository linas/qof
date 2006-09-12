/********************************************************************\
 * qofbackend.h: api for data storage backend                       *
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 51 Franklin Street, Fifth Floor    Fax:    +1-617-542-2652       *
 * Boston, MA  02110-1301,  USA       gnu@gnu.org                   *
 *                                                                  *
\********************************************************************/
/** @addtogroup Backend

    The QOF Backend is a pseudo-object providing an interface between
	the engine and a persistant data store (e.g. a server, a database,
	or a file).   Backends are not meant to be used directly by an
    application; instead the Session should be used to make a 
    connection with some particular backend.
    There are no backend functions that are 'public' to
    users of the engine.  The backend can, however, report errors to
    the GUI & other front-end users.  This file defines these errors.
   
    Backends are used to save and restore Entities in a Book.
    @{ 
*/
/** @file qofbackend.h
    @brief API for data storage Backend
    @author Copyright (C) 2000-2001 Linas Vepstas <linas@linas.org>
    @author Copyright 2004,2005,2006 Neil Williams
	<linux@codehelp.co.uk>
*/

#ifndef QOF_BACKEND_H
#define QOF_BACKEND_H

#include "qofinstance.h"

#define QOF_MOD_BACKEND "qof-backend"

/** \brief The ID of this error.

0 == QOF_SUCCESS
(equivalent to ERR_BACKEND_NO_ERR )
*/
typedef gint32 QofErrorId;

/** A structure that declares backend services that can be gotten.
The Provider specifies a URL access method, and specifies the
function to create a backend that can handle that URL access
function. */
typedef struct QofBackendProvider_s QofBackendProvider;

/** \brief Pseudo-object providing an interface between the
framework and a persistant data store (e.g. a server, a database, 
or a file).

There are no backend functions that are 'public' to users of the
framework. The backend can, however, report errors to the GUI
& other front-end users. */
typedef struct QofBackend_s QofBackend;

/** \brief DOCUMENT ME! */
typedef void (*QofBePercentageFunc) (const gchar * message, double percent);

/** @name Allow access to the begin routine for this backend.

QOF_BEGIN_EDIT and QOF_COMMIT_EDIT_PART1 and part2 rely on 
calling QofBackend *be->begin and be->commit. This means the
QofBackend struct becomes part of the public API.
These function replaces those calls to allow the macros to be
used when QOF is built as a library.
@{
*/
void qof_backend_run_begin (QofBackend * be, QofInstance * inst);

gboolean qof_backend_begin_exists (QofBackend * be);

void qof_backend_run_commit (QofBackend * be, QofInstance * inst);

gboolean qof_backend_commit_exists (QofBackend * be);
/** @} */

/** @name Backend Configuration using KVP

The backend uses qof_backend_get_config to pass back a KvpFrame of
QofBackendOption that includes the \b translated strings that serve
as description and tooltip for that option.

qof_backend_prepare_frame, qof_backend_prepare_option and
qof_backend_complete_frame are intended to be used by the
backend itself to create the options.

qof_backend_get_config, qof_backend_option_foreach and
qof_backend_load_config are intended for either the backend or the
frontend to retrieve the option data from the frame or set new data.

Backends are loaded using QofBackendProvider via the function
specified in prov->backend_new. Before backend_new returns, you
should ensure that your backend is fully configured and ready for use.

@{
*/

/** A single Backend Configuration Option. */
typedef struct QofBackendOption_s
{
	KvpValueType type;		   /**< Only GINT64, DOUBLE, NUMERIC,
	STRING and TIME supported. TIMESPEC is deprecated. */
	const gchar *option_name;  /**< non-translated, key. */
	const gchar *description;  /**< translatable description. */
	const gchar *tooltip;	   /**< translatable tooltip */
	gpointer value;			   /**< The value of the option. */
} QofBackendOption;

/** Initialise the backend_configuration */
void qof_backend_prepare_frame (QofBackend * be);

/** Add an option to the backend_configuration. Repeat for more. */
void qof_backend_prepare_option (QofBackend * be, QofBackendOption * option);

/** Complete the backend_configuration and return the frame. */
KvpFrame *qof_backend_complete_frame (QofBackend * be);

/** Backend configuration option foreach callback prototype. */
typedef void (*QofBackendOptionCB) (QofBackendOption *, gpointer data);

/** Iterate over the frame and process each option. */
void
qof_backend_option_foreach (KvpFrame * config, QofBackendOptionCB cb,
							gpointer data);

/** \brief Load configuration options specific to this backend.

@param be The backend to configure.
@param config A KvpFrame of QofBackendOptions that this backend
will recognise. Each backend needs to document their own config
types and acceptable values.

*/
void qof_backend_load_config (QofBackend * be, KvpFrame * config);

/** \brief Get the available configuration options

To retrieve the options from the returned KvpFrame, the caller
needs to parse the XML file that documents the option names and
data types. The XML file itself is part of the backend and is
installed in a directory determined by the backend. Therefore,
loading a new backend requires two paths: the path to the .la file
and the path to the xml. Both paths are available by including a
generated header file, e.g. gncla-dir.h defines GNC_LIB_DIR for
the location of the .la file and GNC_XML_DIR for the xml.

@param be The QofBackend to be configured.

@return A new KvpFrame containing the available options or
NULL on failure.

*/
KvpFrame *qof_backend_get_config (QofBackend * be);
/** @} */

/** \brief Load a QOF-compatible backend shared library.

\param directory Can be NULL if filename is a complete path.
\param filename  Name of the .la file that describes the
	shared library. This provides platform independence,
	courtesy of libtool.
\param init_fcn  The QofBackendProvider init function.

\return FALSE in case or error, otherwise TRUE.
*/
gboolean
qof_load_backend_library (const gchar * directory,
						  const gchar * filename, const gchar * init_fcn);

/** \brief Retrieve the backend used by this book */
QofBackend *qof_book_get_backend (QofBook * book);

/** \brief Set the backend used by this book.

Should only be used within a backend itself.
*/
void qof_book_set_backend (QofBook * book, QofBackend *);

/** @} */

#endif /* QOF_BACKEND_H */
