/********************************************************************\
 * duiresolver.h -- Resolve names into actual pointers              *
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

/**
 * @file: duiresolver.h
 * @brief: Resolve names into actual pointers to actual objects 
 * @author: Copyright (c) 2003 Linas Vepstas <linas@linas.org>
 */

#ifndef DUI_RESOLVER_H_
#define DUI_RESOLVER_H_

#include <glib.h>
#include "duifield.h"

typedef struct DuiResolver_s DuiResolver;

struct DuiResolver_s
{
	GList * field_list;
};

DuiResolver * dui_resolver_new (void);
void dui_resolver_destroy (DuiResolver *res);

/** Add a field which will need resolution later */
void dui_resolver_add_field (DuiResolver*, DuiField *);

#endif /* DUI_RESOLVER_H_ */
