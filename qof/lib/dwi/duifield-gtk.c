/********************************************************************\
 * duifield-gtk.c -- Implements the gtk type of the field.          *
 * Copyright (C) 2002,2003 Linas Vepstas <linas@linas.org>          *
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
 * Implements the gtk type of the field.
 *
 * HISTORY:
 * Copyright (c) 2002, 2003 Linas Vepstas <linas@linas.org> 
 */

#include "config.h"

#include <string.h>

#include <glib.h>
#include <gnome.h>
#include <gtk/gtk.h>

#include "duifield.h"
#include "duifield-gtk.h"
#include "duifilter.h"
#include "duiresolver.h"
#include "perr.h"
#include "util.h"
#include "window.h"

/* A Gtk-1.2 type widget from which we will read data */
struct gtk_widget_s 
{
	GtkWidget * widget; 
	int column;
	int row;

	/* effing ctree not directly addressable by row number,
	 * so cache the node instead */
	GtkCTreeNode *ctn;
};

/* A Gtk-1.2 type widget which has data hanging from it. */
struct gtk_widata_s 
{
	GtkWidget * widget; 
	int column;
	int row;
	char * datakey;  /* key that's attached to gtk_object_data */

	/* effing ctree not directly addressible by row number,
	 * so cache the node instead */
	GtkCTreeNode *ctn;
};

/* A Gtk-1.2 type widget which has GtkArg hanging from it. */
struct gtk_widarg_s 
{
	GtkWidget * widget; 
	int column;
	char * arg;  /* key that's attached to GtkArg */
};

/* A Gtk-1.2 type widget which is used for comparison */
struct gtk_widwhere_s 
{
	char * match_value;
	GtkWidget * widget; 
	int row;
	int column;
	char * compareop;

	/* misc scratch space used by clist iterator */
	char **blank_row;

	/* effing ctree not directly addressable by row number,
	 * so cache the node instead */
	GtkCTreeNode *ctn;
};

/* =================================================================== */

#define WIDGET(f) ((struct gtk_widget_s *)(&((f)->u.priv)))
#define WID_WHERE(f) ((struct gtk_widwhere_s *)(&((f)->u.priv)))
#define WID_DATA(f) ((struct gtk_widata_s *)(&((f)->u.priv)))
#define WID_ARG(f) ((struct gtk_widarg_s *)(&((f)->u.priv)))

static void
widget_field_clear (DuiField *f)
{
	g_free (f->fieldname);
	f->type = DUI_FIELD_NONE;
}

static void
widata_field_clear (DuiField *f)
{
	g_free (WID_DATA(f)->datakey);
	g_free (f->fieldname);
	f->type = DUI_FIELD_NONE;
}

static void
widarg_field_clear (DuiField *f)
{
	g_free (WID_ARG(f)->arg);
	g_free (f->fieldname);
	f->type = DUI_FIELD_NONE;
}

static void
where_field_clear (DuiField *f)
{
	g_free (WID_WHERE(f)->compareop);
	g_free (WID_WHERE(f)->blank_row);

	g_free (f->fieldname);
	f->type = DUI_FIELD_NONE;
}

/* =================================================================== */

