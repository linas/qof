/********************************************************************\
 * qofinstance-p.h -- private fields common to all object instances *
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
/*
 * Object instance holds many common fields that most
 * QofObjects use.
 * 
 * Copyright (C) 2003 Linas Vepstas <linas@linas.org>
 */

#ifndef QOF_INSTANCE_P_H
#define QOF_INSTANCE_P_H
#include "config.h"
#include "qofinstance.h"
#include "qofclass.h"

struct QofInstance_s
{
	/** Globally unique id identifying this instance */
	QofEntity entity;

	/** The entity_table in which this instance is stored */
	QofBook *book;

	/** kvp_data is a key-value pair database for storing arbirtary
	 * information associated with this instance.  
	 * See src/engine/kvp_doc.txt for a list and description of the 
	 * important keys. */
	KvpFrame *kvp_data;

	/** Current QofParam being edited / committed 
	\since 0.7.1
	*/
	const QofParam * param;

	/**  Time of the last modification to this 
	instance.  Typically used to compare two versions of the
	same object, to see which is newer.  Reserved for SQL use, 
	to compare the version in local memory to the remote, 
	server version.
	\since 0.7.0
	*/
	QofTime *update_time;
#ifndef QOF_DISABLE_DEPRECATED
	Timespec last_update;
#endif
	/*  Keep track of nesting level of begin/end edit calls */
	gint editlevel;

	/*  In process of being destroyed */
	gboolean do_free;

	/*  dirty/clean flag. If dirty, then this instance has been modified,
	 *  but has not yet been written out to storage (file/database)
	 */
	gboolean dirty;
};

/* reset the dirty flag */
void qof_instance_mark_clean (QofInstance *);

void qof_instance_set_slots (QofInstance *, KvpFrame *);

/*  Set the update time. Reserved for use by the SQL backend;
 *  used for comparing version in local memory to that in remote 
 *  server. The QofTime becomes the property of the instance.
 */
void
qof_instance_set_update_time (QofInstance * inst, QofTime * time);

#endif /* QOF_INSTANCE_P_H */
