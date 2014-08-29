/***************************************************************************
 *            qof-sqlite.h
 *
 *  Sun Jan 15 12:52:58 2006
 *  Copyright  2006-2007  Neil Williams
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/** @addtogroup Backend
    @{ */
/** @addtogroup SQLite QOF-backend-SQLite support

The QOF SQLite backend is now mostly functional. This
backend is designed for use by embedded systems where libxml2
is too large - other, larger, systems will be able to use a more 
comprehensive libgda backend that can connect with a variety
of databases using plugins.

SQLite is typeless so the types defined for the various QOF
parameter types act as more of a guide than a rule.

 \since 0.7.3 KVP support - a separate table in the sqlite file
in order to support a value, a path and a type for each KvpValue 
and to support more than one value (more than one frame) per entity. 
i.e. a relational database arrangement where the entity record contains a 
reference to the kvp table. That reference id is the GUID of the entity.
The KVP table name is preset and created as:
 \verbatim
CREATE TABLE sqlite_kvp 
("kvp_id int primary key not null", "guid char(32)", "path mediumtext", "type mediumtext", "value text", 
END_DB_VERSION);
 \endverbatim
Entity tables therefore do not contain kvp data. Although the KVP table uses
an internal ID number, all lookups are done via the GUID of the entity.

    @{ */
/** @file  qof-sqlite.h
	@brief Public interface of qof-backend-sqlite
	@author Copyright 2006 Neil Williams <linux@codehelp.co.uk>
*/

#ifndef _QOF_SQLITE_H
#define _QOF_SQLITE_H

/** \brief Initialises the SQLite backend. 

Sets QOF SQLite Backend Version 0.3, access method = sqlite:

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

Instance KvpFrames are retrieved and stored as a separate table
in the same SQLite file "sqlite_kvp". The primary key is an internal,
sequential, identifier that does not need to be exposed. Each
KvpValue is one record in the kvp table - frames are just a type
of value. Records include fields for the path, type and value of
the KvpValue as well as the GUID of the entity that holds the
value.

Table name: sqlite_kvp
Table values: internal_id, guid_as_string, path, type, value


At present, qof_sqlite has no QofBackendOption options
and therefore no strings that are translatable.
*/
void qof_sqlite_provider_init (void);

/** @} */
/** @} */

#endif /* _QOF_SQLITE_H */