static const char *
widget_get_value (DuiField *fs)
{
	GtkWidget *widget;
	const char * val = NULL;
	struct gtk_widget_s *gw;

	/* Hmm, this condition should never happen */
 	if (DUI_FIELD_WIDGET != fs->type) return NULL;

	gw = WIDGET(fs);
	widget = gw->widget;
	if (!widget) return NULL;   /* this shouldn't happen, really */

	/* Supported GTK-1.2 widgets, in pseudo-alpha order;
	 * Gotta handle ctree first */
	if (GTK_IS_CTREE(widget))
	{
		if (NULL == gw->ctn) return NULL;
		gtk_ctree_node_get_text (GTK_CTREE(widget), gw->ctn, gw->column, (gchar **) &val);
	}
	else
	if (GTK_IS_CLIST(widget))
	{
		if (0 > gw->row) return NULL;
		gtk_clist_get_text (GTK_CLIST(widget), gw->row, gw->column, (gchar **) &val);
	}
	else
	if (GTK_IS_MENU(widget))
	{
		static char menuval[18];  /* hack alert not thread safe */
		GList *node; 
		int i=0;
		GtkWidget *w = gtk_menu_get_active (GTK_MENU(widget));
		for (node=GTK_MENU_SHELL(widget)->children; node; node=node->next)
		{
			if (w == node->data)
			{
				snprintf (menuval, 18, "%d", i);
				return menuval;		  
			}
			i++;
		}
	}
	else
	if (GTK_IS_OPTION_MENU(widget))
	{
		DuiField dfs;
		dfs = *fs;
		WIDGET(&dfs)->widget = GTK_OPTION_MENU(widget)->menu;

		/* pass-through on menu, let the child report */
		val = widget_get_value (&dfs);
	}
	else
	if (GTK_IS_RADIO_BUTTON(widget))
	{
		GSList *node;

		/* loop over buttons in group, get data for the one that was pushed */
		node = gtk_radio_button_get_group (GTK_RADIO_BUTTON (widget));
		for ( ; node; node=node->next)
		{
			gboolean pushed;
			pushed = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(node->data));
			if (pushed) break;
		}
		if (!node) return NULL;
		val = gtk_object_get_data (GTK_OBJECT(node->data), "/system/data");
	}
	else
	if (GTK_IS_TOGGLE_BUTTON(widget))
	{
		static char togbuff[2];  /* XXX hack alert not thread safe */
		gboolean state;
	  	state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(widget));
		if (state) togbuff[0] = '1'; else togbuff[0] = '0';
		togbuff[1] = 0;  
		val = togbuff;
	}
	else
	if (GTK_IS_SPIN_BUTTON(widget))
	{
		static char spinbuff[40];    /* hack alert not thread safe */
	  	gdouble dbl = gtk_spin_button_get_value (GTK_SPIN_BUTTON(widget));
		snprintf (spinbuff, 40, "%24.18g", dbl);
		return spinbuff;
	}
	else
	if (GTK_IS_ENTRY(widget))
	{
	  	val = gtk_entry_get_text (GTK_ENTRY(widget));
	}
	else
	if (GTK_IS_LABEL(widget))
	{
	  	gtk_label_get (GTK_LABEL(widget), (gchar **) &val);
	}
	else
	if (GTK_IS_TEXT_VIEW(widget))
	{
	  	val = xxxgtk_textview_get_text (GTK_TEXT_VIEW(widget));
	}
	else
	if (GTK_IS_COMBO(widget))
	{
	  	val = gtk_entry_get_text (GTK_ENTRY(GTK_COMBO(widget)->entry));
	}
	else
	if (GTK_IS_RANGE(widget))
	{
		GtkAdjustment *adj = gtk_range_get_adjustment (GTK_RANGE(widget));
		double flt = adj->value;
		static char rangebuff[40];
		snprintf (rangebuff, 40, "%24.18g", flt);
	  	val = rangebuff;
	}
	else
	if (GNOME_IS_FILE_ENTRY (widget))
	{
		GtkWidget * entry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY(widget));
		val = gtk_entry_get_text (GTK_ENTRY (entry));
	}
	else
	if (GTK_IS_FILE_SELECTION (widget))
	{
		val = gtk_entry_get_text (GTK_ENTRY (GTK_FILE_SELECTION
		                   (widget)->selection_entry));
	}
	else
	if (GNOME_IS_DATE_EDIT(widget))
	{
		static char datebuff[50];
		time_t thyme = gnome_date_edit_get_time (GNOME_DATE_EDIT(widget));
		xxxgnc_secs_to_iso8601_buff (thyme, datebuff);
		return datebuff;
	}
	else
	if (GTK_IS_BIN(widget))
	{
		/* catch-all handles both GtkButton and GtkOptionMenu */
		DuiField dfs;
		dfs = *fs;
		WIDGET(&dfs)->widget = GTK_BIN(widget)->child;

		/* pass-through on bin, let the child report */
		val = widget_get_value (&dfs);
	}
	else
	{
	  	PERR ("unsupported <where> or <update> widget type %s",
			gtk_type_name(GTK_OBJECT_TYPE(widget)));
	}
	return val;
}

/* =================================================================== */

