/********************************************************************\
 * util.h -- misc utilities                                         *
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
 * misc utilities
 *
 * HISTORY:
 * Linas Vepstas <linas@linas.org> March 2002
 */

#ifndef DUI_UTIL_H
#define DUI_UTIL_H

#include <time.h>
#include <gtk/gtktextview.h>

/* ------------------------------------------------------------------ */
/* some gtk-like utilities */
void xxxgtk_textview_set_text (GtkTextView *text, const char *str);
char * xxxgtk_textview_get_text (GtkTextView *text);


/* The following should be eliminated if/when a link to QOF is mandatory */
struct xxxtimespec64
{
   long long int tv_sec;
   long int tv_nsec;
};
typedef struct xxxtimespec64 xxxTimespec;

time_t xxxgnc_iso8601_to_secs_gmt(const char *str);
xxxTimespec xxxgnc_iso8601_to_timespec_gmt(const char *str);

char * xxxgnc_secs_to_iso8601_buff (time_t secs, char * buff);

#endif /* DUI_UTIL_H */

/* ========================== END OF FILE ============================ */
