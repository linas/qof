/****************************************************************
 *            qofsql-p.h
 *
 *  Mon Mar 17 11:26:49 GMT 2008
 *  Copyright  2008  Neil Williams
 *  linux@codehelp.co.uk
 ****************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

/** @addtogroup Query
@{ */
/**
    @file qofsql-p.h
    @brief Private QOF SQL generation routines
    @author Copyright (c) 2008 Neil Williams <linux@codehelp.co.uk>

 These functions are private - only accessible to QOF backends. The purpose
is to make it easier for SQL-based backends to pass SQL commands to the
relevant backend. There is currently no QOF support for reading the entities
back from the backend as each backend has specialized methods for data
retrieval (GDA has GdaDataModel, sqlite uses **columnNames etc.) Actually, it
is generally easier to read data from a SQL based backend than it is to create,
update or delete data.
*/

#ifndef _QOFSQL_P_H
#define QOFSQL_P_H

#include <glib.h>
#include "qof.h"

G_BEGIN_DECLS

/** @addtogroup SQL
 @{
*/

typedef enum
{
	/** no operation defined. init value. */
	SQL_NONE = 0,
	/** Create a new database */
	SQL_CREATE,
	/** Load all data from existing database. */
	SQL_LOAD,
	/** Write / sync all data to the database. */
	SQL_WRITE,
	/** Run a single INSERT statement. */
	SQL_INSERT,
	/** Run a single DELETE statement. */
	SQL_DELETE,
	/** Run a single UPDATE statement. */
	SQL_UPDATE
} QsqlStatementType;

/** \brief Set a default KVP table name for each backend
 @internal

Each backend can choose a different KVP table name by
overwriting the default name (sql_kvp) with this function.

 e.g. the SQLite backend uses 'sqlite_kvp'.
*/
void qof_sql_entity_set_kvp_tablename (const gchar * name);

/** \brief Build a SQL 'CREATE' statement for this entity
 @internal

  Prepares a SQL statement that will create a table for this
  entity.
*/
gchar *
qof_sql_entity_create_table (QofEntity * ent);

/** \brief Build a SQL 'INSERT' statement for this entity
 @internal

  Prepares a SQL statement that will insert data for this
  entity into the appropriate table (which must already exist).
*/
gchar *
qof_sql_entity_insert (QofEntity * ent);

/** \brief Build a SQL 'UPDATE' statement for this entity parameter
 @internal

  Prepares a SQL statement that will update a single parameter for this
  entity into the appropriate table (which must already exist).
  The data for the entity must already have been INSERTed into the table.

  \note this relies on qof_util_param_edit and qof_util_param_commit 
  which identify the particular parameter to be committed.
*/
gchar *
qof_sql_entity_update (QofEntity * ent);

/** \brief Build a SQL 'UPDATE' statement for a list of parameters
 @internal

  Prepares a SQL statement that will update the specified parameters for this
  entity into the appropriate table (which must already exist).
  The data for the entity must already have been INSERTed into the table.

 \bug unfinished function.
*/
gchar *
qof_sql_entity_update_list (QofEntity * ent, GList **params);

/** \brief Build a SQL 'DELETE' statement for this entity
 @internal

  Prepares a SQL statement that will delete the row for this
  entity into the appropriate table (which must already exist).
  The data for the entity must already have been INSERTed into the table.
*/
gchar *
qof_sql_entity_delete (QofEntity * ent);

/** \brief Build a SQL 'DROP' statement for this entity type
 @internal

  Prepares a SQL statement that will \b DROP the table for this
  entity type. (This function is fairly obvious but exists for
  completeness.) The table must already exist.
*/
gchar *
qof_sql_entity_drop_table (QofEntity * ent);

/** \brief Build a SQL 'CREATE' statement for this object
 @internal

  Prepares a SQL statement that will create a table for this
  object for those times when an entity does not yet exist.

*/
gchar *
qof_sql_object_create_table (QofObject * obj);

G_END_DECLS

/** @} */
/** @} */
#endif /* QOFSQL_P_H */