static void
widget_set_value (DuiField *fs, const char * val)
{
	struct gtk_widget_s *gw;
	GtkObject *obj;

	/* Hmm, this condition should never happen */
 	if (DUI_FIELD_WIDGET != fs->type) return;

	gw = WIDGET(fs);
	obj = GTK_OBJECT (gw->widget);
	if (!obj) return;   /* this shouldn't happen, really */

	/* The order in which the classes appear here is IMPORTANT!
	 * Child classes must appear before parents, else inheritance
	 * will cause the wrong if(GTK_IS_XXX(obj)) to be taken!
	 */
	
	if (GTK_IS_TREE_VIEW(obj))
	{
printf ("duude! DWI doesn't support gtk_tree_view because "
        "glade and gtk tree is fundemntaly broken!\n");
printf ("want to set col=%d val=%s\n", gw->column, val);
		GtkTreeModel *tm = gtk_tree_view_get_model (GTK_TREE_VIEW(obj));
printf ("gtk tree model=%p\n", tm);

GtkTreeViewColumn *tvc = gtk_tree_view_get_column(GTK_TREE_VIEW(obj), gw->column);
printf ("gtk tree view column=%p\n", tvc);
		if (NULL == tm) 
		{
			// tm = 
#if 0
// glurg XXX we have to wait for either glade-2 to be fixed, 
// or for gtk_tree_view to be fixed. What an effing disaster!
GtkTreeStore  *treestore;
treestore = gtk_tree_store_new(NUM_COLS,
                                 G_TYPE_STRING,
                                 G_TYPE_STRING,
                                 G_TYPE_UINT);
GtkTreeIter    toplevel;
gtk_tree_store_append(treestore, &toplevel, NULL);
  gtk_tree_store_set(treestore, &toplevel,
                     COL_FIRST_NAME, "Maria",
                     COL_LAST_NAME, "Incognito",
                     -1);
tm = GTK_TREE_MODEL(treestore);
#endif
			return;
		}
	}
	else
	if (GTK_IS_CTREE(obj))
	{
		if (NULL == gw->ctn) return;
		gtk_ctree_node_set_text (GTK_CTREE(obj), gw->ctn,
		                gw->column, val);
	}
	else
	if (GTK_IS_CLIST(obj))
	{
		if (0 > gw->row) return;
		gtk_clist_set_text (GTK_CLIST(obj), gw->row, 
		                                    gw->column, val);
	}
	else
	if (GTK_IS_SPIN_BUTTON (obj))
	{
		gfloat flt;
		flt = atof (val);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON(obj), flt);
	}
	else
	if (GTK_IS_ENTRY (obj))
	{
		gtk_entry_set_text (GTK_ENTRY(obj), val);
	}
	else
	if (GTK_IS_LABEL (obj))
	{
		gtk_label_set_text (GTK_LABEL(obj), val);
	}
	else
	if (GTK_IS_TEXT_VIEW (obj))
	{
		xxxgtk_textview_set_text (GTK_TEXT_VIEW(obj), val);
	}
	else
	if (GTK_IS_COMBO (obj))
	{
		gtk_entry_set_text (GTK_ENTRY(GTK_COMBO (obj)->entry), val);
	}
	else
	if (GTK_IS_RANGE (obj))
	{
		gfloat flt = atof (val);
		GtkAdjustment *adj = gtk_range_get_adjustment (GTK_RANGE(obj));
		gtk_adjustment_set_value (adj, flt);
	}
	else
	if (GTK_IS_MENU_SHELL (obj))
	{
		/* handles menubar, menu, and option menu */
		int i=0;
		int ival = 0;
		GList *node; 
		GtkMenuShell *menu = GTK_MENU_SHELL(obj);
		if (val) ival = atoi (val);
		for (node = menu->children; node; node=node->next)
		{
			if (i == ival)
			{
				gtk_menu_shell_select_item (menu, node->data);
				gtk_menu_shell_activate_item (menu, node->data, 1);
				break;
			}
			i++;
		}
	}
	else
	if (GTK_IS_OPTION_MENU (obj))
	{
		DuiField lf;
		lf = *fs;
		WIDGET(&lf)->widget = GTK_WIDGET(GTK_OPTION_MENU(obj)->menu);
		widget_set_value (&lf, val);
	}
	else
	if (GTK_IS_RADIO_BUTTON (obj))
	{
		GSList *node;
		const char *data = gtk_object_get_data (obj, "/system/data");
							 
		/* If a value hasn't been set, set it now */
		if (!data)
		{
			gtk_object_set_data (obj, "/system/data", (gpointer) val);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(obj), 1);
		}
		else
		{
			/* Find the button with the matching data an push it */
			node = gtk_radio_button_group (GTK_RADIO_BUTTON (obj));
			for ( ; node; node=node->next)
			{
				data = gtk_object_get_data (GTK_OBJECT(node->data), 
			                 "/system/data");
				if (data && !strcmp (data, val)) break;
			}
			if (node)
			{
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(node->data), 1);
			}
		}
	}
	else
	if (GTK_IS_TOGGLE_BUTTON (obj))
	{
		int ival = 0;
		if (val) ival = dui_util_bool_to_int (val);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(obj), ival);
	}
	else
	if (GNOME_IS_FILE_ENTRY (obj))
	{
		GtkWidget * entry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY(obj));
		gtk_entry_set_text (GTK_ENTRY (entry), val);
	}
	else
	if (GTK_IS_FILE_SELECTION (obj))
	{
		gtk_entry_set_text (GTK_ENTRY (GTK_FILE_SELECTION 
		                 (obj)->selection_entry), val);
	}
	else
	if (GTK_IS_FRAME(obj))
	{
		gtk_frame_set_label (GTK_FRAME(obj), val);
	}
	else
	if (GTK_IS_WINDOW(obj))
	{
		gtk_window_set_title (GTK_WINDOW(obj), val);
	}
	else
	if (GTK_IS_BIN (obj))  /* must appear after all other bins */
	{
		DuiField lf;

		/* catch-all for GtkButton, others */
		lf = *fs;
		WIDGET(&lf)->widget = GTK_WIDGET (GTK_BIN(obj)->child);
		widget_set_value (&lf, val);
	}
	else
	if (GNOME_IS_DATE_EDIT (obj))
	{
		/* hack alert -- we assume the database returns iso8601
		 * formatted date strings ... true for postgres, but what
		 * about others ???
		 */
		time_t thyme = xxxgnc_iso8601_to_secs_gmt (val);
		gnome_date_edit_set_time (GNOME_DATE_EDIT(obj), thyme);
	}
	else
	if (GNOME_IS_ABOUT (obj))
	{
		/* no-op */
	}
	else
	{
		SYNTAX ("unsupported report widget \"%s\"\n", 
	     	gtk_type_name(GTK_OBJECT_TYPE(obj)));
	}
}

