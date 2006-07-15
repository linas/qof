/***************************************************************************
 *            qofundo-p.h
 *
 *  Thu Aug 25 09:20:14 2005
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

#ifndef _QOFUNDO_P_H
#define _QOFUNDO_P_H

#include <qof.h>
#include "qofundo.h"

/** @brief The parameter changes, one or more per affected entity 

One per parameter change - the bottom level of any undo. A single click of 
Undo could use the data from one or many parameter changes - as determined by 
the event. Each parameter change can be for any entity of any registered type 
in the book and parameter changes can be repeated for the multiple changes to 
different parameters of the same entity. The combination of param, guid and 
type will be unique per event. (i.e. no event will ever set the same 
parameter of the same entity twice (with or without different data) in one 
undo operation.)
*/
typedef struct QofUndoEntity_t QofUndoEntity;

/** @brief The affected entities, one or more per operation 

The top level of any undo. Contains a GList that keeps the type of operation
and the GList of qof_undo_entity* instances relating to that operation. Some
form of index / counter probably too in order to speed up freeing unwanted
operations and undo data upon resumption of editing and in controlling the
total number of operations that can be undone.

Each qof_undo_event.entity_list can contain data about more than 1 type of
entity.
*/
typedef struct QofUndoOperation_t QofUndoOperation;

typedef struct QofUndo_s
{
	GList *undo_list;
	GList *undo_cache;
	gchar *undo_label;
	gint index_position;
	gboolean undo_operation_open;
} QofUndo;

/* Undo is limited, not infinite. */
#define MAX_UNDO_LENGTH     300

/* reads the data from this parameter to allow undo

To be able to undo and then redo an action, QOF needs to know the
before and after states. Initially, the before state is the same as
the file but after that point, the state of the entity needs to be
tracked whenever it is opened for editing.
*/
QofUndoEntity *
qof_prepare_undo (QofEntity * ent, const QofParam * param);

/* Add the changes to be undone to the event.

Designed to be used with g_list_foreach, simply adds
any number of undo_entity pointers (representing the 
entity changes relating to this event) to the list 
of changes for this event.
*/
void 
qof_undo_new_entry (gpointer event, gpointer changes);

/* Add an undo event to the list.

type holds the type of event that has just occurred.

If the event follows a successful qof_commit_edit, then the
cached undo_entity changes are placed into this undo_event.
*/
QofUndoOperation *
qof_undo_new_operation (QofBook * book, gchar * label);

#endif /* _QOFUNDO_P_H */
