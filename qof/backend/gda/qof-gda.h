/********************************************************************
 *            qof-gda.h
 *
 *  Sat Sep  9 13:12:06 2006
 *  Copyright  2006  Neil Williams
 *  linux@codehelp.co.uk
 ********************************************************************/
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

#ifndef _QOF_GDA_H
#define _QOF_GDA_H

/** @addtogroup Backend
@{
*/
/**	@addtogroup LibGda qof-backend-gda outline

This is a prototype only, it is not yet fully functional. This
backend is to complement the libxml2 backend and provide full
database access for any QOF application without requiring a
permanent connection to a real database. A QofBook can be stored
in a GDA plugin (mysql, sqlite, postgres or odbc) when available,
or XML if not. Data can be queried independently of which backend
is in use via QofQuery.

KVP is an anomaly, it may or may not work.

Note that QOF_TYPE_GUID is stored as a string.

 @{
*/
/** @file  qof-gda.h
	@brief Public interface of qof-backend-gda
	@author Copyright 2006 Neil Williams <linux@codehelp.co.uk>
*/

/** \brief Initialises the libgda2 QOF backend.

Sets QOF GDA Backend Version 0.1, access method = gda:

The ID in all GDA tables created by QOF is the GUID of the entity,
expressed as a hexadecimal string.

The version number only changes if:
-# QOF_OBJECT_VERSION changes
-# The QofBackendProvider struct is modified in QOF to
support new members and SQLite can support the new function, or
-# The QofBackendOption settings are modified.

Initialises the backend and provides access to the
functions that will load and save the data. Initialises
default values for the QofBackendOption KvpFrame.

At present, qof_gda has no QofBackendOption options
and therefore no strings that are translatable.
*/

void qof_gda_provider_init(void);

/** @} */
/** @} */
#endif /* _QOF_GDA_H */
