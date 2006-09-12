/********************************************************************
 *            qoferror-p.h
 *
 *  Sun Sep 10 21:19:25 2006
 *  Copyright  2006  Neil Williams
 *  linux@codehelp.co.uk
 *******************************************************************/
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
 
#ifndef _QOFERROR_P_H
#define _QOFERROR_P_H

#ifndef QOF_DISABLE_DEPRECATED
/* deprecated code support */
#define ERR_LAST 5000
void
set_deprecated_errors (void);
void
deprecated_support (QofErrorId id, const gchar * err_message);
#endif

void
qof_error_init (void);

void
qof_error_close (void);

#endif /* _QOFERROR_P_H */
