/********************************************************************\
 * duifield.c -- Holder (abstract base class) for different field types.
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
 * Holder (abstract base class) for different field types.
 *
 * HISTORY:
 * Linas Vepstas September 2003
 */

#include "config.h"

#include <string.h>

#include <glib.h>

#include "duifield.h"
#include "perr.h"
#include "duiresolver.h"

/* =================================================================== */

void
dui_field_clear (DuiField *f)
{
	if (DUI_FIELD_IS_TYPE(f,DUI_FIELD_NONE)) return;
	if (f->clear_field) { f->clear_field (f); }
	f->clear_field = NULL;
	f->type = DUI_FIELD_NONE;
}

const char *
dui_field_get_fieldname (DuiField *f)
{
	if (!f) return NULL;
	return f->fieldname;
}

/* =================================================================== */

static void 
val_clear (DuiField *f)
{
	g_free (f->u.value);
}

static const char * 
get_const_value (DuiField *fs)
{
	return fs->u.value;
}

void 
dui_field_set_const (DuiField *fs, const char *value)
{
	if (!fs) return;
	if (!value) value = "";
  	fs->type = DUI_FIELD_CONST;
	fs->fieldname = "const";
	fs->u.value = g_strdup (value);
	fs->get_field_value = get_const_value;
	fs->set_field_value = NULL;
	fs->clear_field = val_clear;
}

/* =================================================================== */

void 
dui_field_iter_pre (DuiField *matcher, gboolean do_clear)
{
	if (NULL == matcher) return;
	if (NULL == matcher->iter_pre) return;
	(matcher->iter_pre) (matcher, do_clear);
}

gboolean
dui_field_iter_next (DuiField *matcher)
{
	if (NULL == matcher) return TRUE;
	if (NULL == matcher->iter_next) return TRUE;
	return (matcher->iter_next) (matcher);
}

void 
dui_field_iter_column (DuiField *target, DuiField *matcher)
{
	if (NULL == target) return;
	if (NULL == target->iter_column) return;
	(target->iter_column) (target, matcher);
}

void 
dui_field_iter_post (DuiField *matcher)
{
	if (NULL == matcher) return;
	if (NULL == matcher->iter_post) return;
	(matcher->iter_post) (matcher);
}

/* ========================== END OF FILE ============================ */
