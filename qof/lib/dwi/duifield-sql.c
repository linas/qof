/********************************************************************\
 * duifield-sql.c -- Implementation of the SQL Field type           *
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
 * Implement the SQL field type.
 *
 * HISTORY:
 * Linas Vepstas September 2003
 */

#include "config.h"

#include <glib.h>
#include <string.h>

#include "dui-initdb.h"
#include "duifield.h"
#include "duifield-sql.h"
#include "duiresolver.h"
#include "perr.h"

/* Data is in a named database field. */
struct sql_field_s
{
	DuiDBRecordSet *recordset;
};

/* Data is compared to a database field. */
struct where_field_s
{
	char * compareop;
};

/* =================================================================== */

#define UPDATE(f) ((struct sql_field_s *)(&((f)->u.priv)))
#define WHERE(f) ((struct where_field_s *)(&((f)->u.priv)))

static void
update_field_clear (DuiField *f)
{
	g_free (f->fieldname);
	f->type = DUI_FIELD_NONE;
}

static void
where_field_clear (DuiField *f)
{
	g_free (f->fieldname);
	g_free (WHERE(f)->compareop);
	f->type = DUI_FIELD_NONE;
}

/* =================================================================== */

static const char * 
get_sql_value (DuiField *fs)
{
	const char * fieldval = NULL;
	fieldval = dui_recordset_get_value (UPDATE(fs)->recordset, 
	                                    fs->fieldname);
	return fieldval;
}

void
dui_field_set_sql (DuiField *ft, const char * fieldname)
{
	ft->type = DUI_FIELD_SQL;
	ft->get_field_value = get_sql_value;
	ft->set_field_value = NULL;
	ft->clear_field = update_field_clear;

	ft->fieldname = g_strdup (fieldname);
	UPDATE(ft)->recordset = NULL;
}

void
dui_field_set_where (DuiField *ft, const char * fieldname,
                                   const char * compareop)
{
	ft->type = DUI_FIELD_WHERE;
	ft->get_field_value = NULL;
	ft->set_field_value = NULL;
	ft->clear_field = where_field_clear;

	ft->fieldname = g_strdup (fieldname);
	WHERE(ft)->compareop = g_strdup (compareop);
}

const char * 
dui_field_where_get_op (DuiField *ft)
{
	if (!ft) return NULL;
	if (DUI_FIELD_WHERE != ft->type) return NULL;
	return WHERE(ft)->compareop;
}

/* =================================================================== */

void 
dui_field_resolve_recordset (DuiField *fs, DuiDBRecordSet *rset)
{
	if (!DUI_FIELD_IS_TYPE(fs,DUI_FIELD_SQL)) return;
	UPDATE(fs)->recordset = rset;
}

/* ========================== END OF FILE ============================ */