/* =================================================================== */

static const char *
widget_get_data (DuiField *fs)
{
	GtkWidget *widget;
	const char * val = NULL;
	struct gtk_widata_s *gw;

	/* Hmm, this condition should never happen */
 	if (DUI_FIELD_WID_DATA != fs->type) return NULL;

	gw = WID_DATA(fs);
	widget = gw->widget;
	if (!widget) return NULL;   /* this shouldn't happen, really */

	/* supported widgets, in alpha order */
	if (GTK_IS_CTREE(widget))
	{
		GHashTable *tbl;
		if (NULL == gw->ctn) return NULL;
		tbl = gtk_ctree_node_get_row_data (GTK_CTREE(widget), gw->ctn);
		if (tbl) val = g_hash_table_lookup (tbl, gw->datakey);
	}
	else
	if (GTK_IS_CLIST(widget))
	{
		GHashTable *tbl;
		if (0 > gw->row) return NULL;
		tbl = gtk_clist_get_row_data (GTK_CLIST(widget), gw->row);
		if (tbl) val = g_hash_table_lookup (tbl, gw->datakey);
	}
	else
	if (GTK_IS_RADIO_BUTTON(widget))
	{
		GSList *node;

		/* loop over buttons in group, get data for the one that was pushed */
		node = gtk_radio_button_group (GTK_RADIO_BUTTON (widget));
		for ( ; node; node=node->next)
		{
			gboolean pushed;
			pushed = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(node->data));
			if (pushed) break;
		}
		if (!node) return NULL;
		val = gtk_object_get_data (GTK_OBJECT(node->data), gw->datakey);
	}
	else
	{
		val = gtk_object_get_data (GTK_OBJECT(widget), gw->datakey);
	}
	return val;
}

/* =================================================================== */

static void
widget_set_data (DuiField *fs, const char * val)
{
	GtkWidget *widget;
	struct gtk_widata_s *gw;
	const char * datakey;
	
	/* Hmm, this condition should never happen */
 	if (DUI_FIELD_WID_DATA != fs->type) return;

	gw = WID_DATA(fs);
	widget = gw->widget;
	if (!widget) return;   /* this shouldn't happen, really */

	datakey = gw->datakey;

	if (GTK_IS_CTREE(widget))
	{
		GHashTable *tbl;
		if (NULL == gw->ctn) return;
		tbl = gtk_ctree_node_get_row_data (GTK_CTREE(widget), gw->ctn);
		if (!tbl)
		{
			/* xxx hack alert -- we need to create destructors as well,
			 * otherwise a memory leak */
			tbl = g_hash_table_new (g_str_hash, g_str_equal);

			/* gtk_ctree_set_row_data_full() has destructor */
			gtk_ctree_node_set_row_data (GTK_CTREE(widget), gw->ctn, tbl);
		}
		g_hash_table_insert (tbl, g_strdup(datakey), g_strdup(val));
	}
	else
	if (GTK_IS_CLIST(widget))
	{
		GHashTable *tbl;
		if (0 > gw->row) return;
		tbl = gtk_clist_get_row_data (GTK_CLIST(widget), gw->row);
		if (!tbl)
		{
			/* xxx hack alert -- we need to create destructors as well,
			 * otherwise a memory leak */
			tbl = g_hash_table_new (g_str_hash, g_str_equal);

			/* gtk_clist_set_row_data_full() has destructor */
			gtk_clist_set_row_data (GTK_CLIST(widget), gw->row, tbl);
		}
		g_hash_table_insert (tbl, g_strdup(datakey), g_strdup(val));
	}
#if 0
	else
	if (GTK_IS_RADIO_BUTTON(widget))
	{
		/* XXX uhh, we are supposed to do something special here ... !? */
	}
#endif
	else
	{
		gtk_object_set_data (GTK_OBJECT (widget), datakey, g_strdup(val));
	}
}

