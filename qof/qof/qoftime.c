/********************************************************************
 *       qoftime.c - QofTime, 64bit UTC time handling (seconds).
 *       Rewritten from scratch for QOF 0.7.0
 *
 *  Fri May  5 15:05:24 2006
 *  Copyright  2006  Neil Williams
 *  linux@codehelp.co.uk
 ********************************************************************/
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA  02110-1301,  USA
 */

#include "config.h"
#include <glib.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qof.h"

static QofLogModule log_module = QOF_MOD_TIME;

struct QofTimespec64
{
	QofTimeSecs qt_sec;
	glong qt_nsec;
	gboolean valid;
};

QofTime *
qof_time_new (void)
{
	QofTime *time;

	time = g_new0 (QofTime, 1);
	return time;
}

void
qof_time_free (QofTime * qt)
{
	if (!qt)
		return;
	g_free (qt);
	qt = NULL;
}

QofTime *
qof_time_add_secs (QofTime * time, QofTimeSecs secs)
{
	g_return_val_if_fail (time->valid, NULL);
	time->qt_sec += secs;
	return time;
}

static QofTime *
time_normalize (QofTime * time)
{
	g_return_val_if_fail (time->valid, NULL);
	if ((time->qt_sec < 0) && (time->qt_nsec > QOF_NSECS))
	{
		time->qt_sec -= (time->qt_nsec / QOF_NSECS);
		time->qt_nsec = time->qt_nsec % QOF_NSECS;
	}
	if ((time->qt_sec >= 0) && (time->qt_nsec > QOF_NSECS))
	{
		time->qt_sec += (time->qt_nsec / QOF_NSECS);
		time->qt_nsec = time->qt_nsec % QOF_NSECS;
	}
	if ((time->qt_sec < 0) && (time->qt_nsec < -QOF_NSECS))
	{
		time->qt_sec -= -(-time->qt_nsec / QOF_NSECS);
		time->qt_nsec = -(-time->qt_nsec % QOF_NSECS);
	}
	if ((time->qt_sec >= 0) && (time->qt_nsec < -QOF_NSECS))
	{
		time->qt_sec += -(-time->qt_nsec / QOF_NSECS);
		time->qt_nsec = -(-time->qt_nsec % QOF_NSECS);
	}
	if (time->qt_sec >= 0 && time->qt_nsec < 0)
	{
		time->qt_sec--;
		time->qt_nsec = QOF_NSECS + time->qt_nsec;
	}
	if (time->qt_sec < 0 && time->qt_nsec > 0)
	{
		time->qt_sec++;
		time->qt_nsec = -QOF_NSECS + time->qt_nsec;
	}
	return time;
}

void
qof_time_set_secs (QofTime * time, QofTimeSecs secs)
{
	time->qt_sec = secs;
	time->valid = TRUE;
	time_normalize (time);
}

void
qof_time_set_nanosecs (QofTime * time, glong nano)
{
	time->qt_nsec = nano;
	time->valid = TRUE;
	time_normalize (time);
}

QofTimeSecs
qof_time_get_secs (const QofTime * time)
{
	g_return_val_if_fail (time->valid == TRUE, 0);
	return time->qt_sec;
}

glong
qof_time_get_nanosecs (const QofTime * time)
{
	g_return_val_if_fail (time->valid == TRUE, 0);
	return time->qt_nsec;
}

gboolean
qof_time_equal (const QofTime * ta, const QofTime * tb)
{
	g_return_val_if_fail (ta->valid && tb->valid, FALSE);
	if (ta == tb)
		return TRUE;
	if (ta->qt_sec != tb->qt_sec)
		return FALSE;
	if (ta->qt_nsec != tb->qt_nsec)
		return FALSE;
	return TRUE;
}

gint
qof_time_cmp (const QofTime * ta, const QofTime * tb)
{
	g_return_val_if_fail (ta->valid && tb->valid, -1);
	if (ta == tb)
		return 0;
	if (ta->qt_sec < tb->qt_sec)
		return -1;
	if (ta->qt_sec > tb->qt_sec)
		return 1;
	if (ta->qt_nsec < tb->qt_nsec)
		return -1;
	if (ta->qt_nsec > tb->qt_nsec)
		return 1;
	return 0;
}

