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
#include "qofdate-p.h"

static QofLogModule log_module = QOF_MOD_TIME;

struct QofTime64
{
	QofTimeSecs qt_sec;
	glong qt_nsec;
	gboolean valid;
};

QofTime *
qof_time_new (void)
{
	QofTime *qt;

	qt = g_new0 (QofTime, 1);
	qt->valid = FALSE;
	return qt;
}

void
qof_time_free (QofTime * qt)
{
	if (qt == NULL)
		return;
	g_free (qt);
	qt = NULL;
}

void
qof_time_add_secs (QofTime * qt, QofTimeSecs secs)
{
	g_return_if_fail (qt);
	g_return_if_fail (qt->valid);
	qt->qt_sec += secs;
}

QofTime *
qof_time_add_secs_copy (QofTime * qt, QofTimeSecs secs)
{
	QofTime *copy;

	g_return_val_if_fail (qt, NULL);
	g_return_val_if_fail (qt->valid, NULL);
	copy = qof_time_copy (qt);
	copy->qt_sec += secs;
	return copy;
}

static QofTime *
time_normalize (QofTime * qt)
{
	g_return_val_if_fail (qt->valid, NULL);
	if ((qt->qt_sec < 0) && (qt->qt_nsec > QOF_NSECS))
	{
		qt->qt_sec -= (qt->qt_nsec / QOF_NSECS);
		qt->qt_nsec = qt->qt_nsec % QOF_NSECS;
	}
	if ((qt->qt_sec >= 0) && (qt->qt_nsec > QOF_NSECS))
	{
		qt->qt_sec += (qt->qt_nsec / QOF_NSECS);
		qt->qt_nsec = qt->qt_nsec % QOF_NSECS;
	}
	if ((qt->qt_sec < 0) && (qt->qt_nsec < -QOF_NSECS))
	{
		qt->qt_sec -= -(-qt->qt_nsec / QOF_NSECS);
		qt->qt_nsec = -(-qt->qt_nsec % QOF_NSECS);
	}
	if ((qt->qt_sec >= 0) && (qt->qt_nsec < -QOF_NSECS))
	{
		qt->qt_sec += -(-qt->qt_nsec / QOF_NSECS);
		qt->qt_nsec = -(-qt->qt_nsec % QOF_NSECS);
	}
	if (qt->qt_sec >= 0 && qt->qt_nsec < 0)
	{
		qt->qt_sec--;
		qt->qt_nsec = QOF_NSECS + qt->qt_nsec;
	}
	return qt;
}

void
qof_time_set_secs (QofTime * qt, QofTimeSecs secs)
{
	qt->qt_sec = secs;
	qt->valid = TRUE;
	time_normalize (qt);
}

void
qof_time_set_nanosecs (QofTime * qt, glong nano)
{
	qt->qt_nsec = nano;
	qt->valid = TRUE;
	time_normalize (qt);
}

QofTimeSecs
qof_time_get_secs (const QofTime * qt)
{
	g_return_val_if_fail (qt, 0);
	g_return_val_if_fail (qt->valid == TRUE, 0);
	return qt->qt_sec;
}

glong
qof_time_get_nanosecs (const QofTime * qt)
{
	g_return_val_if_fail (qt->valid == TRUE, 0);
	return qt->qt_nsec;
}

gboolean
qof_time_equal (const QofTime * ta, const QofTime * tb)
{
	if (ta == tb)
		return TRUE;
	if (!ta)
		return FALSE;
	if (!tb)
		return FALSE;
	g_return_val_if_fail (ta->valid && tb->valid, FALSE);
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
qof_time_abs (QofTime * qt)
{
	g_return_val_if_fail (qt, NULL);
	return time_normalize (qt);
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
qof_time_copy (const QofTime *qt)
{
	g_return_val_if_fail (qt, NULL);
	g_return_val_if_fail (qt->valid, NULL);
	return qof_time_set (qt->qt_sec, qt->qt_nsec);
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
qof_time_from_tm (struct tm * qtm, glong nanosecs)
{
	QofDate *qd;
	QofTime *qt;

	/* avoids use of gmtime_r and therefore time_t */
	qd = qof_date_from_struct_tm (qtm);
	qd->qd_nanosecs = nanosecs;
	qt = qof_date_to_qtime (qd);
	qof_date_free (qd);
	return qt;
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
qof_time_to_gdate (QofTime * qt)
{
	QofDate *qd;
	GDate *d;

	qd = qof_date_from_qtime (qt);
	d = g_date_new_dmy (qd->qd_mday, qd->qd_mon, qd->qd_year);
	if (g_date_valid (d))
		return d;
	return NULL;
}

QofTime *
qof_time_from_gdate (GDate * date)
{
	struct tm gtm;
	QofTime *qt;
	QofDate *qd;

	g_return_val_if_fail (date, NULL);
	g_date_to_struct_tm (date, &gtm);
	qd = qof_date_from_struct_tm (&gtm);
	qt = qof_date_to_qtime (qd);
	qof_date_free (qd);
	return qt;
}

gboolean
qof_time_set_day_end (QofTime * qt)
{
	if (!qof_time_set_day_start (qt))
		return FALSE;
	qt->qt_sec += (SECS_PER_DAY - 1);
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
	QofDate *qd;
	QofTimeSecs c;
	
	g_return_val_if_fail (qt, FALSE);
	qd = qof_date_from_qtime (qt);
	if (qd->qd_year < 1970)
	{
		c = QOF_DAYS_TO_SEC(qd->qd_yday);
		c -= QOF_DAYS_TO_SEC(days_between (1970, qd->qd_year));
		c -= qd->qd_gmt_off;
		qt->qt_sec = c;
		qt->qt_nsec = 0;
	}
	if (qd->qd_year >= 1970)
	{
		c = QOF_DAYS_TO_SEC(qd->qd_yday);
		c += QOF_DAYS_TO_SEC(days_between (1970, qd->qd_year));
		c -= qd->qd_gmt_off;
		qt->qt_sec = c;
		qt->qt_nsec = 0;
	}
	qof_date_free (qd);
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
qof_time_last_mday (QofTime * qt)
{
	GDate *d;
	GDateMonth m;
	GDateYear y;

	g_return_val_if_fail (qt, 0);
	d = qof_time_to_gdate (qt);
	if (!d)
		return 0;
	m = g_date_get_month (d);
	y = g_date_get_year (d);
	return g_date_get_days_in_month (m, y);
}

gboolean
qof_time_to_dmy (QofTime * qt, guint8 * day, guint8 * month, 
				 guint16 * year)
{
	GDate *d;

	d = qof_time_to_gdate (qt);
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
	struct tm qtm;
	time_t t;
	gchar test[MAX_DATE_LENGTH];
	const gchar *fmt;

	ENTER (" ");
	t = time (NULL);
	qtm = *gmtime_r (&t, &qtm);
	fmt = qof_date_format_get_format (QOF_DATE_FORMAT_UTC);
	len = strftime (test, MAX_DATE_LENGTH, fmt, &qtm);
	if (len == 0 && test[0] != '\0')
	{
		LEAVE (" strftime failed.");
		return NULL;
	}
	LEAVE (" ");
	return g_strdup (test);
}
