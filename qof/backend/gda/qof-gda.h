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
 
#ifndef _QOF-GDA_H
#define _QOF-GDA_H

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

<b>QofIdType and GdaValue conversions.</b>

GdaValue supports a much wider range of value types than
QofIdType provides. QOF uses a subset of GdaValueType 
and some of the conversions may not appear intuitive.

GDA_VALUE_TYPE_DATE is not equivalent to QofDate -
the GdaValueType only supports day, month and year.
GDA_VALUE_TYPE_TIME is not equivalent to QofTime - 
the GdaValueType only supports hour, minute and second
(all gushort values) and a glong for timezone (::qd_gmt_off).
Both would involve data loss in converting to QofDate or QofTime.
QOF therefore uses GDA_VALUE_TYPE_TIMESTAMP for QofTime.

GdaValueType also includes GDA_VALUE_TYPE_MONEY and 
GDA_VALUE_TYPE_NUMERIC but these do not match with QofNumeric.
GDA_VALUE_TYPE_MONEY is just a double with a currency mnemonic
as a string (QOF does not store the currency with the numeric.)
GDA_VALUE_TYPE_NUMERIC has a string value for the number and
requires precision and width settings as would be used by 
printf. QofNumeric involves a denominator and numerator so
conversion to either a string or a double is required. Currently,
QOF uses a double.

KVP is another anomaly, GDA_VALUE_TYPE_LIST may or may not work.

Note that QOF_TYPE_GUID is stored as a string.

\verbatim
typedef enum {
	GDA_VALUE_TYPE_NULL,
	GDA_VALUE_TYPE_BIGINT,			QOF_TYPE_INT64
	GDA_VALUE_TYPE_BIGUINT,
	GDA_VALUE_TYPE_BINARY,
	GDA_VALUE_TYPE_BLOB,
	GDA_VALUE_TYPE_BOOLEAN,			QOF_TYPE_BOOLEAN
	GDA_VALUE_TYPE_DATE,
	GDA_VALUE_TYPE_DOUBLE,			QOF_TYPE_DOUBLE, QOF_TYPE_NUMERIC
									QOF_TYPE_DEBCRED
	GDA_VALUE_TYPE_GEOMETRIC_POINT,
	GDA_VALUE_TYPE_GOBJECT,
	GDA_VALUE_TYPE_INTEGER,			QOF_TYPE_INT32
	GDA_VALUE_TYPE_LIST,			QOF_TYPE_KVP
	GDA_VALUE_TYPE_MONEY,
	GDA_VALUE_TYPE_NUMERIC,			
	GDA_VALUE_TYPE_SINGLE,
	GDA_VALUE_TYPE_SMALLINT,
	GDA_VALUE_TYPE_SMALLUINT,
	GDA_VALUE_TYPE_STRING,			QOF_TYPE_STRING, QOF_TYPE_GUID
									QOF_TYPE_CHAR
	GDA_VALUE_TYPE_TIME,
	GDA_VALUE_TYPE_TIMESTAMP,		QOF_TYPE_TIME
	GDA_VALUE_TYPE_TINYINT,
	GDA_VALUE_TYPE_TINYUINT,
	GDA_VALUE_TYPE_TYPE,
	GDA_VALUE_TYPE_UINTEGER,
	GDA_VALUE_TYPE_UNKNOWN
} GdaValueType;

typedef struct {
	gint defined_size;
	gchar *name;				param_name
	gchar *table;				obj->e_type
	gchar *caption;
	gint scale;
	GdaValueType gda_type;
	gboolean allow_null;
	gboolean primary_key;
	gboolean unique_key;
	gchar *references;
	gboolean auto_increment;
	glong auto_increment_start;
	glong auto_increment_step;
	gint position;
	GdaValue *default_value;
} GdaFieldAttributes;
\endverbatim

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

#endif /* _QOF-GDA_H */
