/********************************************************************\
 * duifield-qof.h -- Implementation of the QOF Entity field         *
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

/**
 * @brief: Implement the field for QOF Entities
 * @author: Copyright (c) 2004 Linas Vepstas <linas@linas.org>
 */

#ifndef DUI_FIELD_QOF_H
#define DUI_FIELD_QOF_H

#include "config.h"

#include <qof/qof.h>

#include "duiresolver.h"
#include "duifield.h"
#include "interface.h"

/** 
 * qof_obj_type is the QOF type of the object. 
 * prop is a param on a QOF Object.
 * Note that this is not enough to specify a specific instance/entity.
 */
void dui_field_set_qof (DuiField *ft,
                        const char * qof_obj_type,
                        const char * prop);

/** Used to resolve a particular instance. 
 *  This will attempt to find the instance whose property 'prop'
 *  has the indicated value.
 */
void dui_field_set_qof_match (DuiField *ft,
                              const char * qof_obj_type,
                              const char * prop);

void dui_resolver_resolve_qof (DuiResolver *res , QofBook *);

#endif /* DUI_FIELD_QOF_H */
