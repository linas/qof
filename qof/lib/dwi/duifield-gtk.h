/********************************************************************\
 * duifield-gtk.h -- Implement the gtk-specific fields.             *
 * Copyright (C) 2002, 2003 Linas Vepstas <linas@linas.org>         *
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

/**
 * @addtogroup Field-GTK
 * @{
 * @file: duifield-gtk.h
 * @brief: Implement the gtk-specific fields.
 * @author: Copyright (C) 2002, 2003 Linas Vepstas <linas@linas.org>
 */

#ifndef DUI_FIELD_GTK_H
#define DUI_FIELD_GTK_H

#include "config.h"

#include "duifield.h"
#include "duiresolver.h"
#include "interface.h"

/** XXX should probably convert these to work with named 
 * columns instead of numbered columns */

/** Data is linked to the contents of a GTK widget.  If its a 
 *  columned widget, the column number identifies which column 
 *  to work with. 
 */
void dui_field_set_widget (DuiField *fs, 
                             const char * widname,
                             int colnum);

/** Data is linked to the contents of a GTK widget.  If its a 
 *  columned widget, the column number identifies which column 
 *  to work with. This widget will be matched as to value, 
 *  serving as the 'where' clause in an sql query.  That is,
 *  'change value X where/when Y .compareop. Z'
 */
void dui_field_set_wid_where (DuiField *fs, 
                             const char * widname,
                             int colnum,
                             const char * compareop);

/** Data is linked to a private hash table stored with the widget.
 *  The 'key' identifies the hash table entry.  If this is a columned
 *  widget, there will be a distinct hash table for each column.
 */
void dui_field_set_wid_data (DuiField *fs, 
                             const char * widname,
                             int colnum,
                             const char * datakey);

/** Data is linked to a GTK-1.2 GtkArg on the indicated widget 
 *  (or widget column if this is a columned widget).
 */
void dui_field_set_wid_arg (DuiField *fs, 
                            const char * widname,
                            int colnum,
                            const char * arg);

void dui_resolver_resolve_widgets (DuiResolver *res , DuiWindow *win);

#endif /* DUI_FIELD_GTK_H */
/** @} */
