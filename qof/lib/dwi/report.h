/********************************************************************\
 * report.h --  Fill in widget values based on database query       *
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
/** @addtogroup Report
 *  @{ */
/**
 * @file report.h
 * @brief Fill in widget values based on results of database query
 * @author created by Linas Vepstas March 2002
 * @author Copyright (C) 2002,2004 Linas Vepstas <linas@linas.org> 
 */

#ifndef DUI_REPORT_H
#define DUI_REPORT_H

#include <qof/qof.h>

#include "dui-initdb.h"
#include "duifieldmap.h"
#include "duitxnreport.h"
#include "interface.h"

DuiReport * dui_report_new (const char * name);
void dui_report_destroy (DuiReport *);

/** Tell the report about the window that contains widgets */
void dui_report_set_window (DuiReport *, DuiWindow *);

/** Tell the report about where it should find 'globals' 
 *  such as the kvp tree, gobjects, etc. 
 */
void dui_report_set_interface (DuiReport *, DuiInterface *);

/** Tell the report about where it should find QOF
 *  object instances.
 */
void dui_report_set_book (DuiReport *, QofBook *);

/** Add a row to the report.  The row will typically hold a 
 *  table widget, or possibly an object class name.  If the 
 *  row holds a table widget (e.g. gtklist, gtkctree, etc.), 
 *  then the rows of the table will be filled in from the 
 *  result of the query.  If the row holds an object class, 
 *  then the report will find the matching instances of that
 *  class, and fill out those instances.
 */
void dui_report_add_row (DuiReport *, DuiTxnReport *row);

/** Add a report 'column'. The fieldmap target should probably contain
 * a widget into which data will be copied, possibly specifying a 
 * column of a widget.   The fieldmap source is probably going to 
 * be some SQL table column.
 */
void dui_report_add_term (DuiReport *rpt, DuiFieldMap *fm); 

/** Resolve stuff; the book/window/interface must be set before
 *  calling this. The report must be realized before it can be run.
 */
void dui_report_do_realize (DuiReport *);

/** Return the name of the indicated report */
const char * dui_report_get_name (DuiReport *);

/** Pull data off the indicated connection and display it. 
 * Returns a non-zero value if there was data and it was 
 * displayed.  If there was no data, then no GUI element
 * was displayed, and this retruns zero.
 */
int dui_report_show_data (DuiReport *, DuiDBRecordSet *);

/** Set the most recent action that generated data for this window.
 * If this window need to be refreshed, then this action will be
 * used to refresh it. 
 */
void dui_report_set_last_action (DuiReport *, DuiAction *);

/** Rerun it again, using old settings. */
void dui_report_refresh (DuiReport *);

/** @} */
#endif /* DUI_REPORT_H */
