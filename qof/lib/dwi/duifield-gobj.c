/********************************************************************\
 * duifield-gobj.c -- Implementation of the glib-2.0 GObject field  *
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
 * Implementation of the glib-2.0 GObject field 
 *
 * HISTORY:
 * Linas Vepstas September 2003
 */

#include "config.h"

#include <glib.h>
#include <stdlib.h>
#include <string.h>

#include "duifield.h"
#include "duifield-gobj.h"
#include "duifilter.h"
#include "duiresolver.h"
#include "interface.h"
#include "perr.h"

/* A property (actually, a parameter) on a glib-2.0 gobject */
struct gobj_field_s
{
	DuiInterface *gobj_root;
	char * property;
	char * cache_value;
};

/* =================================================================== */

#define GOBJ(f) ((struct gobj_field_s *)(&((f)->u.priv)))

static void
gobj_field_clear (DuiField *f)
{
	g_free (f->fieldname);
	g_free (GOBJ(f)->property);
	g_free (GOBJ(f)->cache_value);
	f->type = DUI_FIELD_NONE;
}

/* =================================================================== */

static const char *
get_gobj_value (DuiField *fs)
{
	GObject *gob;

	gob = dui_interface_find_object_by_name(GOBJ(fs)->gobj_root, 
	                fs->fieldname);
	if (!gob)
	{
		PERR ("Can't find object named \'%s\'\n", fs->fieldname);
		return NULL;
	}
									
	GParamSpec *ps = g_object_class_find_property
	             (G_OBJECT_GET_CLASS(gob), GOBJ(fs)->property);
	if (NULL == ps)
	{
		PERR ("Cannot find parameter to get %s:%s\n", 
		              fs->fieldname, GOBJ(fs)->property);
		return NULL;
	}
	if (G_IS_PARAM_SPEC_STRING(ps))
	{
		GValue val = {G_TYPE_INVALID};
		g_value_init (&val, G_TYPE_STRING);
		g_object_get_property (gob, GOBJ(fs)->property, &val);

		const char * str = g_value_get_string (&val);
		return str;
	}

	if (G_IS_PARAM_SPEC_INT(ps))
	{
		GValue val = {G_TYPE_INVALID};
		g_value_init (&val, G_TYPE_INT);
		g_object_get_property (gob, GOBJ(fs)->property, &val);
		int v = g_value_get_int (&val);

		g_free (GOBJ(fs)->cache_value);
		GOBJ(fs)->cache_value = g_strdup_printf ("%d", v);
		return GOBJ(fs)->cache_value;
	}

	if (G_IS_PARAM_SPEC_BOOLEAN(ps))
	{
		GValue val = {G_TYPE_INVALID};
		g_value_init (&val, G_TYPE_BOOLEAN);
		g_object_get_property (gob, GOBJ(fs)->property, &val);
		gboolean v = g_value_get_boolean (&val);

		if (v) 
		{
			return "t";
		}
		return "f";
	}

	if (G_IS_PARAM_SPEC_FLOAT(ps))
	{
		GValue val = {G_TYPE_INVALID};
		g_value_init (&val, G_TYPE_FLOAT);
		g_object_get_property (gob, GOBJ(fs)->property, &val);
		float v = g_value_get_float (&val);

		g_free (GOBJ(fs)->cache_value);
		GOBJ(fs)->cache_value = g_strdup_printf ("%g", v);
		return GOBJ(fs)->cache_value;
	}

	if (G_IS_PARAM_SPEC_DOUBLE(ps))
	{
		GValue val = {G_TYPE_INVALID};
		g_value_init (&val, G_TYPE_DOUBLE);
		g_object_get_property (gob, GOBJ(fs)->property, &val);
		double v = g_value_get_double (&val);

		g_free (GOBJ(fs)->cache_value);
		GOBJ(fs)->cache_value = g_strdup_printf ("%g", v);
		return GOBJ(fs)->cache_value;
	}

	PERR ("unsupported type %s for %s:%s\n", 
	         G_PARAM_SPEC_TYPE_NAME(ps), fs->fieldname, GOBJ(fs)->property);
	return NULL;
}

static void 
set_gobj_value (DuiField *fs, const char *value)
{
	GObject *gob;

	gob = dui_interface_find_object_by_name(GOBJ(fs)->gobj_root, 
	                fs->fieldname);
	if (!gob)
	{
		PERR ("Can't find object named \'%s\'\n", fs->fieldname);
		return;
	}

	GParamSpec *ps = g_object_class_find_property
	             (G_OBJECT_GET_CLASS(gob), GOBJ(fs)->property);
	if (NULL == ps)
	{
		PERR ("Cannot find parameter to set %s:%s\n", 
		              fs->fieldname, GOBJ(fs)->property);
		return;
	}
	if (G_IS_PARAM_SPEC_STRING(ps))
	{
		GValue val = {G_TYPE_INVALID};
		g_value_init (&val, G_TYPE_STRING);
		g_value_set_string (&val, value);

		g_object_set_property (gob, GOBJ(fs)->property, &val);
		return;
	} 
	else
	if (G_IS_PARAM_SPEC_INT(ps))
	{
		GValue val = {G_TYPE_INVALID};
		g_value_init (&val, G_TYPE_INT);
		gint ii = atoi (value);
		g_value_set_int (&val, ii);

		g_object_set_property (gob, GOBJ(fs)->property, &val);
		return;
	}
	else
	if (G_IS_PARAM_SPEC_BOOLEAN(ps))
	{
		GValue val = {G_TYPE_INVALID};
		g_value_init (&val, G_TYPE_BOOLEAN);
		gboolean ii = dui_util_bool_to_int (value);
		g_value_set_boolean (&val, ii);

		g_object_set_property (gob, GOBJ(fs)->property, &val);
		return;
	}
	else
	if (G_IS_PARAM_SPEC_FLOAT(ps))
	{
		GValue val = {G_TYPE_INVALID};
		g_value_init (&val, G_TYPE_FLOAT);
		gfloat ff = atof (value);
		g_value_set_float (&val, ff);

		g_object_set_property (gob, GOBJ(fs)->property, &val);
		return;
	}
	else
	if (G_IS_PARAM_SPEC_DOUBLE(ps))
	{
		GValue val = {G_TYPE_INVALID};
		g_value_init (&val, G_TYPE_DOUBLE);
		gdouble ff = atof (value);
		g_value_set_double (&val, ff);

		g_object_set_property (gob, GOBJ(fs)->property, &val);
		return;
	}
	else
	{
		PERR ("unsupported type %s for %s:%s\n", 
		         G_PARAM_SPEC_TYPE_NAME(ps), fs->fieldname, GOBJ(fs)->property);
		return;
	}
}

void
dui_field_set_gobj (DuiField *ft, const char * obj,
                                  const char * prop)
{
	struct gobj_field_s *gf;

	ft->type = DUI_FIELD_GOBJ;
	ft->fieldname = g_strdup (obj);

	ft->get_field_value = get_gobj_value;
	ft->set_field_value = set_gobj_value;
	ft->clear_field = gobj_field_clear;

	gf = GOBJ(ft);
	gf->property = g_strdup (prop);
	gf->cache_value = NULL;
}

/* =================================================================== */

void 
dui_resolver_resolve_gobj (DuiResolver *res , DuiInterface *rot)
{
	GList *node;

	for (node = res->field_list; node; node=node->next)
	{
		DuiField *fld = node->data;
		if (!DUI_FIELD_IS_TYPE(fld,DUI_FIELD_GOBJ)) continue;

		GOBJ(fld)->gobj_root = rot;
	}
}

/* ========================== END OF FILE ============================ */
