/********************************************************************\
 * duifilter.h -- Provide a set of data filters                     *
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
 * Provide a set of data filters that turn strings of one 
 * type into strings of another.
 *
 * HISTORY:
 * Linas Vepstas March 2002
 */

#ifndef DUI_FILTER_H
#define DUI_FILTER_H

#include "interface.h"

DuiFilter * dui_filter_new (const char *name, const char *revname);

/* add to the lookup table */
void dui_filter_add_lookup (DuiFilter *, const char * key, const char * value);

DuiFilter * dui_filter_find_by_name (const char *);

/* apply the indicated filter to the string, returning the 
 * result string */
const char * dui_filter_apply (DuiFilter *, const char *);

/* return NULL if the field is whitespace (blank, tab, formfeed etc.)  */
const char * whitespace_filter (const char * val);

/** Return integer 1 if the string starts with 't' or 'T" or contians the
 * word 'true' or 'TRUE';if string is a number, return that number.
 */
int dui_util_bool_to_int (const char * val);

#endif /* DUI_FILTER_H */
