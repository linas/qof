/********************************************************************\
 * duifield-sql.h -- Field API for the SQL field type               *
 * Copyright (C) 2004 Linas Vepstas <linas@linas.org>               *
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
/** @addtogroup Field-SQL
 *  @{ */
/**
 * @file: duifield-sql.h
 * @author: Copyright (c) 2003 Linas Vepstas <linas@linas.org>
 * @brief: Implement the field for the SQL type.
 */

#ifndef DUI_FIELD_SQL_H
#define DUI_FIELD_SQL_H

#include "config.h"

#include "dui-initdb.h"

/** Data is from an sql table column */
void dui_field_set_sql (DuiField *ft, const char * fieldname);

/** Used for matching an sql field */
void dui_field_set_where (DuiField *ft, const char * fieldname,
                                        const char * compareop);

/** Return the compare op from an SQL Where field */
const char * dui_field_where_get_op (DuiField *);

/* ---------------------------------------------- */

void dui_field_resolve_recordset (DuiField *fs, DuiDBRecordSet *recs);

#endif /* DUI_FIELD_SQL_H */
/** @} */