/* =================================================================== */

#ifdef BROKEN_IN_GTK_2
XXX fix me later

static void
widget_set_arg (DuiField *fs, const char * val)
{
	GtkObject *obj;
	const char * argname;
	GtkArgInfo *arginfo;
	GtkType obj_type, arg_type;
	int ival = 0;
	double fval = 0.0;
	
	/* Hmm, this condition should never happen */
 	if (DUI_FIELD_WID_ARG != fs->type) return;

	obj = GTK_OBJECT (WID_ARG(fs)->widget);
	if (!obj) return;   /* this shouldn't happen, really */

	argname = WID_ARG(fs)->arg;

	ENTER ("(obj=%p, arg=\'%s\', val=\'%s\')", obj, argname, val);
	obj_type = GTK_OBJECT_TYPE(obj);
	gtk_object_arg_get_info (obj_type, argname, &arginfo);
	arg_type = arginfo->type;
	PINFO ("arg is of type (%d) \'%s\'\n", arg_type, gtk_type_name (arg_type));

	switch (arg_type)
	{
		case GTK_TYPE_INVALID:
		case GTK_TYPE_NONE:
			break;
		case GTK_TYPE_CHAR:
		case GTK_TYPE_UCHAR:
			gtk_object_set (obj, argname, val[0], NULL);
			break;
		case GTK_TYPE_BOOL:
			if (val) ival = atoi(val);
			gtk_object_set (obj, argname, (gboolean) ival, NULL);
			break;
		case GTK_TYPE_INT:
		case GTK_TYPE_UINT:
			if (val) ival = atoi(val);
			gtk_object_set (obj, argname, (int) ival, NULL);
			break;
		case GTK_TYPE_LONG:
		case GTK_TYPE_ULONG:
			if (val) ival = atoi(val);
			gtk_object_set (obj, argname, (long) ival, NULL);
			break;
		case GTK_TYPE_FLOAT:
			if (val) fval = atof(val);
			gtk_object_set (obj, argname, (float) fval, NULL);
			break;
		case GTK_TYPE_DOUBLE:
			if (val) fval = atof(val);
			gtk_object_set (obj, argname, fval, NULL);
			break;
		case GTK_TYPE_STRING:
			gtk_object_set (obj, argname, val, NULL);
			break;
		default:
			break;
	}
}
#endif /* BROKEN_IN_GTK_2 */

/* ============================================================ */
/* iterators for table matching */

static void
ctree_pre (DuiField *fld, gboolean do_clear)
{
	int i, columns;
	struct gtk_widwhere_s *tbl = WID_WHERE (fld);
	// GtkCTree *ctree = GTK_CTREE (tbl->widget);
	GtkCList *clist = GTK_CLIST (tbl->widget);
	
	gtk_clist_freeze (clist);

	/* Don't clear if we're filling in subtrees or matching some rows */
	if (do_clear)
	{
		gtk_clist_clear (clist);
	}

	/* create a blank row -- hack for adding rows */
	columns = clist->columns;
	if (tbl->blank_row) g_free (tbl->blank_row);
	tbl->blank_row = g_new (char *, columns);
	for (i=0; i<columns; i++) tbl->blank_row[i] = "";
}
	
static void
ctree_post (DuiField *fld)
{
	struct gtk_widwhere_s *tbl = WID_WHERE (fld);
	// GtkCTree *ctree = GTK_CTREE (tbl->widget);
	GtkCList *clist = GTK_CLIST (tbl->widget);
	
	gtk_clist_thaw(clist);
	g_free (tbl->blank_row);
	tbl->blank_row = NULL;
}

/* ============================================================ */
/* ctree_iter/ctree_recur will look for the row whose 'match_col' 
 * contains 'match_val'.  Do a breadth-first search, and if its not
 * there, then drill down into the tree.
 */

static GtkCTreeNode *
ctree_recur (GtkCTree *ctree, GtkCTreeNode *start_ctn, 
             int match_col, const char * match_value)
{
	GtkCTreeRow *ctr;
	GtkCTreeNode *ctn;

	ctn = start_ctn;

	/* breadth first search */
	while (ctn)
	{
		char * cval;

		gtk_ctree_node_get_text (ctree, ctn, match_col, &cval);
		if (!strcmp (cval, match_value))
		{
			return ctn;
		}

		ctr = ctn->list.data;
		if (!ctr) break;
		ctn = ctr->sibling;
	}

	/* Now do the depth search */
	ctn = start_ctn;
	while (ctn)
	{
		GtkCTreeNode *child_ctn, *match_ctn;
		ctr = ctn->list.data;
		if (!ctr) break;
		child_ctn = ctr->children;

		match_ctn = ctree_recur (ctree, child_ctn, match_col, match_value);

		if (match_ctn) return match_ctn;

		ctn = ctr->sibling;
	}

	return NULL;
}

