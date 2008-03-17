/********************************************************************\
 * kvputil-p.h -- misc odd-job kvp utils (private file)             *
 *                                                                  *
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 51 Franklin Street, Fifth Floor    Fax:    +1-617-542-2652       *
 * Boston, MA  02110-1301,  USA       gnu@gnu.org                   *
 *                                                                  *
\********************************************************************/

#ifndef KVPUTIL_P_H
#define KVPUTIL_P_H

#include "guid.h"
#include "kvpframe.h"

/** @addtogroup KVP
    @{ 
*/
/** @file kvputil-p.h
  * @brief Private KVP utilities for backends etc.
  * @author Copyright (C) 2001, 2003 Linas Vepstas <linas@linas.org>
  * @author Copyright 2008 Neil Williams <linux@codehelp.co.uk>
*/

/** \brief Convert a QofIdType to a KvpValueType

 Used by various backends to convert QofParam into SQL structures.

 \note Not all KvpValueType types can be converted to QofIdTypeConst,
 in particular ::KVP_TYPE_BINARY, ::KVP_TYPE_GLIST and ::KVP_TYPE_FRAME

 \return The KvpValueType or zero if the type cannot be converted.
*/
KvpValueType
qof_id_to_kvp_value_type (QofIdTypeConst type_string);

/** \brief Convert a KvpValueType to a  QofIdType

 Used by various backends to convert QofParam into SQL structures.

 \note Not all KvpValueType types can be converted to QofIdTypeConst,
 in particular ::KVP_TYPE_BINARY, ::KVP_TYPE_GLIST and ::KVP_TYPE_FRAME

 \return The QofIdTypeConst or NULL if the type cannot be converted.
*/
QofIdTypeConst
kvp_value_type_to_qof_id (KvpValueType n);

/** @} */

/** @} */
#endif /* KVPUTIL_P_H */
