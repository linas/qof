/* *****************************************************************\
 * deprecated.c -- QOF deprecated function replacements            *
 * Copyright (c) 2005 Neil Williams <linux@codehelp.co.uk>          *
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

#include "config.h"
#ifndef QOF_DISABLE_DEPRECATED

/* keep the deprecated functions close to the originals -
including use of localtime - that's why these are deprecated! */
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>

#ifdef HAVE_LANGINFO_H
#define HAVE_LANGINFO_D_FMT 1
#include <langinfo.h>
#endif
#ifdef HAVE_LANGINFO_D_FMT
#  define QOF_D_FMT (nl_langinfo (D_FMT))
#  define QOF_D_T_FMT (nl_langinfo (D_T_FMT))
#  define QOF_T_FMT (nl_langinfo (T_FMT))
#else
#  define QOF_D_FMT   "%F"
#  define QOF_D_T_FMT "%F %r"
#  define QOF_T_FMT   "%i"
#endif

#include <glib.h>
#include <libintl.h>
#define _(String) dgettext (GETTEXT_PACKAGE, String)
#include "qof.h"
#include "qofsession-p.h"
#include "qoferror-p.h"

static QofLogModule log_module = "deprecated";
static FILE *fout = NULL;

/* Don't be fooled: gnc_trace_num_spaces has external linkage and
   static storage, but can't be defined with 'extern' because it has
   an initializer, and can't be declared with 'static' because that
   would give it internal linkage. (this is why it is deprecated) */
gint __attribute__ ((unused)) gnc_trace_num_spaces = 0;
void
gnc_log_init (void)
{
	qof_log_init ();
}

void
gnc_set_log_level (QofLogModule log_module, gncLogLevel level)
{
	qof_log_set_level (log_module, (QofLogLevel) level);
}

void
gnc_set_log_level_global (gncLogLevel level)
{
	qof_log_set_level_registered ((QofLogLevel) level);
}

void
qof_log_set_level_global (QofLogLevel level)
{
	qof_log_set_level_registered ((QofLogLevel) level);
}

void
gnc_set_logfile (FILE * outfile)
{
	qof_log_set_file (outfile);
}
const char *
gnc_log_prettify (const char *name)
{
	return qof_log_prettify (name);
}

void
gnc_start_clock (int a __attribute__ ((unused)), 
	QofLogModule b __attribute__ ((unused)), 
	gncLogLevel c __attribute__ ((unused)), 
	const char *d __attribute__ ((unused)),
	const char *e __attribute__ ((unused)), ...)
{
}
void
gnc_report_clock (int a __attribute__ ((unused)), 
	QofLogModule b __attribute__ ((unused)), 
	gncLogLevel c __attribute__ ((unused)), 
	const char *d __attribute__ ((unused)),
	const char *e __attribute__ ((unused)), ...)
{
}
void
gnc_report_clock_total (int a __attribute__ ((unused)), 
	QofLogModule b __attribute__ ((unused)), 
	gncLogLevel c __attribute__ ((unused)),
	const char *d __attribute__ ((unused)), 
	const char *e __attribute__ ((unused)), ...)
{
}

gboolean
gnc_should_log (QofLogModule log_module, gncLogLevel log_level)
{
	return qof_log_check (log_module, log_level);
}

gint
gnc_engine_register_event_handler (GNCEngineEventHandler handler,
	gpointer user_data)
{
	return qof_event_register_old_handler (handler, user_data);
}

void
gnc_engine_unregister_event_handler (gint handler_id)
{
	qof_event_unregister_handler (handler_id);
}

void
gnc_engine_suspend_events (void)
{
	qof_event_suspend ();
}

void
gnc_engine_resume_events (void)
{
	qof_event_resume ();
}

void
gnc_engine_gen_event (QofEntity * entity, GNCEngineEventType event_type)
{
	qof_event_gen (entity, event_type, NULL);
}

QofBookMergeData *
qof_book_mergeInit (QofBook * importBook, QofBook * targetBook)
{
	return qof_book_merge_init (importBook, targetBook);
}

QofBookMergeData *
qof_book_mergeUpdateResult (QofBookMergeData * mergeData,
	QofBookMergeResult tag)
{
	return qof_book_merge_update_result (mergeData, tag);
}

gint
qof_book_mergeCommit (QofBookMergeData * mergeData)
{
	return qof_book_merge_commit (mergeData);
}

void
qof_book_mergeRuleForeach (QofBookMergeData * mergeData,
	QofBookMergeRuleForeachCB cb, QofBookMergeResult mergeResult)
{
	qof_book_merge_rule_foreach (mergeData, cb, mergeResult);
}

gpointer
gnc_string_cache_insert (gconstpointer key)
{
	return qof_util_string_cache_insert (key);
}

gchar *
gnc_stpcpy (gchar * dest, const gchar * src)
{
	return g_stpcpy (dest, src);
}

GCache *
gnc_engine_get_string_cache (void)
{
	return qof_util_get_string_cache ();
}

void
gnc_engine_string_cache_destroy (void)
{
	qof_util_string_cache_destroy ();
}

void
gnc_string_cache_remove (gconstpointer key)
{
	qof_util_string_cache_remove (key);
}

gboolean
gnc_date_string_to_dateformat (const gchar * format_string,
	QofDateFormat * format)
{
	QofDateFormat df;

	df = qof_date_format_from_name (format_string);
	if (df < 0)
		return TRUE;
	*format = df;
	return FALSE;
}

gboolean
gnc_date_string_to_monthformat (const gchar * format_string,
	GNCDateMonthFormat * format)
{
	if (!format_string)
		return TRUE;

	if (!strcmp (format_string, "number"))
		*format = GNCDATE_MONTH_NUMBER;
	else if (!strcmp (format_string, "abbrev"))
		*format = GNCDATE_MONTH_ABBREV;
	else if (!strcmp (format_string, "name"))
		*format = GNCDATE_MONTH_NAME;
	else
		return TRUE;

	return FALSE;
}
static inline QofTime *
timespecToQofTime (Timespec ts)
{
	QofTime *time;

	time = qof_time_new ();
	qof_time_set_secs (time, ts.tv_sec);
	qof_time_set_nanosecs (time, ts.tv_nsec);
	return time;
}
static inline Timespec
qof_time_to_Timespec (QofTime * time)
{
	Timespec ts;

	ts.tv_sec = qof_time_get_secs (time);
	ts.tv_nsec = qof_time_get_nanosecs (time);
	return ts;
}
static inline Timespec
null_timespec (void)
{
	Timespec ts;

	ts.tv_sec = 0;
	ts.tv_nsec = 0;
	return ts;
}

gboolean
timespec_equal (const Timespec * ta, const Timespec * tb)
{
	QofTime *qta, *qtb;
	gboolean retval;

	qta = timespecToQofTime (*ta);
	qtb = timespecToQofTime (*tb);
	retval = qof_time_equal (qta, qtb);
	qof_time_free (qta);
	qof_time_free (qtb);
	return retval;
}