static gboolean
ctree_iter (DuiField *fld)
{
	struct gtk_widwhere_s *tbl = WID_WHERE (fld);
	GtkCTree *ctree = GTK_CTREE (tbl->widget);
	GtkCTreeNode *ctn;
	int match_col;

	tbl->ctn = NULL;
	if ((NULL == tbl->match_value) || (0 == tbl->match_value[0]))
	{
		tbl->ctn = gtk_ctree_insert_node (ctree, 
			   NULL, NULL, tbl->blank_row, 0, 
				NULL, NULL, NULL, NULL, FALSE, FALSE);
		return TRUE;
	} 

	match_col = tbl->column;

	ctn = gtk_ctree_node_nth (ctree, 0);
	ctn = ctree_recur (ctree, ctn, match_col, tbl->match_value);
	if (ctn)
	{
		tbl->ctn = gtk_ctree_insert_node (ctree, ctn,
				NULL, tbl->blank_row, 0, 
				NULL, NULL, NULL, NULL, FALSE, FALSE);
		return TRUE;
	}

	SYNTAX ("could not locate a parent row with value %s", tbl->match_value);
	return FALSE;
}
	
/* ============================================================ */

static void
clist_pre (DuiField *fld, gboolean do_clear)
{
	int i, columns;
	struct gtk_widwhere_s *tbl = WID_WHERE (fld);
	GtkCList *clist = GTK_CLIST (tbl->widget);
	
	gtk_clist_freeze (clist);

	/* Don't clear if we're filling in subtrees or matching some rows */
	if (do_clear)
	{
		gtk_clist_clear (clist);
	}

	tbl->row = -1;
	/* create a blank row -- hack for adding rows */
	columns = clist->columns;
	if (tbl->blank_row) g_free (tbl->blank_row);
	tbl->blank_row = g_new (char *, columns);
	for (i=0; i<columns; i++) tbl->blank_row[i] = "";
}
	
static void
clist_post (DuiField *fld)
{
	struct gtk_widwhere_s *tbl = WID_WHERE (fld);
	GtkCList *clist = GTK_CLIST (tbl->widget);
	
	gtk_clist_thaw(clist);
	g_free (tbl->blank_row);
	tbl->blank_row = NULL;
}
	
static gboolean
clist_iter (DuiField *fld)
{
	struct gtk_widwhere_s *tbl = WID_WHERE (fld);
	GtkCList *clist = GTK_CLIST (tbl->widget);
	tbl->row ++;
	gtk_clist_append (clist, tbl->blank_row);
	return TRUE;
}
	
/* =================================================================== */
/* =================================================================== */
/* Table management grunge */

static void 
resolve_target (DuiField *target, DuiField *matcher)
{
	struct gtk_widwhere_s *tbl;

	if (!target || !matcher) return;
	if (!DUI_FIELD_IS_TYPE(matcher,DUI_FIELD_WID_WHERE)) return;
	tbl = WID_WHERE (matcher);

	if (DUI_FIELD_IS_TYPE (target, DUI_FIELD_WIDGET)) 
	{
		if (tbl->widget != WIDGET(target)->widget) return;
		WIDGET(target)->ctn = tbl->ctn;
		WIDGET(target)->row = tbl->row;
		return;
	}
	if (DUI_FIELD_IS_TYPE (target, DUI_FIELD_WID_DATA)) 
	{
		if (tbl->widget != WID_DATA(target)->widget) return;
		WID_DATA(target)->ctn = tbl->ctn;
		WID_DATA(target)->row = tbl->row;
		return;
	}
}

static void
set_match_value (DuiField *fld, const char * val)
{
	struct gtk_widwhere_s *gw = WID_WHERE(fld);
	if (gw->match_value) g_free (gw->match_value);

	if (val)
	{
		gw->match_value = g_strdup (val);
	}
	else
	{
		gw->match_value = g_strdup ("");
	}
}

/* =================================================================== */
/* =================================================================== */
/* =================================================================== */
/* =================================================================== */

void 
dui_field_set_widget (DuiField *fs, const char * widname, int colnum)
{
	struct gtk_widget_s *gw;

	if (!fs || !DUI_FIELD_IS_TYPE(fs,DUI_FIELD_NONE)) return;
  	fs->type = DUI_FIELD_WIDGET;
	fs->fieldname = g_strdup (widname),

	fs->get_field_value = widget_get_value;
	fs->set_field_value = widget_set_value;
	fs->clear_field = widget_field_clear;

	fs->iter_pre = NULL;
	fs->iter_next = NULL;
	fs->iter_column = resolve_target;
	fs->iter_post = NULL;

	gw = WIDGET(fs);
	gw->widget = NULL,
	gw->column = colnum;
	gw->row = -1;
	gw->ctn = NULL;
}

