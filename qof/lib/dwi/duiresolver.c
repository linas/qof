/********************************************************************\
 * duiresolver.c -- Resolve names into actual pointers              *
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

/* duiresolver.c
 * Resolve names into actual pointers to actual objects 
 *
 * Linas Vepstas September 2003
 */

#include <glib.h>
#include "duifield.h"
#include "duiresolver.h"

/* =================================================================== */

DuiResolver *
dui_resolver_new (void)
{
	DuiResolver *res;
	res = g_new0 (DuiResolver,1);
	res->field_list = NULL;
	return res;
}

void 
dui_resolver_destroy (DuiResolver *res)
{
	g_list_free (res->field_list);
}

void 
dui_resolver_add_field (DuiResolver *res , DuiField *fld)
{
	if (!res) return;
	res->field_list = g_list_prepend (res->field_list, fld);
}

/* ======================= END OF FILE =============================== */
