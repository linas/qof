/********************************************************************\
 * fieldmap.c -- Map one source to target, with a filter in the middle
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
 * Map one source to target, with a filter in the middle
 *
 * HISTORY:
 * Linas Vepstas Sept 2003
 */

#include "config.h"

#include <glib.h>

#include "perr.h"
#include "duifield.h"
#include "duifieldmap.h"
#include "duifilter.h"

/* =================================================================== */

DuiFieldMap *
dui_field_map_new (void)
{
	DuiFieldMap *fm;
	fm = g_new0 (DuiFieldMap, 1);
	fm->source.type = DUI_FIELD_NONE;
	fm->target.type = DUI_FIELD_NONE;
	fm->filtername = NULL;
	fm->filter = NULL;
	return fm;
}

void
dui_field_map_destroy (DuiFieldMap *fm)
{
	if (!fm) return;
	dui_field_clear (&fm->source);
	dui_field_clear (&fm->target);

	g_free (fm->filtername);
	g_free (fm);
}

/* =================================================================== */
/* Transfer from source to target */

void
dui_field_map_transfer_data (DuiFieldMap *fm)
{
	DuiField *src, *tgt;
	const char * fieldval = NULL;

	if (!fm) return;

	src = &fm->source;
	tgt = &fm->target;

	if (NULL == tgt->set_field_value) return;
	if (src->get_field_value)
	{
		fieldval = src->get_field_value (src);
	}

	/* Note that the empty string is a valid value for "clearing" 
	 * fields. i.e. its used to reset fields to empty. */
	if (fm->filter) fieldval = dui_filter_apply (fm->filter, fieldval);
	if (!fieldval) fieldval = "";
	PINFO ("src=%s::\"%s\" val=\"%s\" tgt=%s::\"%s\"", 
	       src->type, src->fieldname, fieldval, 
	       tgt->type, tgt->fieldname);
	tgt->set_field_value (tgt, fieldval);
}

/* =================================================================== */
/* Transfer from source to target */

const char *
dui_field_map_get_value (DuiFieldMap *fm)
{
	DuiField *src;
	const char * fieldval = NULL;

	if (!fm) return NULL;
	src = &fm->source;

	if (src->get_field_value)
	{
		fieldval = src->get_field_value (src);
	}

	/* Note that the empty string is a valid value for "clearing" 
	 * fields. i.e. its used to reset fields to empty. */
	PINFO ("src=%s:\"%s\" val=\"%s\"", src->type, src->fieldname, fieldval);

	if (fm->filter) fieldval = dui_filter_apply (fm->filter, fieldval);
	return fieldval;
}

/* =================================================================== */
/* Resolve filters */

void
dui_field_map_resolve (DuiFieldMap *fm)
{
	if (!fm) return;

	if (fm->filtername)
	{
		fm->filter = dui_filter_find_by_name (fm->filtername);
	}
	else
	{
		fm->filter = NULL;
	}
}

/* ========================== END OF FILE ============================ */