void 
dui_field_set_wid_data (DuiField *fs, const char * widname,
                                      int colnum,
                                      const char * datakey)
{
	struct gtk_widata_s *gw;

	if (!fs || !DUI_FIELD_IS_TYPE(fs,DUI_FIELD_NONE)) return;
  	fs->type = DUI_FIELD_WID_DATA;
	fs->fieldname = g_strdup (widname),

	fs->get_field_value = widget_get_data;
	fs->set_field_value = widget_set_data;
	fs->clear_field = widata_field_clear;

	fs->iter_pre = NULL;
	fs->iter_next = NULL;
	fs->iter_column = resolve_target;
	fs->iter_post = NULL;

	gw = WID_DATA(fs);
	gw->widget = NULL,
	gw->column = colnum;
	gw->row = -1;
	gw->ctn = NULL;
	gw->datakey = g_strdup (datakey);
}

void 
dui_field_set_wid_arg (DuiField *fs, const char * widname,
                                     int colnum,
                                     const char * arg)
{
	struct gtk_widarg_s *gw;

	if (!fs || !DUI_FIELD_IS_TYPE(fs,DUI_FIELD_NONE)) return;
  	fs->type = DUI_FIELD_WID_ARG;
	fs->fieldname = g_strdup (widname),

	fs->get_field_value = NULL;
	// fs->set_field_value = widget_set_arg;   XXX currently broken
	fs->set_field_value = NULL;
	fs->clear_field = widarg_field_clear;

	fs->iter_pre = NULL;
	fs->iter_next = NULL;
	fs->iter_column = resolve_target;
	fs->iter_post = NULL;

	gw = WID_ARG(fs);
	gw->widget = NULL,
	gw->column = colnum;
	gw->arg = g_strdup (arg);
}

void 
dui_field_set_wid_where (DuiField *fs, const char * widname,
                         int colnum, const char * compareop)
{
	struct gtk_widwhere_s *gw;

	if (!fs || !DUI_FIELD_IS_TYPE(fs,DUI_FIELD_NONE)) return;
  	fs->type = DUI_FIELD_WID_WHERE;
	fs->fieldname = g_strdup (widname);

	fs->get_field_value = NULL;
	fs->set_field_value = set_match_value;
	fs->clear_field = where_field_clear;

	fs->iter_pre = NULL;
	fs->iter_next = NULL;
	fs->iter_column = NULL;
	fs->iter_post = NULL;

	gw = WID_WHERE(fs);
	gw->match_value = NULL;
	gw->widget = NULL;
	gw->column = colnum;
	gw->row = -1;
	gw->blank_row = NULL;
	gw->compareop = g_strdup(compareop);
	gw->ctn = NULL;
}

/* =================================================================== */

static void
get_clist_row (GtkCList *clist,
               gint            row,
               gint            column,
               GdkEvent       *event,
               DuiField *fs)
{
	if (DUI_FIELD_IS_TYPE (fs, DUI_FIELD_WIDGET)) {
		WIDGET(fs)->row = row; return;
	} 
	if (DUI_FIELD_IS_TYPE (fs, DUI_FIELD_WID_DATA)) {
		WID_DATA(fs)->row = row; return;
	}
}

static void
unget_clist_row (GtkCList *clist,
               gint            row,
               gint            column,
               GdkEvent       *event,
               DuiField *fs)
{
	if (DUI_FIELD_IS_TYPE (fs, DUI_FIELD_WIDGET)) {
		WIDGET(fs)->row = -1; return;
	} 
	if (DUI_FIELD_IS_TYPE (fs, DUI_FIELD_WID_DATA)) {
		WID_DATA(fs)->row = -1; return;
	}
}

static void
get_ctree_row (GtkCList *clist,
               GtkCTreeNode *ctn,
               gint            column,
               DuiField *fs)
{
	if (DUI_FIELD_IS_TYPE (fs, DUI_FIELD_WIDGET)) {
		WIDGET(fs)->ctn = ctn; return;
	} 
	if (DUI_FIELD_IS_TYPE (fs, DUI_FIELD_WID_DATA)) {
		WID_DATA(fs)->ctn = ctn; return;
	}
}

static void
unget_ctree_row (GtkCList *clist,
               GtkCTreeNode *ctn,
               gint            column,
               DuiField *fs)
{
	if (DUI_FIELD_IS_TYPE (fs, DUI_FIELD_WIDGET)) {
		WIDGET(fs)->ctn = NULL; return;
	} 
	if (DUI_FIELD_IS_TYPE (fs, DUI_FIELD_WID_DATA)) {
		WID_DATA(fs)->ctn = NULL; return;
	}
}