QofTime *
qof_time_diff (const QofTime * ta, const QofTime * tb)
{
	QofTime *retval;

	g_return_val_if_fail (ta->valid && tb->valid, NULL);
	retval = g_new0 (QofTime, 1);
	retval->qt_sec = ta->qt_sec - tb->qt_sec;
	retval->qt_nsec = ta->qt_nsec - tb->qt_nsec;
	retval->valid = TRUE;
	time_normalize (retval);
	return retval;
}

QofTime *
qof_time_abs (QofTime * time)
{
	g_return_val_if_fail (time, NULL);
	return time_normalize (time);
}

gboolean
qof_time_is_valid (const QofTime * qt)
{
	g_return_val_if_fail (qt, FALSE);
	return qt->valid;
}

QofTime *
qof_time_set (QofTimeSecs t, glong nanosecs)
{
	QofTime *qt;

	qt = qof_time_new ();
	qt->qt_sec = t;
	qt->qt_nsec = nanosecs;
	qt->valid = TRUE;
	time_normalize (qt);
	return qt;
}

QofTime *
qof_time_from_time_t (time_t t, glong nanosecs)
{
	return qof_time_set (t, nanosecs);
}

gboolean
qof_time_to_time_t (QofTime * qt, time_t * t, glong * nanosecs)
{
	if (!qt->valid)
		return FALSE;
	if (qt->qt_sec < 0)
		return FALSE;
	if (qt->qt_nsec > 0)
	{
		*nanosecs = qt->qt_nsec;
	}
	if ((sizeof (qt->qt_sec) > sizeof (time_t))
		&& (qt->qt_sec > G_MAXINT32))
	{
		PERR (" QofTime too large for time_t on this platform.");
		return FALSE;
	}
	*t = qt->qt_sec;
	return TRUE;
}

QofTime *
qof_time_from_tm (struct tm * tm, glong nanosecs)
{
	GDate *d;
	QofTime *time;

	/* avoids use of gmtime_r and therefore time_t */
	d = g_date_new_dmy (tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900);
	time = qof_time_from_gdate (d);
	time->qt_sec += tm->tm_hour * 60 * 60;
	time->qt_sec += tm->tm_min * 60;
	time->qt_sec += tm->tm_sec;
	time->qt_nsec = nanosecs;
	return time;
}

gboolean
qof_time_to_gtimeval (QofTime * qt, GTimeVal * gtv)
{
	if (!qt->valid)
	{
		PERR (" invalid QofTime passed");
		return FALSE;
	}
	if (qt->qt_sec > G_MAXLONG)
	{
		PERR (" QofTime out of range for GTimeVal");
		return FALSE;
	}
	gtv->tv_sec = (glong) qt->qt_sec;
	gtv->tv_usec = qt->qt_nsec;
	return TRUE;
}

void
qof_time_from_gtimeval (QofTime * qt, GTimeVal * gtv)
{
	qt->qt_sec = (QofTimeSecs) gtv->tv_sec;
	qt->qt_nsec = gtv->tv_usec * 1000;
	qt->valid = TRUE;
	time_normalize (qt);
}

GDate *
qof_time_to_gdate (QofTime * time)
{
	GDate *d;
	time_t t;
	glong nsecs;
	gboolean success;
	struct tm utc;

	/** \todo replace with qofstrftime ( [qof]strptime actually)
	%Y for 2006 == (tm_year)
	%m for 05   == (tm_mon + 1)
	%d for 22
	to avoid range problems with time_t
	then g_date_set_parse (GDate, gchar*)
	gchar* str = "%d/%m/%Y";
	
	*/
	success = qof_time_to_time_t (time, &t, &nsecs);
	if (!success)
		return NULL;
	utc = *gmtime_r (&t, &utc);
	d = g_date_new_dmy (utc.tm_mday, utc.tm_mon + 1, utc.tm_year + 1900);
	if (g_date_valid (d))
		return d;
	return NULL;
}

QofTime *
qof_time_from_gdate (GDate * date)
{
	GTimeVal *current, from_date;
	GDate *now;
	gint days_between;
	gint64 secs_between;
	QofTime *qt;

	g_return_val_if_fail (date, NULL);
	current = qof_time_get_current_start ();
	now = g_date_new ();
	g_date_set_time_val (now, current);
	/* if date is in the future, days_between is negative */
	days_between = g_date_days_between (date, now);
	qt = qof_time_new ();
	qof_time_from_gtimeval (qt, &from_date);
	secs_between = days_between * SECS_PER_DAY;
	qof_time_set_secs (qt, current->tv_sec - secs_between);
	qof_time_set_nanosecs (qt, 0);
	return qt;
}

