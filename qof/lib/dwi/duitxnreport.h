/********************************************************************\
 * duitxnreport.h -- copy from recordset to field targets.          *
 * Copyright (C) 2002, 2003 Linas Vepstas <linas@linas.org>         *
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
 * @addtogroup TxnReport
 * A 'txnreport' will fill in a bunch of field values, based on 
 * a previously obtained collection of SQL records (i.e. the results
 * of a previous SQL query.)  This is typically used to fill in fields
 * that are in gtk widgets, and/or possibly a gtk table widget.
 * The txnreport can handle multiple table rows in the latter case.
 *
 * The set of db records should probably be obtained using DuiTxnQuery,
 * although this is not a requirement (yet?).
 *
 * Another way to think of this thing as being a grouping of fieldmaps,
 * a 'table' as it were.  Data is copied from the SQL query into this
 * grouping as a unit.
XXXX
except that this is wrong.  we have two competeing concepts:
the set of data that came in from sql, and the way that it will be 
mapped to the targets.  We need a distinct concept of target tables
with matchers.

but this is the matcher.
We need to declare the matching terms here, not else where.


 *
 * @{
 * @file: duitxnreport.h
 * @author Copyright (c) 2002, 2003 Linas Vepstas <linas@linas.org>
 */

#ifndef DUI_TXN_REPORT_H_
#define DUI_TXN_REPORT_H_

#include "config.h"

#include "dui-initdb.h"
#include "duifield.h"
#include "duifieldmap.h"
#include "duiresolver.h"
#include "duiresolver.h"

typedef struct DuiTxnReport_s DuiTxnReport;

DuiTxnReport * dui_txnreport_new (const char * name, int nest);
void dui_txnreport_destroy (DuiTxnReport *row);

const char * dui_txnreport_get_name (DuiTxnReport *);
int dui_txnreport_get_nest (DuiTxnReport *);

/** Add a 'column' to this 'row'.  The fieldmap acts to specify
 *  a data target when the report is run. 
 */
void dui_txnreport_add_term (DuiTxnReport *row, DuiFieldMap *fm);

/** Specify a 'matcher' term which must be satisfied for a row
 * to be filled out.   The matcher will typically indicate a 
 * row in a table widget, or a specific instance of an object.
 */
void dui_txnreport_add_match_term (DuiTxnReport *row, DuiFieldMap *fm);

void dui_txnreport_set_resolver (DuiTxnReport * row, DuiResolver *res);

/** Return TRUE if no data transfer would result, for instance
 *  because there's no data in the recordset */
gboolean dui_txnreport_is_empty (DuiTxnReport * row, DuiDBRecordSet *recs);

/** Force the data transer to occur */
void dui_txnreport_run (DuiTxnReport * row, DuiDBRecordSet *recs);

/* @} */
#endif /* DUI_TXN_REPORT_H_ */