gint
timespec_cmp (const Timespec * ta, const Timespec * tb)
{
	QofTime *qta, *qtb;
	gint retval;

	qta = timespecToQofTime (*ta);
	qtb = timespecToQofTime (*tb);
	retval = qof_time_cmp (qta, qtb);
	qof_time_free (qta);
	qof_time_free (qtb);
	return retval;
}

void
timespecFromTime_t (Timespec * ts, time_t t)
{
	QofTime *time;

	time = qof_time_new ();
	time = qof_time_from_time_t (t, 0);
	*ts = qof_time_to_Timespec (time);
	qof_time_free (time);
}

time_t
timespecToTime_t (Timespec ts)
{
	return ts.tv_sec;
}

Timespec
timespec_diff (const Timespec * ta, const Timespec * tb)
{
	Timespec ts;
	QofTime *qta, *qtb, *qt;

	qta = timespecToQofTime (*ta);
	qtb = timespecToQofTime (*tb);
	qt = qof_time_diff (qta, qtb);
	ts = qof_time_to_Timespec (qt);
	qof_time_free (qta);
	qof_time_free (qtb);
	qof_time_free (qt);
	return ts;
}

Timespec
timespec_abs (const Timespec * t)
{
	Timespec ts;
	QofTime *qt = timespecToQofTime (*t);
	qof_time_abs (qt);
	ts = qof_time_to_Timespec (qt);
	qof_time_free (qt);
	return ts;
}

Timespec
timespecCanonicalDayTime (Timespec t)
{
	struct tm tm, *result;
	Timespec retval;

	time_t t_secs = t.tv_sec + (t.tv_nsec / QOF_NSECS);
	result = localtime(&t_secs);
	tm = *result;
	gnc_tm_set_day_middle(&tm);
	retval.tv_sec = mktime(&tm);
	retval.tv_nsec = 0;
	return retval;
}

time_t
qof_date_dmy_to_sec (gint day, gint month, gint year)
{
	QofTime *qt;
	QofDate *qd;
	time_t retval;

	qd = qof_date_new ();
	qd->qd_mday = day;
	qd->qd_mon  = month;
	qd->qd_year = year;
	qt = qof_date_to_qtime (qd);
	retval = qof_time_get_secs (qt);
	qof_time_free (qt);
	qof_date_free (qd);
	return retval;
}

size_t
qof_print_hours_elapsed_buff (char *buff, size_t len, int secs,
	gboolean show_secs)
{
	size_t flen;
	if (0 <= secs)
	{
		if (show_secs)
		{
			flen = g_snprintf (buff, len,
				"%02d:%02d:%02d", (int) (secs / 3600),
				(int) ((secs % 3600) / 60), (int) (secs % 60));
		}
		else
		{
			flen = g_snprintf (buff, len,
				"%02d:%02d", (int) (secs / 3600),
				(int) ((secs % 3600) / 60));
		}
	}
	else
	{
		if (show_secs)
		{
			flen = g_snprintf (buff, len,
				"-%02d:%02d:%02d", (int) (-secs / 3600),
				(int) ((-secs % 3600) / 60), (int) (-secs % 60));
		}
		else
		{
			flen = g_snprintf (buff, len,
				"-%02d:%02d", (int) (-secs / 3600),
				(int) ((-secs % 3600) / 60));
		}
	}
	return flen;
}

size_t
qof_print_minutes_elapsed_buff (char *buff, size_t len, int secs,
	gboolean show_secs)
{
	size_t flen;
	if (0 <= secs)
	{
		if (show_secs)
		{
			flen = g_snprintf (buff, len,
				"%02d:%02d", (int) (secs / 60), (int) (secs % 60));
		}
		else
		{
			flen = g_snprintf (buff, len, "%02d", (int) (secs / 60));
		}
	}
	else
	{
		if (show_secs)
		{
			flen = g_snprintf (buff, len,
				"-%02d:%02d", (int) (-secs / 60), (int) (-secs % 60));
		}
		else
		{
			flen = g_snprintf (buff, len, "-%02d", (int) (-secs / 60));
		}
	}
	return flen;
}

size_t
qof_print_date_time_buff (char *buff, size_t len, time_t secs)
{
	int flen;
	int day, month, year, hour, min, sec;
	struct tm ltm, gtm;

	if (!buff)
		return 0;
	ltm = *localtime (&secs);
	day = ltm.tm_mday;
	month = ltm.tm_mon + 1;
	year = ltm.tm_year + 1900;
	hour = ltm.tm_hour;
	min = ltm.tm_min;
	sec = ltm.tm_sec;
	switch (qof_date_format_get_current ())
	{
	case QOF_DATE_FORMAT_UK:
		flen =
			g_snprintf (buff, len, "%2d/%2d/%-4d %2d:%02d", day, month,
			year, hour, min);
		break;
	case QOF_DATE_FORMAT_CE:
		flen =
			g_snprintf (buff, len, "%2d.%2d.%-4d %2d:%02d", day, month,
			year, hour, min);
		break;
	case QOF_DATE_FORMAT_ISO:
		flen =
			g_snprintf (buff, len, "%04d-%02d-%02d %02d:%02d", year, month,
			day, hour, min);
		break;
	case QOF_DATE_FORMAT_UTC:
		{
			gtm = *gmtime (&secs);
			flen = strftime (buff, len, QOF_UTC_DATE_FORMAT, &gtm);
			break;
		}
	case QOF_DATE_FORMAT_LOCALE:
		{
			flen = strftime (buff, len, QOF_D_T_FMT, &ltm);
		}
		break;

	case QOF_DATE_FORMAT_US:
	default:
		flen =
			g_snprintf (buff, len, "%2d/%2d/%-4d %2d:%02d", month, day,
			year, hour, min);
		break;
	}
	return flen;
}

size_t
qof_print_time_buff (gchar * buff, size_t len, time_t secs)
{
	gint flen;
	struct tm ltm, gtm;

	if (!buff)
		return 0;
	if (qof_date_format_get_current () == QOF_DATE_FORMAT_UTC)
	{
		gtm = *gmtime (&secs);
		flen = strftime (buff, len, QOF_UTC_DATE_FORMAT, &gtm);
		return flen;
	}
	ltm = *localtime (&secs);
	flen = strftime (buff, len, QOF_T_FMT, &ltm);

	return flen;
}

gboolean
qof_is_same_day (time_t ta, time_t tb)
{
	struct tm lta, ltb;
	lta = *localtime (&ta);
	ltb = *localtime (&tb);
	if (lta.tm_year == ltb.tm_year)
	{
	return (ltb.tm_yday - lta.tm_yday);
	}
	return (ltb.tm_year - lta.tm_year)*365;  /* very approximate */
}

void
gnc_tm_set_day_start (struct tm *tm)
{
	tm->tm_hour = 0;
	tm->tm_min = 0;
	tm->tm_sec = 0;
	tm->tm_isdst = -1;
}

