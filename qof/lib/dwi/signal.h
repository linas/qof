/********************************************************************\
 * signal.h -- Handle a gtk signal                                  *
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
 * Handle a gtk signal
 *
 * HISTORY:
 * Linas Vepstas March 2002
 */

#ifndef DUI_SIGNAL_H
#define DUI_SIGNAL_H

#include "interface.h"

DuiSignal * dui_signal_new (const char * objname, 
                             const char * signalname, const char *act);
void dui_signal_destroy (DuiSignal *sig);

const char * dui_signal_get_widgetname (DuiSignal *sig);

void dui_signal_connect (DuiSignal *sig, GObject *obj, 
                    GCallback sigfunc, gpointer user_data);

void dui_signal_connect_in_window (DuiSignal *sig, DuiWindow *win,
                                    GCallback sigfunc, gpointer user_data);

void dui_signal_do_realize (DuiSignal *sig, DuiWindow *);

#endif /* DUI_SIGNAL_H */
