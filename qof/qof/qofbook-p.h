/********************************************************************\
 * qof-book-p.h -- private functions for QOF books.                 *
 *                                                                  *
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
/** @addtogroup Object
@{ */
/** @addtogroup Object_Private
@{ */
/** @file qofbook-p.h
	@brief Private QofBook interface.
	@author Copyright (c) 2001 by Rob Browning
	@author Copyright (c) 2001,2003 Linas Vepstas <linas@linas.org>
	@author Copyright 2006 Neil Williams <linux@codehelp.co.uk>
 */

/** @name  Book_Private
@{ */

#ifndef QOF_BOOK_P_H
#define QOF_BOOK_P_H

#include "kvp_frame.h"
#include "qofbackend.h"
#include "qofbook.h"
#include "qofid.h"
#include "qofid-p.h"
#include "qofinstance-p.h"
#include "qofundo-p.h"

/** Book structure */
struct _QofBook
{
	/** Unique guid for this book. */
	QofInstance inst;

	/** The entity table associates the GUIDs of all the objects
	belonging to this book, with their pointers to the respective
	objects.  This allows a lookup of objects based on their guid.
	*/
	GHashTable *hash_of_collections;

	/** In order to store arbitrary data, for extensibility,
	add a table	that will be used to hold arbitrary pointers. */
	GHashTable *data_tables;

	/** Hash table of destroy callbacks for the data table. */
	GHashTable *data_table_finalizers;

	/** state flag: 'y' means 'open for editing',
	'n' means 'book is closed'
	\todo shouldn't this be replaced by the instance editlevel ?
	*/
	gchar book_open;

	/** a flag denoting whether the book is closing down, used to
	help the QOF objects shut down cleanly without maintaining
	internal consistency.
	\todo shouldn't shutting_down be replaced by instance->do_free?
	*/
	gboolean shutting_down;

	/** version number, used for tracking multiuser updates */
	gint32 version;

	/** To be technically correct, backends belong to sessions and
	not books.  So the pointer below "really shouldn't be here",
	except that it provides a nice convenience, avoiding a lookup
	from the session.  Better solutions welcome ...
	\todo allow book to lookup session and then backend. */
	QofBackend *backend;

	/** Undo support object */
	QofUndo *undo_data;

	/** Backend private expansion data
	used by the sql backend for kvp management */
	guint32 idata;
};

/**
    qof_book_set_backend() is used by backends to
    initialize the pointers in the book structure to
    something that contains actual data.  These routines
    should not be used otherwise.  (Its somewhat questionable
    if the backends should even be doing this much, but for
    backwards compatibility, we leave these here.)
*/
void qof_book_set_backend (QofBook * book, QofBackend * be);

/** Register books with the framework */
gboolean qof_book_register (void);

/** @} */
/** @} */
/** @} */
#endif /* QOF_BOOK_P_H */
