/********************************************************************\
 * interface.h --  Define the top-most DWI application structure.   *
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
 * Define the top-most DWI application structure.
 *
 * HISTORY:
 * Linas Vepstas March 2002
 */

#ifndef DUI_INTERFACE_H
#define DUI_INTERFACE_H

#include <glib-object.h>

#ifdef HAVE_QOF
#include <qof/qof.h>
#endif /* HAVE_QOF */

typedef struct DuiAction_s    DuiAction;
typedef struct DuiDatabase_s  DuiDatabase;
typedef struct DuiFilter_s    DuiFilter;
typedef struct DuiInterface_s DuiInterface;
typedef struct DuiReport_s    DuiReport;
typedef struct DuiSignal_s    DuiSignal;
typedef struct DuiWindow_s    DuiWindow;

DuiInterface * dui_interface_new (void);
void dui_interface_destroy (DuiInterface *dui);

void dui_interfaced_set_name (DuiInterface *dui, const char * name);

/* This call will start making gui calls, and must be called only
 * after glade and gtk have been initialized.  The second argument,
 * "fatal_if_no_main", indicates whether or not its a fatal error
 * if no main window was found.
 */
void dui_interface_realize (DuiInterface *, int fatal_if_no_main);

/* Scan the list of reports in this interface, and return the one with the 
 * indicated name 
 */
DuiAction * dui_interface_find_action_by_name (DuiInterface *, const char * name);
DuiDatabase * dui_interface_find_database_by_name (DuiInterface *, const char * name);
DuiReport * dui_interface_find_report_by_name (DuiInterface *, const char * name);
DuiWindow * dui_interface_find_window_by_name (DuiInterface *, const char * name);

/* Insert a key-value pair into a hash table associated with this
 * interface. */
void dui_interface_kvp_insert (DuiInterface *, const char * key, 
                                             const char * value);

/* Fetch the value assoicated with the key in the hash table */
const char * dui_interface_kvp_lookup (DuiInterface *, const char * key);


void dui_interface_add_action (DuiInterface *, DuiAction *);
void dui_interface_add_database (DuiInterface *, DuiDatabase *);
void dui_interface_add_report (DuiInterface *, DuiReport *);
void dui_interface_add_window (DuiInterface *, DuiWindow *);

void dui_interface_add_object (DuiInterface *, const char * instance_name, GObject *);
GObject * dui_interface_find_object_by_name (DuiInterface *dui, const char *instance_name);

#ifdef HAVE_QOF
QofEntity *dui_interface_find_entity_by_name (DuiInterface *dui, const char *instance_name);
#endif /* HAVE_QOF */
		  

#endif /* DUI_INTERFACE_H */

