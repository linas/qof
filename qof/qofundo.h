/***************************************************************************
 *            qofundo.h
 *
 *  Thu Aug 25 09:19:25 2005
 *  Copyright  2005,2006  Neil Williams
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA  02110-1301,  USA
 */

/** @addtogroup UNDO

 QOF Undo operates within a QofBook. In order to undo the changes
 to the entity, the initial state of each parameter is cached
 when an operation begins. If the entity changes are not 
 successful, the lack of a ::qof_book_end_operation call before a 
 ::qof_book_start_operation will cause the cached data to be freed.
 If the entity is changed successfully, ::qof_book_end_operation 
 will create the undo data using the operation label and each of 
 the entity changes that were successful.

 Within each operation, entity changes can be recorded using
 ::QofEventHandler or individually.
- New entity: call ::qof_undo_create after creation.
 	(::QOF_EVENT_CREATE)
- Before editing a parameter of an entity, ::qof_undo_modify
	(::QOF_EVENT_MODIFY)
- After editing a parameter of an entity, ::qof_undo_commit
	(::QOF_EVENT_COMMIT)
- Before deleting an entity, ::qof_undo_delete.
	(::QOF_EVENT_DESTROY)

 Undo data consists of a list of operations that have changed data
 in this book and a list of entity changes for each of those 
 operations. Each operation can relate to more than one entity 
 change and cover more than one entity but must only relate to 
 one book.
 
-# Only QOF parameter changes can be undone or redone. Data from 
 structs that are not QOF objects or which have no QofParam to 
 get <b>and set</b> the data will not be available to the undo 
 process.
-# Undo relates to 'user interface operations', not engine events.
 This is because an operation (like an import or voiding a 
 transaction) can involve multiple, possibly conflicting, engine 
 events - e.g. removing an entity from one reference and inserting 
 it as another. Therefore, the UI developer alone can decide where 
 an operation begins and ends. All changes between the two will be 
 undone or redone in one call to ::qof_book_undo.
-# Undo operations \b cannot be nested. Be careful where you
 start and end an undo operation, if your application calls 
 qof_book_start_operation() before calling 
 qof_book_end_operation(), the undo cache will be freed and 
 QofUndo will not notify you of this. The API is designed to 
 silently handle user aborts during a user operation. As undo 
 data is cached when editing begins, if the edit is never
 completed the cache must be cleared before the next operation.
 i.e. if the user starts to edit an entity but then cancels 
 the operation, there are no changes to undo. It follows that
 any one book can only be the subject of one operation at a time.

\since 0.7.0

@{
 */
/** @file  qofundo.h
    @brief QOF undo handling
    @author Copyright (c) 2005,2006  Neil Williams <linux@codehelp.co.uk>
*/
#ifndef _QOFUNDO_H
#define _QOFUNDO_H

#define QOF_MOD_UNDO "qof-undo"

/** \brief Set a value in this parameter of the entity.

Setting an arbitrary parameter in an entity can involve
repetitive string comparisons and setter function prototypes.
This function accepts a QofParam (which determines the type of 
value) and a string representing the value. e.g. for a boolean,
pass "TRUE", for a GUID pass the result of guid_to_string_buff.

Sets the undo data for this modification at the same time,
calling qof_undo_modify, sets the parameter and qof_undo_commit.

@param ent An initialised QofEntity from an accessible QofBook.
@param param from ::qof_class_get_parameter
@param value A string representation of the required value - 
original type as specified in param->param_type.
*/
void 
qof_undo_set_param (QofEntity * ent, const QofParam * param, 
					gchar * value);

/** Mark this instance parameter before modification.

Prepare undo data for this parameter of this instance.
Record the initial state of this parameter of this
instance in preparation for modification so that 
undo can reset the value if required.
*/
void 
qof_undo_modify (QofInstance * inst, const QofParam * param);

/** Mark this instance parameter after modification

Prepare undo data for this instance after committal.
Record the modified state of this parameter of this
instance so that if this operation is undone and then redone,
the modification can be recreated.
*/
void 
qof_undo_commit (QofInstance * inst, const QofParam * param);

/** prepare undo data for a new instance.

Record the creation of a new (empty) instance so that
undo can delete it (and recreate it on redo).

Can be used within a ::QofEventHandler in response
to ::QOF_EVENT_CREATE.
*/
void qof_undo_create (QofInstance * inst);

/** prepare undo data for an instance to be deleted.

Prepare for the deletion of an entity by storing \b ALL 
data in all editable parameters so that this delete 
operation can be undone.

Can be used within a ::QofEventHandler in response
to ::QOF_EVENT_DESTROY, \b before the instance itself
is deleted.
*/
void qof_undo_delete (QofInstance * inst);

/** \brief Free the entire undo list for this book. 

The application needs to decide whether to reset
the undo list upon session_save, application close,
user intervention etc.
*/
void qof_book_clear_undo (QofBook * book);

/** @brief Set parameter values from before the previous event. */
void qof_book_undo (QofBook * book);

/** @brief Set parameter values from after the previous event. */
void qof_book_redo (QofBook * book);

/** @brief event handler for undo widget 

 @return FALSE if length == 0 or index_position == 0,
 otherwise TRUE.
*/
gboolean qof_book_can_undo (QofBook * book);

/** @brief event handler for redo widget

@return FALSE if index_position == 0 or index_position == length
otherwise TRUE.
*/
gboolean qof_book_can_redo (QofBook * book);

/** \brief Start recording operation.

*/
void qof_book_start_operation (QofBook * book, gchar * label);

/** \brief End recording the current operation. */
void qof_book_end_operation (QofBook * book);

/** \brief HIG compliance aid to report time of first change. */
QofTime *
qof_book_undo_first_modified (QofBook * book);

/** \brief Number of undo operations available. */
gint qof_book_undo_count (QofBook * book);

#endif /* _QOFUNDO_H */

/** @} */