gboolean
qof_time_set_day_end (QofTime * time)
{
	if (!qof_time_set_day_start (time))
		return FALSE;
	time->qt_sec += (SECS_PER_DAY - 1);
	return TRUE;
}

gboolean
qof_time_set_day_middle (QofTime * qt)
{
	if (!qof_time_set_day_start (qt))
		return FALSE;
	qt->qt_sec += (SECS_PER_DAY / 2);
	return TRUE;
}

GTimeVal *
qof_time_get_current_start (void)
{
	GTimeVal *current;
	struct tm tm;

	/** \todo replace with QofDate */
	current = g_new0 (GTimeVal, 1);
	g_get_current_time (current);
	/* OK to use time_t for current time. */
	tm = *gmtime_r (&current->tv_sec, &tm);
	current->tv_sec -= tm.tm_sec;
	current->tv_sec -= tm.tm_min * 60;
	current->tv_sec -= tm.tm_hour * 60 * 60;
	return current;
}

QofTime *
qof_time_get_current (void)
{
	QofTime *now;
	GTimeVal gnow;

	now = qof_time_new ();
	g_get_current_time (&gnow);
	qof_time_from_gtimeval (now, &gnow);
	return now;
}

gboolean
qof_time_set_day_start (QofTime * qt)
{
	GDate *d, *now;
	GTimeVal *current, from_date;
	gint days_between;

	/** \todo convert to QofDate */
	g_return_val_if_fail (qt, FALSE);
	d = qof_time_to_gdate (qt);
	if (!d)
		return FALSE;
	now = g_date_new ();
	current = qof_time_get_current_start ();
	g_date_set_time_val (now, current);
	/* if date is in the future, days_between is negative */
	days_between = g_date_days_between (d, now);
	from_date.tv_sec = current->tv_sec - (days_between * SECS_PER_DAY);
	from_date.tv_usec = 0;
	qof_time_from_gtimeval (qt, &from_date);
	g_date_free (d);
	g_free (current);
	return TRUE;
}

QofTime *
qof_time_get_today_start (void)
{
	QofTime *qt;
	GDate *d;
	GTimeVal val;

	qt = qof_time_new ();
	g_get_current_time (&val);
	d = g_date_new ();
	g_date_set_time_val (d, &val);
	qt = qof_time_from_gdate (d);
	return qt;
}

QofTime *
qof_time_get_today_end (void)
{
	QofTime *qt;

	qt = qof_time_get_today_start ();
	qt->qt_sec += SECS_PER_DAY - 1;
	return qt;
}

guint8
qof_time_last_mday (QofTime * t)
{
	GDate *d;
	GDateMonth m;
	GDateYear y;

	g_return_val_if_fail (t, 0);
	d = qof_time_to_gdate (t);
	if (!d)
		return 0;
	m = g_date_get_month (d);
	y = g_date_get_year (d);
	return g_date_get_days_in_month (m, y);
}

gboolean
qof_time_to_dmy (QofTime * t, guint8 * day, guint8 * month, guint16 * year)
{
	GDate *d;

	d = qof_time_to_gdate (t);
	if (!d)
		return FALSE;
	if (day)
		*day = g_date_get_day (d);
	if (month)
		*month = g_date_get_month (d);
	if (year)
		*year = g_date_get_year (d);
	return TRUE;
}

QofTime *
qof_time_dmy_to_time (guint8 day, guint8 month, guint16 year)
{
	GDate *d;
	QofTime *qt;

	g_return_val_if_fail (g_date_valid_dmy (day, month, year), NULL);
	d = g_date_new_dmy (day, month, year);
	qt = qof_time_from_gdate (d);
	return qt;
}

gchar *
qof_time_stamp_now (void)
{
	gint len;
	struct tm tm;
	time_t t;
	gchar test[MAX_DATE_LENGTH];
	const gchar *fmt;

	ENTER (" ");
	t = time (NULL);
	tm = *gmtime_r (&t, &tm);
	fmt = qof_date_format_get_format (QOF_DATE_FORMAT_UTC);
	len = strftime (test, MAX_DATE_LENGTH, fmt, &tm);
	if (len == 0 && test[0] != '\0')
	{
		LEAVE (" strftime failed.");
		return NULL;
	}
	LEAVE (" ");
	return g_strdup (test);
}
