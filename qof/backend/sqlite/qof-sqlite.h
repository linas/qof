/***************************************************************************
 *            qof-sqlite.h
 *
 *  Sun Jan 15 12:52:58 2006
 *  Copyright  2006  Neil Williams
 *  linux@codehelp.co.uk
 ****************************************************************************/
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifndef _QOF_SQLITE_H
#define _QOF_SQLITE_H

/** @addtogroup SQLite QOF-backend-SQLite outline

This is a prototype only, it is not yet functional. This backend is likely
to be used only by embedded systems - other, larger, systems will be able
to use a more comprehensive libgda backend that can connect with a variety
of databases using plugins.

All QOF backends need to map objects between types - I expect to be able to
map SQLite objects using SQL statements. This should allow processes to share
information easily and for the separate sqlite databases on the iPAQ to be
queried as a single data set.
@{
*/
/** @file  qof-sqlite.h
	@brief Public interface of qof-backend-sqlite
	@author Copyright 2006 Neil Williams <linux@codehelp.co.uk>
*/

/** \brief Describe this backend to the application. 

Sets QOF SQLite Backend Version 0.1, access method = sqlite:

The ID in all SQLite tables created by QOF is the GUID of the entity,
expressed as a hexadecimal string.

The version number only changes if:
-# QOF_OBJECT_VERSION changes
-# The QofBackendProvider struct is modified in QOF to
support new members and SQLite can support the new function, or
-# The QofBackendOption settings are modified.

Initialises the backend and provides access to the
functions that will load and save the data. Initialises
default values for the QofBackendOption KvpFrame.

Calls gettext because QofBackendOption
strings are translatable.
*/
void qof_sqlite_provider_init(void);

/** @} */

#endif /* _QOF_SQLITE_H */