void
gnc_tm_get_day_start (struct tm *tm, time_t time_val)
{
	tm = localtime_r (&time_val, tm);
	gnc_tm_set_day_start (tm);
}

void
gnc_tm_set_day_middle (struct tm *tm)
{
	tm->tm_hour = 12;
	tm->tm_min = 0;
	tm->tm_sec = 0;
	tm->tm_isdst = -1;
}

void
gnc_tm_set_day_end (struct tm *tm)
{
	tm->tm_hour = 23;
	tm->tm_min = 59;
	tm->tm_sec = 59;
	tm->tm_isdst = -1;
}

void
gnc_tm_get_day_end (struct tm *tm, time_t time_val)
{
	tm = localtime_r (&time_val, tm);
	gnc_tm_set_day_end (tm);
}

time_t
gnc_timet_get_day_start (time_t time_val)
{
	struct tm tm;

	gnc_tm_get_day_start (&tm, time_val);
	return mktime (&tm);
}

time_t
gnc_timet_get_day_end (time_t time_val)
{
	struct tm tm;

	gnc_tm_get_day_end (&tm, time_val);
	return mktime (&tm);
}

#ifndef GNUCASH_MAJOR_VERSION
time_t
gnc_timet_get_day_start_gdate (GDate * date)
{
	struct tm stm;
	time_t secs;

	stm.tm_year = g_date_get_year (date) - 1900;
	stm.tm_mon = g_date_get_month (date) - 1;
	stm.tm_mday = g_date_get_day (date);
	gnc_tm_set_day_start (&stm);
	secs = mktime (&stm);
	return secs;
}

time_t
gnc_timet_get_day_end_gdate (GDate * date)
{
	struct tm stm;
	time_t secs;

	stm.tm_year = g_date_get_year (date) - 1900;
	stm.tm_mon = g_date_get_month (date) - 1;
	stm.tm_mday = g_date_get_day (date);
	gnc_tm_set_day_end (&stm);
	secs = mktime (&stm);
	return secs;
}
#endif
int
gnc_date_my_last_mday (int month, int year)
{
	return g_date_get_days_in_month (month, year);
}

int
date_get_last_mday (struct tm *tm)
{
	return g_date_get_days_in_month (tm->tm_mon + 1, tm->tm_year + 1900);
}

gboolean
date_is_last_mday (struct tm * tm)
{
	return (tm->tm_mday ==
		g_date_get_days_in_month (tm->tm_mon + 1, tm->tm_year + 1900));
}

int
gnc_timespec_last_mday (Timespec t)
{
	/* Replacement code should not use localtime */
	struct tm *result;
	time_t t_secs = t.tv_sec + (t.tv_nsec / QOF_NSECS);
	result = localtime (&t_secs);
	return date_get_last_mday (result);
}

void
gnc_tm_get_today_start (struct tm *tm)
{
	gnc_tm_get_day_start (tm, time (NULL));
}

void
gnc_tm_get_today_end (struct tm *tm)
{
	gnc_tm_get_day_end (tm, time (NULL));
}

time_t
gnc_timet_get_today_start (void)
{
	struct tm tm;

	gnc_tm_get_day_start (&tm, time (NULL));
	return mktime (&tm);
}

time_t
gnc_timet_get_today_end (void)
{
	struct tm tm;

	gnc_tm_get_day_end (&tm, time (NULL));
	return mktime (&tm);
}

char *
xaccDateUtilGetStamp (time_t thyme)
{
	struct tm *stm;

	stm = localtime (&thyme);
	return g_strdup_printf ("%04d%02d%02d%02d%02d%02d",
		(stm->tm_year + 1900),
		(stm->tm_mon + 1),
		stm->tm_mday, stm->tm_hour, stm->tm_min, stm->tm_sec);
}

size_t
qof_print_date_dmy_buff (char *buff, size_t len, int day, int month,
	int year)
{
	int flen;
	if (!buff)
		return 0;
	switch (qof_date_format_get_current ())
	{
	case QOF_DATE_FORMAT_UK:
		flen = g_snprintf (buff, len, "%2d/%2d/%-4d", day, month, year);
		break;
	case QOF_DATE_FORMAT_CE:
		flen = g_snprintf (buff, len, "%2d.%2d.%-4d", day, month, year);
		break;
	case QOF_DATE_FORMAT_LOCALE:
		{
			struct tm tm_str;
			time_t t;
			tm_str.tm_mday = day;
			tm_str.tm_mon = month - 1;
			tm_str.tm_year = year - 1900;
			gnc_tm_set_day_start (&tm_str);
			t = mktime (&tm_str);
			localtime_r (&t, &tm_str);
			flen = strftime (buff, len, QOF_D_FMT, &tm_str);
			if (flen != 0)
				break;
		}
	case QOF_DATE_FORMAT_ISO:
	case QOF_DATE_FORMAT_UTC:
		flen = g_snprintf (buff, len, "%04d-%02d-%02d", year, month, day);
		break;
	case QOF_DATE_FORMAT_US:
	default:
		flen = g_snprintf (buff, len, "%2d/%2d/%-4d", month, day, year);
		break;
	}
	return flen;
}

size_t
qof_print_date_buff (char *buff, size_t len, time_t t)
{
	struct tm *theTime;
	if (!buff)
		return 0;
	theTime = localtime (&t);
	return qof_print_date_dmy_buff (buff, len,
		theTime->tm_mday, theTime->tm_mon + 1, theTime->tm_year + 1900);
}

size_t
qof_print_gdate (gchar * buf, size_t len __attribute__ ((unused)), 
	GDate * gd)
{
	QofDateFormat df;
	QofDate *qd;
	gchar *str;

	df = qof_date_format_get_current ();
	qd = qof_date_from_gdate (gd);
	str = qof_date_print (qd, df);
	qof_date_free (qd);
	g_stpcpy (buf, str);
	g_free (str);
	return strlen (buf);
}

gchar *
qof_print_date (time_t t)
{
	QofDateFormat df;
	QofTime *time;
	gchar *str;

	df = qof_date_format_get_current ();
	time = qof_time_from_time_t (t, 0);
	str = qof_date_print (qof_date_from_qtime (time), df);
	qof_time_free (time);
	return str;
}
const gchar *
gnc_print_date (Timespec ts)
{
	static gchar buff[MAX_DATE_LENGTH];
	QofDateFormat df;
	QofTime *time;
	gchar *str;

	df = qof_date_format_get_current ();
	ENTER (" using date format %d", df);
	time = timespecToQofTime (ts);
	str = qof_date_print (qof_date_from_qtime (time), df);
	qof_time_free (time);
	g_stpcpy (buff, str);
	g_free (str);
	LEAVE (" printing %s", buff);
	return buff;
}

/* This is now user configured through the gnome options system() */
static QofDateFormat dateFormat = QOF_DATE_FORMAT_LOCALE;
static QofDateFormat prevQofDateFormat = QOF_DATE_FORMAT_LOCALE;