static void 
add_watcher (DuiField *fs, GtkWidget *w)
{
	/* There's no way to ask the clist what row has been selected,
	 * so we have to listen for events instead. */
	if (GTK_IS_CTREE(w))
	{
		gtk_signal_connect(GTK_OBJECT(w), "tree_select_row",
		   	           GTK_SIGNAL_FUNC(get_ctree_row), fs);
		gtk_signal_connect(GTK_OBJECT(w), "tree_unselect_row",
		   	           GTK_SIGNAL_FUNC(unget_ctree_row), fs);
	}
	else
	if (GTK_IS_CLIST (w))
	{
		gtk_signal_connect(GTK_OBJECT(w), "select_row",
		   	           GTK_SIGNAL_FUNC(get_clist_row), fs);
		gtk_signal_connect(GTK_OBJECT(w), "unselect_row",
		   	           GTK_SIGNAL_FUNC(unget_clist_row), fs);
	}
}

/* =================================================================== */
/* Provide sorting by default. Not sure if this is a good thing */

static void 
clist_sort_column (GtkCList *clist, gint column, DuiField *f)
{
	static GtkSortType direction = GTK_SORT_DESCENDING;

	if (GTK_SORT_DESCENDING == direction) direction = GTK_SORT_ASCENDING;
	else direction = GTK_SORT_DESCENDING;

	gtk_clist_freeze (clist);
	gtk_clist_set_sort_type (clist, direction);
	gtk_clist_set_sort_column (clist, column);
	gtk_clist_sort (clist);

	gtk_clist_thaw (clist);
}

static void
add_sorter (DuiField *fld, GtkWidget *w)
{
	/* Currently, the only supported multi-rowed widgets are
	 * ctree and clist */
	if ((GTK_IS_CTREE(w)) || (GTK_IS_CLIST(w)))
	{
		gtk_signal_connect (GTK_OBJECT(w), "click_column", 
		             GTK_SIGNAL_FUNC(clist_sort_column), fld);
	}
}

/* =================================================================== */

static void
resolve_where (DuiField *fld)
{
	struct gtk_widwhere_s *tbl = WID_WHERE (fld);
	GtkWidget *w = tbl->widget;

	/* Currently, the only supported multi-rowed widgets are
	 * ctree and clist */
	if (GTK_IS_CTREE(w))
	{
		fld->iter_pre = ctree_pre;
		fld->iter_post = ctree_post;
		fld->iter_next = ctree_iter;
	}
	else
	if (GTK_IS_CLIST(w))
	{
		fld->iter_pre = clist_pre;
		fld->iter_post = clist_post;
		fld->iter_next = clist_iter;
	}
	else
	{
		fld->iter_pre = NULL;
		fld->iter_post = NULL;
		fld->iter_next = NULL;
	}
}

/* =================================================================== */

static void 
resolve_widget (DuiField *fs, GtkWidget *w)
{
	if (DUI_FIELD_IS_TYPE (fs, DUI_FIELD_WIDGET)) {
		WIDGET(fs)->widget = w; return;
	}
	if (DUI_FIELD_IS_TYPE (fs, DUI_FIELD_WID_DATA)) {
		WID_DATA(fs)->widget = w; return;
	}
	if (DUI_FIELD_IS_TYPE (fs, DUI_FIELD_WID_ARG)) {
		WID_ARG(fs)->widget = w; return;
	}
	if (DUI_FIELD_IS_TYPE (fs, DUI_FIELD_WID_WHERE)) {
		WID_WHERE(fs)->widget = w; 
		resolve_where (fs);
		return;
	}
}

/* =================================================================== */

void 
dui_resolver_resolve_widgets (DuiResolver *res , DuiWindow *win)
{
	GList *node;

	for (node = res->field_list; node; node=node->next)
	{
		DuiField *fld = node->data;
		GtkWidget *w;
		const char * widgetname;

		if (!DUI_FIELD_IS_TYPE (fld, DUI_FIELD_WIDGET) &&
		    !DUI_FIELD_IS_TYPE (fld, DUI_FIELD_WID_DATA) &&
		    !DUI_FIELD_IS_TYPE (fld, DUI_FIELD_WID_ARG) &&
		    !DUI_FIELD_IS_TYPE (fld, DUI_FIELD_WID_WHERE) ) continue;
		widgetname = fld->fieldname;
		if (!widgetname) continue;
		if (0 == widgetname[0]) continue;
		w =  dui_window_get_widget (win, widgetname);
		PINFO ("%s:\"%s\" is at %p", fld->type,widgetname, w);
		resolve_widget (fld, w);
		add_watcher (fld, w);
		add_sorter (fld, w);
	}
}

/* ========================== END OF FILE ============================ */
