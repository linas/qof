/********************************************************************\
 * window.h -- manage a graphical GUI window                        *
 * Copyright (C) 2002 Linas Vepstas <linas@linas.org>               *
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
 * Manage a data window on the screen.  This window may be
 * and 'input' or an 'output window (or both): acepting user input,
 * windowing results of queries.
 *
 * HISTORY:
 * Linas Vepstas March 2002
 */

#ifndef DUI_WINDOW_H
#define DUI_WINDOW_H

#include <gtk/gtkwidget.h>

#include "interface.h"

DuiWindow * dui_window_new (const char *name, DuiInterface *);
void dui_window_destroy (DuiWindow *);

const char * dui_window_get_name (DuiWindow *);
DuiInterface *dui_window_get_interface (DuiWindow *);

/* This records the top-level widget for this window.
 * its typically a top-level window 
 * If is_app_main_window is set, then this window will be automatially
 * created on application startup.
 */
void dui_window_set_glade (DuiWindow *, const char * glade_xml_filepath, 
                           const char *root_widget, int is_app_main_window);


/* These routines indicate that a report and/or form 'live' in this window */
void dui_window_add_action (DuiWindow *, DuiAction *);
void dui_window_add_report (DuiWindow *, DuiReport *);
void dui_window_add_signal (DuiWindow *, DuiSignal *);

/* Thin wrapper around glade_xml_get_widget() 
 * Gets the widget associated with the indicated name.
 */
GtkWidget * dui_window_get_widget (DuiWindow *, const char * widgetname);

/* resolve dangling references */
void dui_window_resolve (DuiWindow *);

/* Cause this window to actually exist on the screen */
void dui_window_realize (DuiWindow *);

/* Return non-zero value if the widget for this window exists */
int  dui_window_is_realized (DuiWindow *);

int dui_window_is_app_main_window (DuiWindow *);

/* Force the database used for queries from this window to be connected to.
 * This will cause any database-connection error messages to be displayed.
 * XXX this is kind-of a hack.  We should have a more formal error reporting 
 * chain.
 */
void dui_window_db_connect (DuiWindow *);

#endif /* DUI_WINDOW_H */