static gboolean
qof_scan_date_internal (const char *buff, int *day, int *month, int *year,
                  QofDateFormat which_format)
{
   char *dupe, *tmp, *first_field, *second_field, *third_field;
   int iday, imonth, iyear;
   struct tm *now, utc;
   time_t secs;

   if (!buff) return(FALSE);

	if(which_format == QOF_DATE_FORMAT_UTC)
	{
		if(strptime(buff, QOF_UTC_DATE_FORMAT, &utc)) {
			*day = utc.tm_mday;
			*month = utc.tm_mon + 1;
			*year = utc.tm_year + 1900;
			return TRUE;
		}
		else { return FALSE; }
	}
   dupe = g_strdup (buff);

   tmp = dupe;
   first_field = NULL;
   second_field = NULL;
   third_field = NULL;

   /* Use strtok to find delimiters */
   if (tmp) {
     static char *delims = ".,-+/\\() ";

      first_field = strtok (tmp, delims);
      if (first_field) {
         second_field = strtok (NULL, delims);
         if (second_field) {
            third_field = strtok (NULL, delims);
         }
      }
   }

   /* If any fields appear to be blank, use today's date */
   time (&secs);
   now = localtime (&secs);
   iday = now->tm_mday; 
   imonth = now->tm_mon+1;
   iyear = now->tm_year+1900;

   /* get numeric values */
   switch (which_format)
   {
     case QOF_DATE_FORMAT_LOCALE:
       if (buff[0] != '\0')
       {
         struct tm thetime;

         /* Parse time string. */
         memset(&thetime, -1, sizeof(struct tm));
         strptime (buff, QOF_D_FMT, &thetime);

         if (third_field) {
           /* Easy.  All three values were parsed. */
           iyear = thetime.tm_year + 1900;
           iday = thetime.tm_mday;
           imonth = thetime.tm_mon + 1;
         } else if (second_field) {
           /* Hard. Two values parsed.  Figure out the ordering. */
           if (thetime.tm_year == -1) {
             /* %m-%d or %d-%m. Don't care. Already parsed correctly. */
             iday = thetime.tm_mday;
             imonth = thetime.tm_mon + 1;
           } else if (thetime.tm_mon != -1) {
             /* Must be %Y-%m-%d. Reparse as %m-%d.*/
             imonth = atoi(first_field);
             iday = atoi(second_field);
           } else {
             /* Must be %Y-%d-%m. Reparse as %d-%m. */
             iday = atoi(first_field);
             imonth = atoi(second_field);
           }
         } else if (first_field) {
           iday = atoi(first_field);
         }
       }
       break;
     case QOF_DATE_FORMAT_UK:
     case QOF_DATE_FORMAT_CE:
       if (third_field) {
         iday = atoi(first_field);
         imonth = atoi(second_field);
         iyear = atoi(third_field);
       } else if (second_field) {
         iday = atoi(first_field);
         imonth = atoi(second_field);
       } else if (first_field) {
         iday = atoi(first_field);
       }
       break;
     case QOF_DATE_FORMAT_ISO:
       if (third_field) {
         iyear = atoi(first_field);
         imonth = atoi(second_field);
         iday = atoi(third_field);
       } else if (second_field) {
         imonth = atoi(first_field);
         iday = atoi(second_field);
       } else if (first_field) {
         iday = atoi(first_field);
       }
       break;
    case QOF_DATE_FORMAT_US:
    default:
       if (third_field) {
         imonth = atoi(first_field);
         iday = atoi(second_field);
         iyear = atoi(third_field);
       } else if (second_field) {
         imonth = atoi(first_field);
         iday = atoi(second_field);
       } else if (first_field) {
         iday = atoi(first_field);
       }
       break;
   }

   g_free (dupe);

   if ((12 < imonth) || (31 < iday)) 
   {
     /* 
      * Ack! Thppfft!  Someone just fed this routine a string in the
      * wrong date format.  This is known to happen if a register
      * window is open when changing the date format.  Try the
      * previous date format.  If that doesn't work, see if we can
      * exchange month and day. If that still doesn't work,
      * bail and give the caller what they asked for (garbage) 
      * parsed in the new format.
      *
      * Note: This test cannot detect any format change that only
      * swaps month and day field, if the day is 12 or less.  This is
      * deemed acceptable given the obscurity of this bug.
      */
     if ((which_format != prevQofDateFormat) &&
         qof_scan_date_internal(buff, day, month, year, prevQofDateFormat))
     {
       return(TRUE);
     }
     if ((12 < imonth) && (12 >= iday))
     {
        int tmp = imonth; imonth = iday; iday = tmp;
     } 
	  else
	  {
        return FALSE;
	  }
   }

   /* If the year entered is smaller than 100, assume we mean the current
      century (and are not revising some roman emperor's books) */
   if (iyear < 100)
     iyear += ((int) ((now->tm_year+1950-iyear)/100)) * 100;

   if (year) *year=iyear;
   if (month) *month=imonth;
   if (day) *day=iday;
   return(TRUE);
}

gboolean
qof_scan_date (const char *buff, int *day, int *month, int *year)
{
  return qof_scan_date_internal(buff, day, month, year, dateFormat);
}

gboolean
qof_scan_date_secs (const gchar * buff, time_t * secs)
{
  gboolean rc;
  int day, month, year;
  
  rc = qof_scan_date_internal(buff, &day, &month, &year, dateFormat);
  if (secs) *secs = xaccDMYToSec (day, month, year);

  return rc;
}

Timespec
gnc_dmy2timespec (gint day, gint month, gint year)
{
	Timespec ts;
	QofTime *qt;
	QofDate *qd;

	if (!g_date_valid_dmy (day, month, year))
		return null_timespec ();
	qd = qof_date_new ();
	qd->qd_mday = day;
	qd->qd_mon  = month;
	qd->qd_year = year;
	qof_date_valid (qd);
	qt = qof_date_to_qtime (qd);
	ts = qof_time_to_Timespec (qt);
	qof_time_free (qt);
	qof_date_free (qd);
	return ts;
}

Timespec
gnc_dmy2timespec_end (gint day, gint month, gint year)
{
	Timespec ts;
	QofTime *qt;
	QofDate *qd;

	if (!g_date_valid_dmy (day, month, year))
		return null_timespec ();
	qd = qof_date_new ();
	qd->qd_mday = day;
	qd->qd_mon  = month;
	qd->qd_year = year;
	qof_date_set_day_end (qd);
	qt = qof_date_to_qtime (qd);
	ts = qof_time_to_Timespec (qt);
	qof_time_free (qt);
	qof_date_free (qd);
	return ts;
}

