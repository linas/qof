/********************************************************************\
 * database.h -- manage database connection info.                   *
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

/**
 * @file database.h
 * @brief manage database connection info
 * @author Copyright (C) 2002 Linas Vepstas <linas@linas.org> 
 */

#ifndef DUI_DATABASE_H
#define DUI_DATABASE_H

#include <glib-object.h>

#include "dui-initdb.h"
#include "interface.h"

#define DUI_DATABASE_TYPE (dui_database_get_type())
#define DUI_DATABASE(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), DUI_DATABASE_TYPE, DuiDatabase))
#define IS_DUI_DATABASE(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), DUI_DATABASE_TYPE))

GType dui_database_get_type (void);

DuiDatabase * dui_database_new (const char * name,
                                const char * provider, const char * dbname,
                                const char * hostname, const char * username,
										  const char * authentication_token);

void dui_database_destroy (DuiDatabase *);
DuiDBConnection * dui_database_do_realize (DuiDatabase *);
const char * dui_database_get_name (DuiDatabase *);

		  
#endif /* DUI_DATABASE_H */
