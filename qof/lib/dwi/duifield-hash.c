/********************************************************************\
 * duifield-hash.c -- Field for the DUI-Interface global hash table.*
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
 *
 * FUNCTION:
 * Implement the field for the DUI-Interface global hash table.
 *
 * HISTORY:
 * Linas Vepstas September 2003
 */

#include "config.h"

#include <string.h>

#include <glib.h>

#include "duifield.h"
#include "duiresolver.h"
#include "interface.h"
#include "perr.h"

struct hash_s
{
	DuiInterface *hash_table_root;
};

/* =================================================================== */

#define HASH(f) ((struct hash_s *)(&((f)->u.priv)))

static void
hash_field_clear (DuiField *f)
{
	g_free (f->fieldname);
	f->type = DUI_FIELD_NONE;
}

/* =================================================================== */

static const char * 
get_hash_key_value (DuiField *fs)
{
	const char * fieldval = NULL;

	fieldval = dui_interface_kvp_lookup (HASH(fs)->hash_table_root, 
	                fs->fieldname);
	return fieldval;
}

static void 
set_hash_key_value (DuiField *fs, const char *val)
{
	dui_interface_kvp_insert (HASH(fs)->hash_table_root, 
	                fs->fieldname, val);
}

/* =================================================================== */

void 
dui_field_set_hash_key (DuiField *fs, const char * key)
{
  	fs->type = DUI_FIELD_HASH_KEY;
	fs->fieldname = g_strdup (key);

	fs->get_field_value = get_hash_key_value;
	fs->set_field_value = set_hash_key_value;
	fs->clear_field = hash_field_clear;
}

/* =================================================================== */

void 
dui_resolver_resolve_hash (DuiResolver *res , DuiInterface *rot)
{
	GList *node;

	for (node = res->field_list; node; node=node->next)
	{
		DuiField *fld = node->data;
		if (!DUI_FIELD_IS_TYPE(fld,DUI_FIELD_HASH_KEY)) continue;
		HASH(fld)->hash_table_root = rot;
	}
}

/* ======================= END OF FILE =============================== */