Timespec
gnc_iso8601_to_timespec_gmt (const gchar * str)
{
	gchar buf[4];
	gchar *dupe;
	Timespec ts;
	struct tm stm;
	glong nsec = 0;

	ts.tv_sec = 0;
	ts.tv_nsec = 0;
	if (!str)
		return ts;
	dupe = g_strdup (str);
	stm.tm_year = atoi (str) - 1900;
	str = strchr (str, '-');
	if (str)
	{
		str++;
	}
	else
	{
		return ts;
	}
	stm.tm_mon = atoi (str) - 1;
	str = strchr (str, '-');
	if (str)
	{
		str++;
	}
	else
	{
		return ts;
	}
	stm.tm_mday = atoi (str);

	str = strchr (str, ' ');
	if (str)
	{
		str++;
	}
	else
	{
		return ts;
	}
	stm.tm_hour = atoi (str);
	str = strchr (str, ':');
	if (str)
	{
		str++;
	}
	else
	{
		return ts;
	}
	stm.tm_min = atoi (str);
	str = strchr (str, ':');
	if (str)
	{
		str++;
	}
	else
	{
		return ts;
	}
	stm.tm_sec = atoi (str);

	if (strchr (str, '.'))
	{
		gint decimals, i, multiplier = 1000000000;
		str = strchr (str, '.') + 1;
		decimals = strcspn (str, "+- ");
		for (i = 0; i < decimals; i++)
			multiplier /= 10;
		nsec = atoi (str) * multiplier;
	}
	stm.tm_isdst = -1;

	str += strcspn (str, "+-");
	if (str)
	{
		buf[0] = str[0];
		buf[1] = str[1];
		buf[2] = str[2];
		buf[3] = 0;
		stm.tm_hour -= atoi (buf);

		str += 3;
		if ('.' == *str)
			str++;
		if (isdigit ((guchar) * str) && isdigit ((guchar) * (str + 1)))
		{
			gint cyn;
			if ('+' == buf[0])
			{
				cyn = -1;
			}
			else
			{
				cyn = +1;
			}
			buf[0] = str[0];
			buf[1] = str[1];
			buf[2] = str[2];
			buf[3] = 0;
			stm.tm_min += cyn * atoi (buf);
		}
	}

	{
		struct tm tmp_tm;
		struct tm tm;
		glong tz;
		gint tz_hour;
		time_t secs;

		tmp_tm = stm;
		tmp_tm.tm_isdst = -1;

		secs = mktime (&tmp_tm);

		if (secs < 0)
		{
			PWARN (" mktime failed to handle daylight saving: "
				"tm_hour=%d tm_year=%d tm_min=%d tm_sec=%d tm_isdst=%d for string=%s",
				stm.tm_hour, stm.tm_year, stm.tm_min,
				stm.tm_sec, stm.tm_isdst, dupe);
			tmp_tm.tm_hour++;
			secs = mktime (&tmp_tm);
			if (secs < 0)
			{
				tmp_tm.tm_hour -= 2;
				secs = mktime (&tmp_tm);
			}
			if (secs < 0)
			{
				PERR (" unable to recover from buggy mktime ");
				g_free (dupe);
				return ts;
			}
		}

		tm = *localtime_r (&secs, &tm);

		tz = gnc_timezone (&tmp_tm);

		tz_hour = tz / 3600;
		stm.tm_hour -= tz_hour;
		stm.tm_min -= (tz % 3600) / 60;
		stm.tm_isdst = tmp_tm.tm_isdst;
		ts.tv_sec = mktime (&stm);
		/* unreachable code */
/*		if (ts.tv_sec < 0)
		{
			PWARN (" mktime failed to adjust calculated time:"
				" tm_hour=%d tm_year=%d tm_min=%d tm_sec=%d tm_isdst=%d",
				stm.tm_hour, stm.tm_year, stm.tm_min,
				stm.tm_sec, stm.tm_isdst);
			ts.tv_sec = secs - tz;
		}*/
		ts.tv_nsec = nsec;
	}
	g_free (dupe);
	return ts;
}

gchar *
gnc_timespec_to_iso8601_buff (Timespec ts, gchar * buff)
{
	gint len, tz_hour, tz_min;
	glong secs;
	gchar cyn;
	time_t tmp;
	struct tm parsed;

	tmp = ts.tv_sec;
	localtime_r (&tmp, &parsed);

	secs = gnc_timezone (&parsed);
	tz_hour = secs / 3600;
	tz_min = (secs % 3600) / 60;

	cyn = '-';
	if (0 > tz_hour)
	{
		cyn = '+';
		tz_hour = -tz_hour;
	}

	len = sprintf (buff, "%4d-%02d-%02d %02d:%02d:%02d.%06ld %c%02d%02d",
		parsed.tm_year + 1900,
		parsed.tm_mon + 1,
		parsed.tm_mday,
		parsed.tm_hour,
		parsed.tm_min,
		parsed.tm_sec, ts.tv_nsec / 1000, cyn, tz_hour, tz_min);

	buff += len;
	return buff;
}

void
gnc_timespec2dmy (Timespec ts, gint * day, gint * month, gint * year)
{
	QofTime *time;
	QofDate *qd;

	time = timespecToQofTime (ts);
	qd = qof_date_from_qtime (time);
	qof_time_free (time);
	if (day)
		*day = qd->qd_mday;
	if (month)
		*month = qd->qd_mon;
	if ((year) && (qd->qd_year < 0) && (qd->qd_year > G_MAXINT))
		*year = (gint)qd->qd_year;
}

glong
gnc_timezone (struct tm *tm)
{
	g_return_val_if_fail (tm != NULL, 0);

	return -(tm->tm_gmtoff);
}

Timespec
qof_instance_get_last_update (QofInstance * inst)
{
	Timespec ts;
	ts = inst->last_update;
	inst->update_time = timespecToQofTime (ts);
	return ts;
}

void
qof_instance_set_last_update (QofInstance * inst, Timespec ts)
{
	QofTime *time;
	g_return_if_fail (inst);
	inst->last_update = ts;
	time = timespecToQofTime (ts);
	qof_instance_set_update_time (inst, time);
}
time_t 
xaccDMYToSec (int day, int month, int year)
{
  struct tm stm;
  time_t secs;

  stm.tm_year = year - 1900;
  stm.tm_mon = month - 1;
  stm.tm_mday = day;
  gnc_tm_set_day_start(&stm);

  secs = mktime (&stm);

  return secs;
}
void date_add_months (struct tm *tm, int months, gboolean track_last_day)
{
  gboolean was_last_day;
  int new_last_mday;

  was_last_day = date_is_last_mday(tm);

  tm->tm_mon += months;
  while (tm->tm_mon > 11) {
    tm->tm_mon -= 12;
    tm->tm_year++;
  }

  if (!track_last_day)
    return;

  new_last_mday = date_get_last_mday(tm);
  if (was_last_day || (tm->tm_mday > new_last_mday))
    tm->tm_mday = new_last_mday;
}
char dateSeparator (void)
{
	return qof_date_format_get_date_separator (qof_date_format_get_current());
}

