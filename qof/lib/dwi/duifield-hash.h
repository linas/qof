/********************************************************************\
 * duifield-hash.h -- Implement the dwi-global interface hash table *
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

/*
 * FUNCTION:
 * Implement the dwi-global interface hash table for fields
 *
 * HISTORY:
 * Linas Vepstas September 2003
 */

#ifndef DUI_FIELD_HASH_H
#define DUI_FIELD_HASH_H

#include "config.h"
#include "duifield.h"
#include "duiresolver.h"
#include "interface.h"

/** data will be looked up in a hash tree */
void dui_field_set_hash_key (DuiField *fs, 
                             const char * key);

void dui_resolver_resolve_hash (DuiResolver *res , DuiInterface *rot);

#endif /* DUI_FIELD_HASH_H */

