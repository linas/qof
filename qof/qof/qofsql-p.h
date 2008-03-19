/* ***************************************************************
 *            qofsql-p.h
 *
 *  Mon Mar 17 11:26:49 GMT 2008
 *  Copyright  2008  Neil Williams
 *  linux@codehelp.co.uk
 *************************************************************** */
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

/** @addtogroup QOF
 @{
*/
/**
    @file qofsql-p.h
    @brief Private QOF SQL generation routines
    @author Copyright (c) 2008 Neil Williams <linux@codehelp.co.uk>
*/

#ifndef QOFSQL_P_H
#define QOFSQL_P_H

#include <glib.h>
#include "qof.h"

/** @addtogroup SQL
 @{

 The qof_sql_entity* functions are private - only accessible to QOF backends.
 The purpose is to make it easier for SQL-based backends to pass SQL commands to the
 relevant database. There is currently no QOF support for reading the entities
 back from the backend as each backend has specialized methods for data
 retrieval (GDA has GdaDataModel, sqlite uses **columnNames etc.) Actually, it
 is generally easier to read data from a SQL based backend than it is to create,
 update or delete data.

 \note qof_sql_entity_update relies on qof_util_param_edit and 
 qof_util_param_commit which identify the particular parameter to be committed.
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

Each backend can choose a different KVP table name by
overwriting the default name (sql_kvp) with this function.

 e.g. the SQLite backend uses 'sqlite_kvp'.
*/
void qof_sql_entity_set_kvp_tablename (const gchar * name);

/** \brief Set the initial index value of the KVP table.

  Each backend table has an ID number for KVP entries as one
  QofEntity can have multiple KvpFrames. The ID number is mapped
  to the GUID of the entity when reading data back from the table.

  The ID is incremented after each call to ::qof_sql_entity_insert
  where ::qof_instance_get_slots does not return an empty frame.
*/
void qof_sql_entity_set_kvp_id (gulong id);

/** \brief Get the index value of the KVP table after the operation(s).

  Each backend table has an ID number for KVP entries as one
  QofEntity can have multiple KvpFrames. The ID number is mapped
  to the GUID of the entity when reading data back from the table.

  The ID is incremented after each call to ::qof_sql_entity_insert
  where ::qof_instance_get_slots does not return an empty frame.
*/
gulong qof_sql_entity_get_kvp_id (void);

/** \brief Set or clear a flag that the KVP table exists or not.
  
  The KVP table should only be created once per session - use
  this flag to indicate that the KVP table has been successfully
  created (or deleted).

  qof_sql_entity_create_table will not attempt to create the KVP
  table if this flag is set. It is up to the backend to control
  this flag.
*/
void qof_sql_entity_set_kvp_exists (gboolean exist);

/** \brief Build a SQL 'CREATE' statement for this entity

  Prepares a SQL statement that will create a table for this
  entity.
*/
gchar *
qof_sql_entity_create_table (QofEntity * ent);

/** \brief Build a SQL 'INSERT' statement for this entity

  Prepares a SQL statement that will insert data for this
  entity into the appropriate table (which must already exist).
*/
gchar *
qof_sql_entity_insert (QofEntity * ent);

/** \brief Build a SQL 'UPDATE' statement for the current entity parameter

  Prepares a SQL statement that will update a single parameter for this
  entity into the appropriate table (which must already exist).
  The data for the entity must already have been INSERTed into the table.
*/
gchar *
qof_sql_entity_update (QofEntity * ent);

/** \brief Build a SQL 'UPDATE' statement for the KVP data in this entity

  Prepares a SQL statement that will update the KVP data for this
  entity (if any) into the KVP table.

  This is a separate function because the KVP data can be modified
  independently of other parameters and updating a parameter should
  not cause an unwanted SQL operation on unchanged KVP data. If you
  know that the KVP data has changed, concatenate the two SQL commands
  into one.
*/
gchar *
qof_sql_entity_update_kvp (QofEntity * ent);

/** \brief Build a SQL 'UPDATE' statement for a list of parameters

  Prepares a SQL statement that will update the specified parameters for this
  entity into the appropriate table (which must already exist).
  The data for the entity must already have been INSERTed into the table.

 \bug unfinished function.
*/
gchar *
qof_sql_entity_update_list (QofEntity * ent, GList **params);

/** \brief Build a SQL 'DELETE' statement for this entity.

  Prepares a SQL statement that will delete the row for this
  entity into the appropriate table (which must already exist).
  The data for the entity must already have been INSERTed into the table.

  Also deletes all KVP data for this entity.
*/
gchar *
qof_sql_entity_delete (QofEntity * ent);

/** \brief Build a SQL 'DROP' statement for this entity type

  Prepares a SQL statement that will \b DROP the table for this
  entity type. (This function is fairly obvious but exists for
  completeness.) The table must already exist.
*/
gchar *
qof_sql_entity_drop_table (QofEntity * ent);

/** \brief Build a SQL 'CREATE' statement for this object

  Prepares a SQL statement that will create a table for this
  object for those times when an entity does not yet exist.
*/
gchar *
qof_sql_object_create_table (QofObject * obj);


/** @} */
/** @} */
#endif /* QOFSQL_P_H */