const char*
gnc_date_dateformat_to_string(QofDateFormat format)
{
  switch (format) {
  case QOF_DATE_FORMAT_US:
    return "us";
  case QOF_DATE_FORMAT_UK:
    return "uk";
  case QOF_DATE_FORMAT_CE:
    return "ce";
  case QOF_DATE_FORMAT_ISO:
    return "iso";
  case QOF_DATE_FORMAT_UTC:
   return "utc";
  case QOF_DATE_FORMAT_LOCALE:
    return "locale";
  case QOF_DATE_FORMAT_CUSTOM:
    return "custom";
  default:
    return NULL;    
  }
}
const char*
gnc_date_monthformat_to_string(GNCDateMonthFormat format)
{
  switch (format) {
  case GNCDATE_MONTH_NUMBER:
    return "number";
  case GNCDATE_MONTH_ABBREV:
    return "abbrev";
  case GNCDATE_MONTH_NAME:
    return "name";
  default:
    return NULL;
  }
}
gboolean
qof_date_add_days(Timespec *ts, gint days)
{
	struct tm tm;
	time_t    tt;

	g_return_val_if_fail(ts, FALSE);
	tt = timespecToTime_t(*ts);
#ifdef HAVE_GMTIME_R
	tm = *gmtime_r(&tt, &tm);
#else
	tm = *gmtime(&tt);
#endif
	tm.tm_mday += days;
	tt = mktime(&tm);
	if(tt < 0) { return FALSE; }
	timespecFromTime_t(ts, tt);
	return TRUE;
}

gboolean
qof_date_add_months(Timespec *ts, gint months, gboolean track_last_day)
{
	struct tm tm;
	time_t    tt;
	gint new_last_mday;
	gboolean was_last_day;

	g_return_val_if_fail(ts, FALSE);
	tt = timespecToTime_t(*ts);
#ifdef HAVE_GMTIME_R
	tm = *gmtime_r(&tt, &tm);
#else
	tm = *gmtime(&tt);
#endif
	was_last_day = date_is_last_mday(&tm);
	tm.tm_mon += months;
	while (tm.tm_mon > 11) {
		tm.tm_mon -= 12;
		tm.tm_year++;
	}
	if (track_last_day) {
		new_last_mday = date_get_last_mday(&tm);
		if (was_last_day || (tm.tm_mday > new_last_mday)) {
			tm.tm_mday = new_last_mday;
		}
	}
	tt = mktime(&tm);
	if(tt < 0) { return FALSE; }
	timespecFromTime_t(ts, tt);
	return TRUE;
}

QofDateFormat qof_date_format_get (void)
{
  return qof_date_format_get_current ();
}

const gchar *qof_date_format_get_string(QofDateFormat df)
{
  switch(df) {
   case QOF_DATE_FORMAT_US:
    return "%m/%d/%y";
   case QOF_DATE_FORMAT_UK:
    return "%d/%m/%y";
   case QOF_DATE_FORMAT_CE:
    return "%d.%m.%y";
   case QOF_DATE_FORMAT_UTC:
    return "%Y-%m-%dT%H:%M:%SZ";
   case QOF_DATE_FORMAT_ISO:
    return "%y-%m-%d";
   case QOF_DATE_FORMAT_LOCALE:
   default:
    return QOF_D_FMT;
  };
}

void qof_date_format_set(QofDateFormat df)
{
	if(!qof_date_format_set_current (df))
		PERR (" unable to set current format, %d", df);
}

const gchar *qof_date_text_format_get_string(QofDateFormat df)
{
	return qof_date_format_get_format (df);
}

char *
xaccDateUtilGetStampNow (void)
{
   return qof_time_stamp_now ();
}

void
kvp_frame_set_timespec (KvpFrame * frame, const char *path, Timespec ts)
{
	KvpValue *value;
	value = kvp_value_new_timespec (ts);
	frame = kvp_frame_set_value_nc (frame, path, value);
	if (!frame)
		kvp_value_delete (value);
}

void
kvp_frame_add_timespec (KvpFrame * frame, const char *path, Timespec ts)
{
	KvpValue *value;
	value = kvp_value_new_timespec (ts);
	frame = kvp_frame_add_value_nc (frame, path, value);
	if (!frame)
		kvp_value_delete (value);
}

Timespec
kvp_frame_get_timespec (const KvpFrame * frame, 
	const char *path __attribute__ ((unused)))
{
	QofTime *qt;
	Timespec ts;
	char *key;

	key = NULL;
	ts.tv_sec = 0;
	ts.tv_nsec = 0;
	qt = kvp_value_get_time (kvp_frame_get_slot (frame, key));
	if(!qt)
		return ts;
	return qof_time_to_Timespec (qt);
}

KvpValue *
kvp_value_new_timespec (Timespec value)
{
	QofTime *qt;
	KvpValue *retval;

	qt = timespecToQofTime (value);
	retval = kvp_value_new_time (qt);
	return retval;
}

Timespec
kvp_value_get_timespec (const KvpValue * value)
{
	Timespec ts;
	QofTime *qt;
	ts.tv_sec = 0;
	ts.tv_nsec = 0;
	if (!value)
		return ts;
	qt = kvp_value_get_time (value);
	ts = qof_time_to_Timespec (qt);
	return ts;
}
#define NUM_CLOCKS 10
static struct timeval qof_clock[NUM_CLOCKS] = {
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
};

static struct timeval qof_clock_total[NUM_CLOCKS] = {
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
};

void
qof_start_clock (gint clockno, 
	QofLogModule log_module __attribute__ ((unused)),
	QofLogLevel log_level __attribute__ ((unused)), 
	const gchar * function_name, const gchar * format, ...)
{
	va_list ap;

	if ((0 > clockno) || (NUM_CLOCKS <= clockno))
		return;
#ifdef HAVE_GETTIMEOFDAY
	gettimeofday (&qof_clock[clockno], NULL);
#else
	time (&(qof_clock[clockno].tv_sec));
	qof_clock[clockno].tv_usec = 0;
#endif

	if (!fout)
		qof_log_init ();

	fprintf (fout, "Clock %d Start: %s: ",
		clockno, qof_log_prettify (function_name));

	va_start (ap, format);

	vfprintf (fout, format, ap);

	va_end (ap);

	fprintf (fout, "\n");
	fflush (fout);
}

void
qof_report_clock (gint clockno, 
	QofLogModule log_module __attribute__ ((unused)),
	QofLogLevel log_level __attribute__ ((unused)), 
	const gchar * function_name, const gchar * format, ...)
{
	struct timeval now;
	va_list ap;

	if ((0 > clockno) || (NUM_CLOCKS <= clockno))
		return;
#ifdef HAVE_GETTIMEOFDAY
	gettimeofday (&now, NULL);
#else
	time (&(now.tv_sec));
	now.tv_usec = 0;
#endif

	if (now.tv_usec < qof_clock[clockno].tv_usec)
	{
		now.tv_sec--;
		now.tv_usec += 1000000;
	}
	now.tv_sec -= qof_clock[clockno].tv_sec;
	now.tv_usec -= qof_clock[clockno].tv_usec;

	qof_clock_total[clockno].tv_sec += now.tv_sec;
	qof_clock_total[clockno].tv_usec += now.tv_usec;

	if (!fout)
		qof_log_init ();

	fprintf (fout, "Clock %d Elapsed: %ld.%06lds %s: ",
		clockno, (long int) now.tv_sec, (long int) now.tv_usec,
		qof_log_prettify (function_name));

	va_start (ap, format);

	vfprintf (fout, format, ap);

	va_end (ap);

	fprintf (fout, "\n");
	fflush (fout);
}

