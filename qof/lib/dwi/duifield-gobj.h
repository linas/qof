/********************************************************************\
 * duifield-gobj.h -- Implementation of the glib-2.0 GObject field  *
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

/*
 * FUNCTION:
 * Implement the field for GLib-2 GObjects
 *
 * HISTORY:
 * Linas Vepstas September 2003
 */

#ifndef DUI_FIELD_GOBJ_H
#define DUI_FIELD_GOBJ_H

#include "config.h"

#include "duiresolver.h"
#include "duifield.h"
#include "interface.h"

/** data is a property on a glib-2.0 gobject */
void dui_field_set_gobj (DuiField *ft,
                           const char * obj,
                           const char * prop);

void dui_resolver_resolve_gobj (DuiResolver *res , DuiInterface *rot);

#endif /* DUI_FIELD_GOBJ_H */
