/********************************************************************\
 * action.h -- API for handling user interface activities.          *
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
 * @file action.h
 * @brief API for handling user interface activities.    
 * @author Copyright (C) 2002,2004 Linas Vepstas <linas@linas.org> 
 */
/** @addtogroup Action
 * API for handling user interface activities.  "Actions" typically
 * get run as a result of the GUI user clicking on a button or performing
 * some other user interface activity.  An action stores state information
 * about what should happen when the button-click/signal/etc. occurs.
 * 
 * More precisely, actions are triggered by GTK signals issued from GTK
 * widgets.  The action stores the template of an SQL query that will be
 * run when the signal is caught.  Along with this, it stores the widgets
 * that will be queried to fill in the blanks in the SQL statement.
 * 
 * The action also stores a list of reports that need to be refreshed 
 * when the SQL query is completed.
 *
 * Not all actions need to perform SQL queries.  Some (wtokey) cause
 * data to be copied from the user interface widgets to the key-value
 * pair tree.  Others (wtoobj) copy the data to GLib-2.0 GObjects.
 *  @{ 
 */

#ifndef DUI_ACTION_H
#define DUI_ACTION_H

#include <glib.h>
#include <qof/qof.h>

#include "interface.h"
#include "database.h"
#include "duifieldmap.h"
#include "duitxnquery.h"

DuiAction * dui_action_new (const char *name);
void dui_action_destroy (DuiAction *);

const char * dui_action_get_name (DuiAction *);

/** Tell the action about the glade window connection that holds 
 *  the GUI widgets */
void dui_action_set_window (DuiAction * act, DuiWindow *win);

/** Tell the action about where to find the 'global' objects,
 *  such as kvp trees, gobjects etc. 
 */
void dui_action_set_interface (DuiAction * act, DuiInterface *);

/** Tell the action about where it should find QOF
 *  object instances.
 */
void dui_action_set_book (DuiAction *, QofBook *);

/** Set the name of the database connection that will be used to 
 *  perform queries on */
void dui_action_set_database_name (DuiAction * act, const char *dbname);

/** Add a 'row' to the action.  The row will typically hold a
 *  table widget, or possibly an object class name.  If the
 *  row holds a table widget (e.g. gtklist, gtkctree, etc.),
 *  then the rows of the table will be used to formulate the 
 *  SQL query.  If the row holds an object class, then the 
 *  action will find the matching instances of that class, 
 *  and use those instances to formulate the query.
 */
void dui_action_add_row (DuiAction *, DuiTxnQuery *qry);
DuiTxnQuery *dui_action_get_query (DuiAction *);

/** Add a term, which may be an sql select, sql update, sql where matcher,
 * or it might be a non-sql map */
void dui_action_add_term (DuiAction * act, DuiFieldMap *fm);

/** Cross-reference to the reports that will show the results.
 */
void dui_action_set_report (DuiAction *, const char * name);
const char * dui_action_get_report (DuiAction *);

/** Specify a signal that will invoke this action. 
 * The widgetname must specify a GtkWidget on which
 * there exists a signal of name 'signalname'.
 * The widgetname must specify the name of a widget 
 * that appears in the glade file for the window that 
 * this action is a part of.
 */

void dui_action_add_signal (DuiAction *act, const char *widgetname, const char *signalname);


/** Add a window that needs to be refreshed when this action is performed */
void dui_action_add_refresh (DuiAction *, const char * name);

/** Rerun the last query (and display it in a report).
 *  this routine is used to refresh a window.
 */
void dui_action_rerun_last_query (DuiAction *act);


/** Add a chained action */
void dui_action_add_chain (DuiAction *act, const char * actionname);

/** Resolve stuff; the book/window/interface must be set before 
 *  calling this. The action must be realized before it can be run.
 */
void dui_action_do_realize (DuiAction *);

/** Force the db to connect. This will cause any connect
 * errors to be reported.
 */
void dui_action_db_connect (DuiAction *act);
		  

/** Cause the indicated action to be run. */
int dui_action_run (DuiAction *act);
		  
/** @} */
#endif /* DUI_ACTION_H */