void
qof_report_clock_total (gint clockno,
	QofLogModule log_module __attribute__ ((unused)), 
	QofLogLevel log_level __attribute__ ((unused)),
	const gchar * function_name, const gchar * format, ...)
{
	va_list ap;

	if ((0 > clockno) || (NUM_CLOCKS <= clockno))
		return;

	while (qof_clock_total[clockno].tv_usec >= 1000000)
	{
		qof_clock_total[clockno].tv_sec++;
		qof_clock_total[clockno].tv_usec -= 1000000;
	}

	if (!fout)
		qof_log_init ();

	fprintf (fout, "Clock %d Total Elapsed: %ld.%06lds  %s: ",
		clockno,
		(long int) qof_clock_total[clockno].tv_sec,
		(long int) qof_clock_total[clockno].tv_usec,
		qof_log_prettify (function_name));

	va_start (ap, format);

	vfprintf (fout, format, ap);

	va_end (ap);

	fprintf (fout, "\n");
	fflush (fout);
}
static QofSession *current_session = NULL;

QofSession *
qof_session_get_current_session (void)
{
	if (!current_session)
	{
		qof_event_suspend ();
		current_session = qof_session_new ();
		qof_event_resume ();
	}

	return current_session;
}

void
qof_session_set_current_session (QofSession * session)
{
	current_session = session;
}

void
qof_session_clear_current_session (void)
{
	current_session = NULL;
}

gboolean
gnc_strisnum (const guchar * s)
{
	return qof_util_string_isnum (s);
}

KvpFrame *
gnc_kvp_bag_add (KvpFrame * pwd, const char *path,
	time_t secs, const char *first_name, ...)
{
	QofTime *qt;
	KvpFrame *cwd;
	va_list ap;

	qt = qof_time_from_time_t (secs, 0);
	va_start (ap, first_name);
	cwd = qof_kvp_bag_add(pwd, path, qt, first_name, ap);
	va_end (ap);
	return cwd;
}

KvpFrame *
gnc_kvp_bag_find_by_guid (KvpFrame * root, const gchar *path,
	const gchar *guid_name, GUID * desired_guid)
{
	return qof_kvp_bag_find_by_guid (root, path, 
		guid_name, desired_guid);
}

void
gnc_kvp_bag_remove_frame (KvpFrame * root, const char *path, 
	KvpFrame * fr)
{
	qof_kvp_bag_remove_frame (root, path, fr);
}

void
gnc_kvp_bag_merge (KvpFrame * kvp_into, const char *intopath,
	KvpFrame * kvp_from, const char *frompath)
{
	qof_kvp_bag_merge (kvp_into, intopath, kvp_from, frompath);
}

static gboolean param_flag = TRUE;
static void
param_edit_cb (QofParam * param, gpointer user_data)
{
	QofInstance * inst = (QofInstance*)user_data;
	param_flag = qof_util_param_edit (inst, param);
	if (!param_flag)
		return;
}

static void
param_commit_cb (QofParam * param, gpointer user_data)
{
	QofInstance * inst = (QofInstance*)user_data;
	param_flag = qof_util_param_commit (inst, param);
	if (!param_flag)
		return;
}
gboolean qof_begin_edit (QofInstance * inst)
{
	QofIdTypeConst type;
	QofEntity * ent;

	param_flag = TRUE;
	ent = &inst->entity;
	type = ent->e_type;
	qof_class_param_foreach (type, param_edit_cb, inst);
	return param_flag;
}
gboolean
qof_commit_edit (QofInstance * inst)
{
	QofIdTypeConst type;
	QofEntity * ent;

	param_flag = TRUE;
	ent = &inst->entity;
	type = ent->e_type;
	qof_class_param_foreach (type, param_commit_cb, inst);
	return param_flag;
}
gboolean
qof_commit_edit_part2 (QofInstance * inst,
	void G_GNUC_UNUSED (*on_error) (QofInstance *, QofBackendError),
	void (*on_done) (QofInstance *), void (*on_free) (QofInstance *))
{
	QofBackend *be;

	/* See if there's a backend.  If there is, invoke it. */
	be = qof_book_get_backend (inst->book);
	if (be)
	{
		QofErrorId errcode;

		errcode = qof_backend_get_error (be);
		if (ERR_BACKEND_NO_ERR != errcode)
		{
			inst->do_free = FALSE;

			qof_backend_set_error (be, errcode);
			if (on_error)
				on_error (inst, errcode);
			return FALSE;
		}
		inst->dirty = FALSE;
	}
    if (inst->do_free) {
        if (on_free)
            on_free(inst);
        return TRUE;
    }
    if (on_done)
        on_done(inst);
    return TRUE;
}
gchar *
qof_util_param_as_string (QofEntity * ent, QofParam * param)
{
	return qof_util_param_to_string (ent, param);
}
gnc_numeric 
double_to_gnc_numeric (double in, gint64 denom, gint how)
{
	return qof_numeric_from_double (in, denom, how);
}
gboolean 
string_to_gnc_numeric (const gchar * str, gnc_numeric * n)
{
	return qof_numeric_from_string (str, n);
}
gnc_numeric 
gnc_numeric_error (GNCNumericErrorCode error_code)
{
	return qof_numeric_error (error_code);
}
gdouble 
gnc_numeric_to_double (gnc_numeric in)
{
	return qof_numeric_to_double (in);
}
gchar *
gnc_numeric_to_string (gnc_numeric n)
{
	return qof_numeric_to_string (n);
}
gchar *
gnc_num_dbg_to_string (gnc_numeric n)
{
	return qof_numeric_dbg_to_string (n);
}
GNCNumericErrorCode 
gnc_numeric_check (gnc_numeric a)
{
	return qof_numeric_check (a);
}
gint 
gnc_numeric_compare (gnc_numeric a, gnc_numeric b)
{
	return qof_numeric_compare (a, b);
}
gboolean 
gnc_numeric_zero_p (gnc_numeric a)
{
	return qof_numeric_zero_p (a);
}
gboolean 
gnc_numeric_negative_p (gnc_numeric a)
{
	return qof_numeric_negative_p (a);
}
gboolean 
gnc_numeric_positive_p (gnc_numeric a)
{
	return qof_numeric_positive_p (a);
}
gboolean 
gnc_numeric_eq (gnc_numeric a, gnc_numeric b)
{
	return qof_numeric_eq (a, b);
}
gboolean 
gnc_numeric_equal (gnc_numeric a, gnc_numeric b)
{
	return qof_numeric_equal (a, b);
}
gint 
gnc_numeric_same (gnc_numeric a, gnc_numeric b, 
				  gint64 denom, gint how)
{
	return qof_numeric_same (a, b, denom, how);
}
gnc_numeric 
gnc_numeric_add (gnc_numeric a, gnc_numeric b,
				 gint64 denom, gint how)
{
	return qof_numeric_add (a, b, denom, how);
}
gnc_numeric 
gnc_numeric_sub (gnc_numeric a, gnc_numeric b,
				 gint64 denom, gint how)
{
	return qof_numeric_sub (a, b, denom, how);
}
gnc_numeric 
gnc_numeric_mul (gnc_numeric a, gnc_numeric b,
				 gint64 denom, gint how)
{
	return qof_numeric_mul (a, b, denom, how);
}
gnc_numeric 
gnc_numeric_div (gnc_numeric x, gnc_numeric y,
				 gint64 denom, gint how)
{
	return qof_numeric_div (x, y, denom, how);
}
gnc_numeric gnc_numeric_neg (gnc_numeric a)
{
	return qof_numeric_neg (a);
}
gnc_numeric gnc_numeric_abs (gnc_numeric a)
{
	return qof_numeric_abs (a);
}
gnc_numeric 
gnc_numeric_add_with_error (gnc_numeric a, gnc_numeric b,
							gint64 denom, gint how,
							gnc_numeric * error)
{
	return qof_numeric_add_with_error (a, b, denom, how, error);
}
gnc_numeric 
gnc_numeric_sub_with_error (gnc_numeric a, gnc_numeric b,
							gint64 denom, gint how,
							gnc_numeric * error)
{
	return qof_numeric_sub_with_error (a, b, denom, how, error);
}
gnc_numeric 
gnc_numeric_mul_with_error (gnc_numeric a, gnc_numeric b,
							gint64 denom, gint how,
							gnc_numeric * error)
{
	return qof_numeric_mul_with_error (a, b, denom, how, error);
}
gnc_numeric 
gnc_numeric_div_with_error (gnc_numeric a, gnc_numeric b,
							gint64 denom, gint how,
							gnc_numeric * error)
{
	return qof_numeric_div_with_error (a, b, denom, how, error);
}

