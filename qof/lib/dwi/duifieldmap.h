/********************************************************************\
 * duifieldmap.h -- Map one source to target, with a filter in the middle
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
/** @file duifieldmap.h
 *  @brief Map one source to target, with a filter in the middle
 *  @author Copyright (C) 2003 Linas Vepstas <linas@linas.org>
 */
/** @addtogoup Field
 *  Map one source to target, with a filter in the middle
 *
 *  The following struct defines how widget or database fields 
 *  (read as input) are mapped to database or widget fields 
 *  @{
 */

#ifndef DUI_FIELDMAP_H
#define DUI_FIELDMAP_H
#include "config.h"

#include "duifield.h"
#include "duifilter.h"

typedef struct DuiFieldMap_s DuiFieldMap;

struct DuiFieldMap_s 
{
	/** The source of the data, where its coming from */
	DuiField source;

	/** Filters: how we plan to process the data after getting it, 
	 *  but before using it. */
	char * filtername;  
	DuiFilter *filter;

	/** The target of the data, where its going to go to. */
	DuiField target;
};

DuiFieldMap * dui_field_map_new (void);
void dui_field_map_destroy (DuiFieldMap *fm);

/** Transfer from source to target */
void dui_field_map_transfer_data (DuiFieldMap *fm);

/** Get a value, filter it */
const char * dui_field_map_get_value (DuiFieldMap *fm);

/** Get handles to the actual filters */
void dui_field_map_resolve (DuiFieldMap *fm);

#endif /* DUI_FIELDMAP_H */
/** @} */
