/********************************************************************\
 * util.c -- misc utility functions                                 *
 * Copyright (C) 1998, 1999, 2000, 2002 Linas Vepstas               *
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
 * Linas Vepstas March 2002
 */

#define _GNU_SOURCE
#define __EXTENSIONS__

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <glib.h>
#include <gtk/gtk.h>

#include "util.h"

/* ============================================================== */

void
xxxgtk_textview_set_text (GtkTextView *text, const char *str)
{
	GtkTextBuffer *buff = gtk_text_view_get_buffer (text);
	if (!str) str = "";
	gtk_text_buffer_set_text (buff, str, strlen (str));

}

char *
xxxgtk_textview_get_text (GtkTextView *text)
{
	GtkTextIter start, end;
	GtkTextBuffer *buff = gtk_text_view_get_buffer (text);
	gtk_text_buffer_get_start_iter (buff, &start);
	gtk_text_buffer_get_end_iter (buff, &end);
	return gtk_text_buffer_get_text(buff, &start, &end, TRUE);
}

/* ============================================================== */


long int
xxxgnc_timezone (struct tm *tm)
{
  if (!tm) return 0;

#ifdef HAVE_STRUCT_TM_GMTOFF
  /* tm_gmtoff is seconds *east* of UTC and is
   * already adjusted for daylight savings time. */
  return -(tm->tm_gmtoff);
#else
  /* timezone is seconds *west* of UTC and is
   * not adjusted for daylight savings time.
   * In Spring, we spring forward, wheee! */
  return timezone - (tm->tm_isdst > 0 ? 60 * 60 : 0);
#endif
}


time_t
xxxgnc_iso8601_to_secs_gmt(const char *str)
{
   return (time_t) xxxgnc_iso8601_to_timespec_gmt(str) . tv_sec;
}

/********************************************************************\
 * iso 8601 datetimes should look like 1998-07-02 11:00:00.68-05
\********************************************************************/
/* hack alert -- this routine returns incorrect values for 
 * dates before 1970 */

xxxTimespec
xxxgnc_iso8601_to_timespec_gmt(const char *str)
{
  char buf[4];
  xxxTimespec ts;
  struct tm stm;
  long int nsec =0;

  ts.tv_sec=0;
  ts.tv_nsec=0;
  if (!str) return ts;

  stm.tm_year = atoi(str) - 1900;
  str = strchr (str, '-'); if (str) { str++; } else { return ts; }
  stm.tm_mon = atoi(str) - 1;
  str = strchr (str, '-'); if (str) { str++; } else { return ts; }
  stm.tm_mday = atoi(str);

  str = strchr (str, ' '); if (str) { str++; } else { return ts; }
  stm.tm_hour = atoi(str);
  str = strchr (str, ':'); if (str) { str++; } else { return ts; }
  stm.tm_min = atoi(str);
  str = strchr (str, ':'); if (str) { str++; } else { return ts; }
  stm.tm_sec = atoi (str);

  /* The decimal point, optionally present ... */
  /* hack alert -- this algo breaks if more than 9 decimal places present */
  if (strchr (str, '.')) 
  { 
     int decimals, i, multiplier=1000000000;
     str = strchr (str, '.') +1;
     decimals = strcspn (str, "+- ");
     for (i=0; i<decimals; i++) multiplier /= 10;
     nsec = atoi(str) * multiplier;
  }
  stm.tm_isdst = -1;

  /* Timezone format can be +hh or +hhmm or +hh.mm (or -) (or not present) */
  str += strcspn (str, "+-");
  if (str)
  {
    buf[0] = str[0];
    buf[1] = str[1];
    buf[2] = str[2];
    buf[3] = 0;
    stm.tm_hour -= atoi(buf);

    str +=3;
    if ('.' == *str) str++;
    if (isdigit (*str) && isdigit (*(str+1)))
    {
      int cyn;
      /* copy sign from hour part */
      if ('+' == buf[0]) { cyn = -1; } else { cyn = +1; } 
      buf[0] = str[0];
      buf[1] = str[1];
      buf[2] = str[2];
      buf[3] = 0;
      stm.tm_min += cyn * atoi(buf);
    }
  }

  /* Note that mktime returns 'local seconds' which is the true time
   * minus the timezone offset.  We don't want to work with local 
   * seconds, since they swim around acording to daylight savings, etc. 
   * We want to work with universal time.  Thus, add an offset
   * to undo the damage that mktime causes.
   */
  {
    struct tm tmp_tm;
    struct tm *tm;
    long int tz;
    int tz_hour;
    time_t secs;

    /* Use a temporary tm struct so the mktime call below
     * doesn't mess up stm. */
    tmp_tm = stm;
    tmp_tm.tm_isdst = -1;

    secs = mktime (&tmp_tm);

    /* The call to localtime is 'bogus', but it forces 'timezone' to
     * be set. Note that we must use the accurate date, since the
     * value of 'gnc_timezone' includes daylight savings corrections
     * for that date. */
    tm = localtime (&secs);

    tz = xxxgnc_timezone (tm);

    tz_hour = tz / 3600;
    stm.tm_hour -= tz_hour;
    stm.tm_min -= (tz - (3600 * tz_hour)) / 60;
    stm.tm_isdst = tmp_tm.tm_isdst;
  }

  ts.tv_sec = mktime (&stm);
  ts.tv_nsec = nsec;

  return ts;
}

/********************************************************************\
\********************************************************************/

char * 
xxxgnc_timespec_to_iso8601_buff (xxxTimespec ts, char * buff)
{
  int len;
  int tz_hour, tz_min;
  char cyn;
  time_t tmp;
  struct tm parsed;

  tmp = ts.tv_sec;
  localtime_r(&tmp, &parsed);

  tz_hour = xxxgnc_timezone (&parsed) / 3600;
  tz_min = (xxxgnc_timezone (&parsed) - 3600*tz_hour) / 60;
  if (0>tz_min) { tz_min +=60; tz_hour --; }
  if (60<=tz_min) { tz_min -=60; tz_hour ++; }

  /* We also have to print the sign by hand, to work around a bug
   * in the glibc 2.1.3 printf (where %+02d fails to zero-pad).
   */
  cyn = '-';
  if (0>tz_hour) { cyn = '+'; tz_hour = -tz_hour; }

  len = sprintf (buff, "%4d-%02d-%02d %02d:%02d:%02d.%06ld %c%02d%02d",
                 parsed.tm_year + 1900,
                 parsed.tm_mon + 1,
                 parsed.tm_mday,
                 parsed.tm_hour,
                 parsed.tm_min,
                 parsed.tm_sec,
                 ts.tv_nsec / 1000,
                 cyn,
                 tz_hour,
                 tz_min);

  /* Return pointer to end of string. */
  buff += len;
  return buff;
}

char * 
xxxgnc_secs_to_iso8601_buff (time_t secs, char * buff)
{
	xxxTimespec ts;
	ts.tv_sec = secs;
	ts.tv_nsec = 0;
	return xxxgnc_timespec_to_iso8601_buff (ts, buff);
}

/* ========================== END OF FILE ============================ */