gnc_numeric 
gnc_numeric_convert (gnc_numeric in, gint64 denom, gint how)
{
	return qof_numeric_convert (in, denom, how);
}

gnc_numeric gnc_numeric_reduce (gnc_numeric in)
{
	return qof_numeric_reduce (in);
}
void
qof_session_push_error (QofSession * session, QofBackendError err,
	const gchar *message)
{
	if (!session)
		return;
	qof_error_set (session, qof_error_register (message, FALSE));
}
const gchar *
qof_session_get_error_message (QofSession * session)
{
	if (!session)
		return "";
	if (!session->error_message)
		return "";
	return session->error_message;
}
QofErrorId
qof_session_pop_error (QofSession * session)
{
	if (!session)
		return QOF_FATAL;
	return qof_error_get_id (session);
}
QofErrorId
qof_session_get_error (QofSession * session)
{
	QofErrorId err;

	if (!session)
		return ERR_BACKEND_NO_BACKEND;

	/* if we have a local error, return that. */
	if (ERR_BACKEND_NO_ERR != session->last_err)
	{
		return session->last_err;
	}

	/* maybe we should return a no-backend error ??? */
	if (!session->backend)
		return ERR_BACKEND_NO_ERR;

	err = qof_backend_get_error (session->backend);
	session->last_err = err;
	return err;
}
void
qof_backend_set_error (QofBackend * be, QofErrorId err)
{
	if (!be)
		return;
	qof_error_set_be (be, err);
}
QofErrorId
qof_backend_get_error (QofBackend * be)
{
	QofErrorId err;
	if (!be)
		return ERR_BACKEND_NO_BACKEND;

	/* use 'stack-pop' semantics */
	err = be->last_err;
	be->last_err = ERR_BACKEND_NO_ERR;
	return err;
}
void
qof_backend_set_message (QofBackend * be, const gchar * format, ...)
{
	va_list args;
	gchar *buffer;

	if (!be)
		return;

	/* If there's already something here, free it */
	if (be->error_msg)
		g_free (be->error_msg);

	if (!format)
	{
		be->error_msg = NULL;
		return;
	}

	va_start (args, format);
	buffer = (gchar *) g_strdup_vprintf (format, args);
	va_end (args);

	be->error_msg = buffer;
}
gchar *
qof_backend_get_message (QofBackend * be)
{
	if (!be)
		return g_strdup ("ERR_BACKEND_NO_BACKEND");
	if (!be->error_msg)
		return NULL;

	return g_strdup(qof_error_get_message_be (be));
}

/* qof_backend_set_error with old values but no strings.
*/

AS_STRING_FUNC(QofBackendError, ENUM_LIST_DEP)

void
set_deprecated_errors (void)
{
	QofErrorId err;

	for (err = 0;err < ERR_LAST; err++)
	{
		switch (err)
		{
			case ERR_BACKEND_NO_ERR:
			{
				break;
			}
			case ERR_BACKEND_NO_HANDLER:
			case ERR_BACKEND_NO_BACKEND:
			case ERR_BACKEND_BAD_URL:
			case ERR_BACKEND_CANT_CONNECT:
			case ERR_BACKEND_CONN_LOST:
			case ERR_BACKEND_TOO_NEW:
			case ERR_BACKEND_NO_SUCH_DB:
			case ERR_BACKEND_LOCKED:
			case ERR_BACKEND_READONLY:
			case ERR_BACKEND_DATA_CORRUPT:
			case ERR_BACKEND_SERVER_ERR:
			case ERR_BACKEND_PERM:
			case ERR_BACKEND_MISC:
			case ERR_QSF_INVALID_OBJ:
			case ERR_QSF_INVALID_MAP:
			case ERR_QSF_BAD_QOF_VERSION:
			case ERR_QSF_BAD_MAP:
			case ERR_QSF_BAD_OBJ_GUID:
			case ERR_QSF_NO_MAP:
			case ERR_QSF_WRONG_MAP:
			case ERR_QSF_MAP_NOT_OBJ:
			case ERR_QSF_OVERFLOW:
			case ERR_QSF_OPEN_NOT_MERGE:
			case ERR_FILEIO_FILE_BAD_READ:
			case ERR_FILEIO_PARSE_ERROR:
			case ERR_FILEIO_FILE_EMPTY:
			case ERR_FILEIO_FILE_NOT_FOUND:
			case ERR_FILEIO_FILE_TOO_OLD:
			case ERR_FILEIO_UNKNOWN_FILE_TYPE:
			case ERR_FILEIO_BACKUP_ERROR:
			case ERR_FILEIO_WRITE_ERROR:
			case ERR_SQL_DB_TOO_OLD:
			case ERR_SQL_DB_BUSY:
			{
				deprecated_support (err, QofBackendErrorasString(err));
				break;
			}
		default:
			break;
		}
	}
}

gint double_compare (gdouble d1, gdouble d2)
{
	return qof_util_double_compare (d1, d2);
}

/* ==================================================================== */
#endif /* QOF_DISABLE_DEPRECATED */
