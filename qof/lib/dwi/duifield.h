/********************************************************************\
 * duifield.h - abstract base class for a 'field'.                  *
 * Copyright (C) 2003 Linas Vepstas <linas@linas.org>               *
 * http://dwi.sourceforge.net                                       *
 *                                                                  *
 * This library is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU Lesser General Public       *
 * License as published by the Free Software Foundation; either     *
 * version 2.1 of the License, or (at your option) any later version.
 *                                                                  *
 * This library is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU Lesser General Public License for more details.              *
 *                                                                  *
 * You should have received a copy of the GNU Lesser General Public *
 * License along with this program; if not, contact:                *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
\********************************************************************/
/** @addtogroup Field
 *  Abstract base class for the 'field'.
 *  The 'field maps to a column on an sql db, or to the value 
 *  contained in an interface widget, etc. etc. 
 *  @{
 *  @file: duifield.h
 *  @brief: abstract base class for a 'field'.
 *  @author: Copyright (c) 2003 Linas Vepstas <linas@linas.org>
 */

#ifndef DUI_FIELD_H
#define DUI_FIELD_H

#include "config.h"
#include <string.h>

typedef struct DuiField_s DuiField;
typedef const char * DuiFieldType; 

#define DUI_FIELD_NONE      "DUI_FIELD_NONE"
#define DUI_FIELD_CONST     "DUI_FIELD_CONST"    /**< Field with const value */
#define DUI_FIELD_WIDGET    "DUI_FIELD_WIDGET"   /**< Value in gtk widget itself */
#define DUI_FIELD_WID_DATA  "DUI_FIELD_WID_DATA"  /**< Value from gtk-1.2 widget data */
#define DUI_FIELD_WID_ARG   "DUI_FIELD_WID_ARG"   /**< Value from gtk-1.2 widget GtkArg */
#define DUI_FIELD_WID_WHERE "DUI_FIELD_WID_WHERE" /**< Match to a particular gtk widget */
#define DUI_FIELD_HASH_KEY  "DUI_FIELD_HASH_KEY"  /**< Global hash table key */
#define DUI_FIELD_SQL       "DUI_FIELD_SQL"       /**< Database table column */
#define DUI_FIELD_WHERE     "DUI_FIELD_WHERE"     /**< match to database column */
#define DUI_FIELD_GOBJ      "DUI_FIELD_GOBJ"      /**< glib-2.0 gobject */
#define DUI_FIELD_QOF       "DUI_FIELD_QOF"       /**< QOF Object */
#define DUI_FIELD_QOF_MATCH "DUI_FIELD_QOF_MATCH" /**< Match to a particular QOF Instance */

#define DUI_FIELD_IS_TYPE(fld,typ) (0==strcmp((fld)->type,typ))

struct DuiField_s
{
	DuiFieldType type;
	char * fieldname;

	/** Return the value associated with this field */
	const char * (*get_field_value) (DuiField *);

	/** Set a value for this field; may be null for const fields. */
	void (*set_field_value) (DuiField *, const char *);

	/** Reset this field to a default value */
	void (*clear_field) (DuiField *);

	/** Private, use the field routines below. */
	void     (*iter_pre)    (DuiField *, gboolean do_clear);
	gboolean (*iter_next)   (DuiField *);
	void     (*iter_column) (DuiField *, DuiField *);
	void     (*iter_post)   (DuiField *);
	union 
	{
		/** Just a plain static, const value */
		char * value;

		/** Private, per-type data area */
		gpointer priv[8];
	} u;
};

/** Get the name of the field */
const char * dui_field_get_fieldname (DuiField *fs);

/** Erase the value stored in teh field. */
void dui_field_clear (DuiField *s);

/** data will be a constant unchanging value */
void dui_field_set_const (DuiField *fs, const char *value);

/** @addtogroup FieldIterator 
 *  For target fields grouped in a table-like organization, 
 *  the matcher is used to iterate/select rows.   It acts as
 *  a "WHERE" term that can be used to match a target row 
 *  for target updates.  The 'matcher' value is updated 
 *  for every source row, which is why the matcher value 
 *  can be used to select a target row.
 */
/** @{ */
/**  The dui_field_iter_pre() will be called before the start of
 *  the source row iteration.
 *  Set the do_clear flag to true if you want to clear the table
 *  before you fill it in.  Otherwise, if you are matching only
 *  certain rows, you probably don't want to clear the table.
 */
void dui_field_iter_pre (DuiField *matcher, gboolean do_clear);

/** The dui_field_iter_next() will be called once for each
 *  row of the source table.  The value passed in 'matcher' will
 *  be based on the matching term in the source table.
 *  It should return TRUE if the source is to be mapped to the
 *  target, else it should return FALSE.
 */
gboolean dui_field_iter_next (DuiField *matcher);

/** This routine will be called once for every target column
 *  in the target row.  The matcher will be the same field as
 *  previously set up with dui_field_iter_next().
 */
void dui_field_iter_column (DuiField *target_column, DuiField *matcher);

/** Called at the end of the source row iteration. */
void dui_field_iter_post (DuiField *matcher);

/** @} */

#endif /* DUI_FIELD_H */
/** @} */
