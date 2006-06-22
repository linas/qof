/********************************************************************
 *       qofdate.c - QofDate, 64bit UTC date handling.
 *       Rewritten from scratch for QOF 0.7.0
 *
 *  Fri May  5 15:05:24 2006
 *  Copyright (C) 1991, 1993, 1997, 1998, 2002, 2006
 *  Free Software Foundation, Inc.
 *  This file contains routines modified from the GNU C Library.
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
#include <glib/gprintf.h>
#include <time.h>
#include "qof.h"
#include "qofdate-p.h"

/* from gnu libc */
#define DIV(a, b) ((a) / (b) - ((a) % (b) < 0))
#define LEAPS_THRU_END_OF(y) (DIV (y, 4) - DIV (y, 100) + DIV (y, 400))

static GHashTable *DateFormatTable = NULL;
static gboolean QofDateInit = FALSE;
static QofLogModule log_module = QOF_MOD_DATE;
static gchar locale_separator = '\0';
static QofDateFormat dateFormat = QOF_DATE_FORMAT_UTC;

/* copied from glib */
static const guint16 days_in_year[2][14] = 
{  /* 0, jan feb mar apr may  jun  jul  aug  sep  oct  nov  dec */
  {  0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 }, 
  {  0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};
static const guint8 days_in_months[2][13] =
{  /* error, jan feb mar apr may jun jul aug sep oct nov dec */
  {  0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
  {  0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 } /* leap year */
};

/* A single Date Format Entry. */
typedef struct QofDateEntry_s
{
	const gchar *format;
	const gchar *name;
	gchar separator;
	QofDateFormat df;
	gboolean locale_specific;
} QofDateEntry;

void
qof_date_init (void)
{
	if (!QofDateInit)
	{
		DateFormatTable = g_hash_table_new (g_direct_hash, g_direct_equal);
	}
	{
		QofDateEntry *d = g_new0 (QofDateEntry, 1);
		d->format = "%m/%d/%Y";
		d->name = "us";
		d->separator = '/';
		d->df = QOF_DATE_FORMAT_US;
		d->locale_specific = FALSE;
		g_hash_table_insert (DateFormatTable, GINT_TO_POINTER (d->df), d);
	}
	{
		QofDateEntry *d = g_new0 (QofDateEntry, 1);
		d->format = "%d/%m/%Y";
		d->name = "uk";
		d->separator = '/';
		d->df = QOF_DATE_FORMAT_UK;
		d->locale_specific = FALSE;
		g_hash_table_insert (DateFormatTable, GINT_TO_POINTER (d->df), d);
	}
	{
		QofDateEntry *d = g_new0 (QofDateEntry, 1);
		d->format = "%d.%m.%Y";
		d->name = "ce";
		d->separator = '.';
		d->df = QOF_DATE_FORMAT_CE;
		d->locale_specific = FALSE;
		g_hash_table_insert (DateFormatTable, GINT_TO_POINTER (d->df), d);
	}
	{
		QofDateEntry *d = g_new0 (QofDateEntry, 1);
		d->format = "%F";
		d->name = "iso";
		d->separator = '-';
		d->df = QOF_DATE_FORMAT_ISO;
		d->locale_specific = FALSE;
		g_hash_table_insert (DateFormatTable, GINT_TO_POINTER (d->df), d);
	}
	{
		QofDateEntry *d = g_new0 (QofDateEntry, 1);
		d->format = QOF_UTC_DATE_FORMAT;
		d->name = "utc";
		d->separator = '-';
		d->df = QOF_DATE_FORMAT_UTC;
		d->locale_specific = FALSE;
		g_hash_table_insert (DateFormatTable, GINT_TO_POINTER (d->df), d);
	}
	{
		QofDateEntry *d = g_new0 (QofDateEntry, 1);
		d->format = "%x";
		d->name = "locale";
		d->separator = locale_separator;
		d->df = QOF_DATE_FORMAT_LOCALE;
		d->locale_specific = TRUE;
		g_hash_table_insert (DateFormatTable, GINT_TO_POINTER (d->df), d);
	}
	{
		QofDateEntry *d = g_new0 (QofDateEntry, 1);
		d->format = "%c";
		d->name = "custom";
		d->separator = locale_separator;
		d->df = QOF_DATE_FORMAT_CUSTOM;
		d->locale_specific = TRUE;
		g_hash_table_insert (DateFormatTable, GINT_TO_POINTER (d->df), d);
	}
	QofDateInit = TRUE;
}

void
qof_date_close (void)
{
	if (QofDateInit)
	{
		g_hash_table_destroy (DateFormatTable);
	}
	QofDateInit = FALSE;
}

gboolean
qof_date_format_add (const gchar * str, QofDateFormat identifier)
{
	struct tm check;
	gint len;
	time_t now;
	gchar test[MAX_DATE_BUFFER];

	/** \todo Move to QofDate and qofgmtime_r */
	g_return_val_if_fail (QofDateInit, FALSE);
	g_return_val_if_fail (str, FALSE);
	g_return_val_if_fail (strlen (str) != 0, FALSE);
	/* prevent really long strings being passed */
	ENTER (" str=%s", str);
	if (strlen (str) > MAX_DATE_LENGTH)
	{
		LEAVE (" '%s' is too long! Max=%d str_len=%d",
			str, MAX_DATE_LENGTH, (gint) strlen (str));
		return FALSE;
	}
	/* test the incoming string using the current time. */
	now = time (NULL);
	test[0] = '\1';
	check = *gmtime_r (&now, &check);
	/* need to allow time related formats - 
	don't use g_date_strftime here. */
	len = strftime (test, (MAX_DATE_BUFFER - 1), str, &check);
	if (len == 0 && test[0] != '\0')
	{
		LEAVE (" strftime could not understand '%s'", str);
		return FALSE;
	}
	len = strlen (test);
	if (len > MAX_DATE_LENGTH)
	{
		LEAVE (" %s creates a string '%s' that is too long!"
			" Max=%d str_len=%d", str, test, MAX_DATE_LENGTH, len);
		return FALSE;
	}
	{
		QofDateEntry *d = g_new0 (QofDateEntry, 1);
		d->format = str;
		d->name = str;
		d->separator = locale_separator;
		d->df = identifier;
		g_hash_table_insert (DateFormatTable, GINT_TO_POINTER (d->df), d);
	}
	LEAVE (" successful");
	return TRUE;
}

const gchar *
qof_date_format_to_name (QofDateFormat format)
{
	QofDateEntry *d;

	g_return_val_if_fail (QofDateInit, NULL);
	d = g_hash_table_lookup (DateFormatTable, GINT_TO_POINTER (format));
	if (!d)
	{
		PERR (" unknown format: '%d'", format);
		return NULL;
	}
	return d->name;
}

gboolean
qof_date_format_set_name (const gchar * name, QofDateFormat format)
{
	QofDateEntry *d;

	g_return_val_if_fail (QofDateInit, FALSE);
	if (format <= DATE_FORMAT_LAST)
		return FALSE;
	d = g_hash_table_lookup (DateFormatTable, GINT_TO_POINTER (format));
	if (!d)
	{
		PERR (" unknown format: '%d'", format);
		return FALSE;
	}
	d->name = name;
	g_hash_table_insert (DateFormatTable, GINT_TO_POINTER (format), d);
	return TRUE;
}
QofDateFormat
qof_date_format_get_current (void)
{
	return dateFormat;
}

gboolean
qof_date_format_set_current (QofDateFormat df)
{
	QofDateEntry *d;

	g_return_val_if_fail (QofDateInit, FALSE);
	d = g_hash_table_lookup (DateFormatTable, GINT_TO_POINTER (df));
	if (!d)
	{
		PERR (" unknown format: '%d'", df);
		return FALSE;
	}
	dateFormat = d->df;
	return TRUE;
}

const gchar *
qof_date_format_get_format (QofDateFormat df)
{
	QofDateEntry *d;

	g_return_val_if_fail (QofDateInit, NULL);
	d = g_hash_table_lookup (DateFormatTable, GINT_TO_POINTER (df));
	if (!d)
	{
		PERR (" unknown format: '%d'", df);
		return NULL;
	}
	return d->format;
}

gchar
qof_date_format_get_date_separator (QofDateFormat df)
{
	QofDateEntry *d;

	g_return_val_if_fail (QofDateInit, locale_separator);
	d = g_hash_table_lookup (DateFormatTable, GINT_TO_POINTER (df));
	if (!d)
	{
		PERR (" unknown format: '%d'", df);
		return locale_separator;
	}
	return d->separator;
}

gboolean
qof_date_format_set_date_separator (const gchar sep, QofDateFormat df)
{
	QofDateEntry *d;

	g_return_val_if_fail (QofDateInit, FALSE);
	if (df < DATE_FORMAT_LAST)
	{
		DEBUG (" Prevented attempt to override a default format");
		return FALSE;
	}
	if (g_ascii_isdigit (sep))
		return FALSE;
	d = g_hash_table_lookup (DateFormatTable, GINT_TO_POINTER (df));
	if (!d)
	{
		PERR (" unknown format: '%d'", df);
		return FALSE;
	}
	d->separator = sep;
	g_hash_table_insert (DateFormatTable, GINT_TO_POINTER (df), d);
	return TRUE;
}

struct iter
{
	const gchar *name;
	QofDateFormat df;
};

static void
lookup_name (gpointer key, gpointer value, gpointer data)
{
	struct iter *i;
	QofDateEntry *d;

	i = (struct iter *) data;
	d = (QofDateEntry *) value;
	if (0 == safe_strcmp (d->name, i->name))
	{
		i->df = d->df;
	}
}

QofDateFormat
qof_date_format_from_name (const gchar * name)
{
	struct iter i;

	if (!name)
		return -1;
	if (0 == safe_strcmp (name, "us"))
		return QOF_DATE_FORMAT_US;
	if (0 == safe_strcmp (name, "uk"))
		return QOF_DATE_FORMAT_UK;
	if (0 == safe_strcmp (name, "ce"))
		return QOF_DATE_FORMAT_CE;
	if (0 == safe_strcmp (name, "utc"))
		return QOF_DATE_FORMAT_UTC;
	if (0 == safe_strcmp (name, "iso"))
		return QOF_DATE_FORMAT_ISO;
	if (0 == safe_strcmp (name, "locale"))
		return QOF_DATE_FORMAT_LOCALE;
	if (0 == safe_strcmp (name, "custom"))
		return QOF_DATE_FORMAT_CUSTOM;
	i.name = name;
	i.df = -1;
	g_hash_table_foreach (DateFormatTable, lookup_name, &i);
	return i.df;
}

QofDate *
qof_date_parse (const gchar * str, QofDateFormat df)
{
	const gchar *format;
	QofDateError error;
	QofDate *date;
	gchar *check;

	ENTER (" ");
	check = NULL;
	error = ERR_NO_ERROR;
	date = qof_date_new ();
	format = qof_date_format_get_format (df);
	check = strptime_internal (str, format, date, &error);
	if (error != ERR_NO_ERROR)
	{
		qof_date_free (date);
		fprintf (stderr, "strptime %s\n", 
			QofDateErrorasString (error));
		LEAVE (" date is null");
		return NULL;
	}
	LEAVE (" valid date parsed.");
	return date;
}

gchar *
qof_date_print (QofDate * date, QofDateFormat df)
{
	size_t result;
	gchar temp[MAX_DATE_BUFFER];
	QofDateEntry *d;

	g_return_val_if_fail (QofDateInit, NULL);
	g_return_val_if_fail (date, NULL);
	d = g_hash_table_lookup (DateFormatTable, 
		GINT_TO_POINTER (df));
	g_return_val_if_fail (d, NULL);
	ENTER (" ");
	temp[0] = '\1';
	result = strftime_case (FALSE, temp, MAX_DATE_BUFFER, 
		d->format, date, 1, date->qd_nanosecs);
	if (result == 0 && temp[0] != '\0')
	{
		LEAVE (" qof extended strftime failed");
		return NULL;
	}
	LEAVE (" ");
	return g_strndup(temp, result);
}

/* QofDate handlers */

QofDate *
qof_date_new (void)
{
	QofDate *d;

	d = g_new0 (QofDate, 1);
	return d;
}

void
qof_date_free (QofDate * date)
{
	g_return_if_fail (date);
	g_free (date);
	date = NULL;
}

static QofDate*
date_normalise (QofDate * date)
{
	gint days;
	gboolean neg_year;

	g_return_val_if_fail (date, NULL);
	neg_year = (date->qd_year < 0) ? TRUE : FALSE;
	/* overrides */
	if ((date->qd_nanosecs < 0) || 
		(date->qd_sec < 0)  ||
		(date->qd_min < 0)  ||
		(date->qd_hour < 0) ||
		(date->qd_mday < 0) ||
		(date->qd_mon < 0)  ||
		(date->qd_yday < 0) ||
		(date->qd_year < 0))
		neg_year = TRUE;
	/* if value is negative, just add */
	if ((date->qd_nanosecs >= QOF_NSECS) || 
		(date->qd_nanosecs <= -QOF_NSECS))
	{
		date->qd_sec += date->qd_nanosecs / QOF_NSECS;
		date->qd_nanosecs = date->qd_nanosecs % QOF_NSECS;
		if (date->qd_nanosecs < 0)
		{
			date->qd_nanosecs += QOF_NSECS;
			date->qd_sec--;
		}
	}
	if ((date->qd_sec >= 60) || (date->qd_sec <= -60))
	{
		date->qd_min += date->qd_sec / 60;
		date->qd_sec  = date->qd_sec % 60;
		if (date->qd_sec < 0)
		{
			date->qd_sec += 60;
			date->qd_min--;
		}
	}
	if ((date->qd_min >= 60) || (date->qd_min <= -60))
	{
		date->qd_hour += date->qd_min / 60;
		date->qd_min   = date->qd_min % 60;
		if (date->qd_min < 0)
		{
			date->qd_min += 60;
			date->qd_hour--;
		}
	}
	if ((date->qd_hour >= 24) || (date->qd_hour <= -24))
	{
		date->qd_mday += date->qd_hour / 24;
		date->qd_hour  = date->qd_hour % 24;
		if (date->qd_hour < 0)
		{
			date->qd_hour += 24;
			date->qd_mday--;
		}
	}
	if ((date->qd_mday >= 32) || (date->qd_mday <= -32))
	{
		gboolean is_leap;
		gint leap;

		is_leap = qof_date_isleap (date->qd_year);
		leap = (is_leap) ? 366 : 365;
		date->qd_mday %= leap;
		date->qd_mon  += date->qd_mon  / 12;
	}
	if ((date->qd_mon > 12) || (date->qd_mon < -12))
	{
		date->qd_year += date->qd_mon / 12;
		date->qd_mon   = date->qd_mon % 12;
	}
	/* Year Zero does not exist, 1BC is immediately followed by 1AD. */
	if (date->qd_year == 0)
		date->qd_year = -1;
	days = days_in_months[qof_date_isleap(date->qd_year)][date->qd_mon];
	while (date->qd_mday < 0)
	{
		date->qd_mday += days;
		date->qd_mon--;
		if (date->qd_mon < 1)
		{
			date->qd_year -= date->qd_mon / 12;
			date->qd_mon   = date->qd_mon % 12;
			/* if year was AD and is now zero, reset to BC. */
			if ((date->qd_year == 0) && (date->qd_mon < 0))
				date->qd_year = -1;
		}
		days = days_in_months[qof_date_isleap(date->qd_year)][date->qd_mon];
	}
	while (date->qd_mday > days)
	{
		date->qd_mday -= days;
		date->qd_mon++;
		if (date->qd_mon > 12)
		{
			date->qd_year += date->qd_mon / 12;
			date->qd_mon   = date->qd_mon % 12;
			/* if year was BC and is now zero, reset to AD. */
			if ((date->qd_year == 0) && (date->qd_mon > 0))
				date->qd_year = +1;
		}
		days = days_in_months[qof_date_isleap(date->qd_year)][date->qd_mon];
	}
	/* use sensible defaults */
	if (date->qd_mday == 0)
		date->qd_mday = 1;
	if (date->qd_mon == 0)
		date->qd_mon = 1;
	/* use days_in_year to set yday */
	date->qd_yday = (date->qd_mday - 1) + 
		days_in_year[qof_date_isleap(date->qd_year)][date->qd_mon];
	/* qd_year has no realistic limits */
	date->qd_valid = TRUE;
	return date;
}

gboolean
qof_date_valid (QofDate *date)
{
	g_return_val_if_fail (date, FALSE);
	date = date_normalise (date);
	if (date->qd_valid == FALSE)
	{
		PERR (" unknown QofDate error");
		return FALSE;
	}
	return TRUE;
}

gboolean
qof_date_equal (const QofDate *d1, const QofDate *d2)
{
	if (0 == qof_date_compare (d1, d2))
		return TRUE;
	return FALSE;
}

gint
qof_date_compare (const QofDate * d1, const QofDate * d2)
{
	if ((!d1) && (!d2))
		return 0;
	if (d1 == d2)
		return 0;
	if (!d1)
		return -1;
	if (!d2)
		return 1;
	if (d1->qd_year < d2->qd_year)
		return -1;
	if (d1->qd_year > d2->qd_year)
		return 1;
	if (d1->qd_mon < d2->qd_mon)
		return -1;
	if (d1->qd_mon > d2->qd_mon)
		return 1;
	if (d1->qd_mday < d2->qd_mday)
		return -1;
	if (d1->qd_mday > d2->qd_mday)
		return 1;
	if (d1->qd_hour < d2->qd_hour)
		return -1;
	if (d1->qd_hour > d2->qd_hour)
		return 1;
	if (d1->qd_min < d2->qd_min)
		return -1;
	if (d1->qd_min > d2->qd_min)
		return 1;
	if (d1->qd_sec < d2->qd_sec)
		return -1;
	if (d1->qd_sec > d2->qd_sec)
		return 1;
	if (d1->qd_nanosecs < d2->qd_nanosecs)
		return -1;
	if (d1->qd_nanosecs > d2->qd_nanosecs)
		return 1;
	return 0;
}

QofDate *
qof_date_from_struct_tm (struct tm *tm)
{
	QofDate *d;

	g_return_val_if_fail (tm, NULL);
	d = g_new0 (QofDate, 1);
	d->qd_sec = tm->tm_sec;
	d->qd_min = tm->tm_min;
	d->qd_hour = tm->tm_hour;
	d->qd_mday = tm->tm_mday;
	d->qd_mon = tm->tm_mon + 1;
	d->qd_year = tm->tm_year + 1900;
	d->qd_wday = tm->tm_wday;
	d->qd_yday = tm->tm_yday;
	d->qd_is_dst = tm->tm_isdst;
	d->qd_gmt_off = tm->tm_gmtoff;
	d->qd_zone = tm->tm_zone;
	d->qd_valid = TRUE;
	d = date_normalise(d);
	return d;
}

gboolean
qof_date_to_struct_tm (QofDate * qt, struct tm * tm, glong *nanosecs)
{
	g_return_val_if_fail (qt, FALSE);
	g_return_val_if_fail (tm, FALSE);
	g_return_val_if_fail (qt->qd_valid, FALSE);
	qt = date_normalise (qt);
	if ((qt->qd_year > G_MAXINT) || (qt->qd_year < 1900))
	{
		PERR (" date too large for struct tm");
		return FALSE;
	}
	tm->tm_sec = qt->qd_sec;
	tm->tm_min = qt->qd_min;
	tm->tm_hour = qt->qd_hour;
	tm->tm_mday = qt->qd_mday;
	tm->tm_mon = qt->qd_mon;
	tm->tm_year = qt->qd_year - 1900;
	tm->tm_wday = qt->qd_wday;
	tm->tm_yday = qt->qd_yday;
	tm->tm_isdst = qt->qd_is_dst;
	tm->tm_gmtoff = qt->qd_gmt_off;
	tm->tm_zone = qt->qd_zone;
	*nanosecs = qt->qd_nanosecs;
	return TRUE;
}

gboolean
qof_date_to_gdate (QofDate *qd, GDate *gd)
{
	g_return_val_if_fail (qd, FALSE);
	g_return_val_if_fail (gd, FALSE);
	qd = date_normalise (qd);
	if (qd->qd_year >= G_MAXUINT16)
	{
		PERR (" QofDate out of range of GDate");
		return FALSE;
	}
	if (!g_date_valid_dmy (qd->qd_mday, qd->qd_mon, qd->qd_year))
	{
		PERR (" GDate failed to allow day, month and/or year");
		return FALSE;
	}
	g_date_set_dmy (gd, qd->qd_mday, qd->qd_mon, qd->qd_year);
	return TRUE;
}

QofDate *
qof_date_from_gdate (GDate *date)
{
	QofDate * qd;

	g_return_val_if_fail (g_date_valid (date), NULL);
	qd = qof_date_new ();
	qd->qd_year = g_date_get_year (date);
	qd->qd_mon  = g_date_get_month (date);
	qd->qd_mday = g_date_get_day (date);
	return qd;
}

/** \todo rationalise this and date_normalise */
static void
qof_date_offset (const QofTime *time, glong offset, QofDate *qd)
{
	glong days;
	gint64 rem, y, yg;
	const guint16 *ip;
	QofTimeSecs t;

	g_return_if_fail (qd);
	g_return_if_fail (time);
	t = qof_time_get_secs ((QofTime*)time);
	days = t / SECS_PER_DAY;
	rem = t % SECS_PER_DAY;
	rem += offset;
	while (rem < 0)
	{
		rem += SECS_PER_DAY;
		--days;
	}
	while (rem >= SECS_PER_DAY)
	{
		rem -= SECS_PER_DAY;
		++days;
	}
	qd->qd_hour = rem / SECS_PER_HOUR;
	rem %= SECS_PER_HOUR;
	qd->qd_min = rem / 60;
	qd->qd_sec = rem % 60;
	/* January 1, 1970 was a Thursday.  */
	qd->qd_wday = (4 + days) % 7;
	if (qd->qd_wday < 0)
		qd->qd_wday += 7;
	y = 1970;
	while (days < 0 || days >= (__isleap (y) ? 366 : 365))
	{
		/* Guess a corrected year, assuming 365 days per year.  */
		yg = y + days / 365 - (days % 365 < 0);
		/* Adjust DAYS and Y to match the guessed year.  */
		days -= ((yg - y) * 365
			+ LEAPS_THRU_END_OF (yg - 1)
			- LEAPS_THRU_END_OF (y - 1));
		y = yg;
	}
	qd->qd_year = y;
	qd->qd_yday = days;
	ip = days_in_year[qof_date_isleap(y)];
	for (y = 12; days < (glong) ip[y]; --y)
		continue;
	days -= ip[y];
	qd->qd_mon = y;
	qd->qd_mday = days + 1;
}

/* safe to use time_t here because only values
within the range of a time_t have any leapseconds. */
static gint
count_leapseconds (time_t interval)
{
	time_t altered;
	struct tm utc;

	altered = interval;
	utc = *gmtime_r (&interval, &utc);
	altered = mktime (&utc);
	/** \todo check not a negative value */
	return altered - interval;
}

static gint
extract_interval (const QofTime *qt)
{
	gint leap_seconds;
	QofTimeSecs t, l;
	const QofTime *now;

	leap_seconds = 0;
	t = qof_time_get_secs (qt);
	now = qof_time_get_current ();
	l = (qof_time_get_secs (now) > G_MAXINT32) ? 
		G_MAXINT32 : qof_time_get_secs (now);
	leap_seconds = ((t > l) || (t < 0)) ? 
		count_leapseconds (l) :
		count_leapseconds (t);
	return leap_seconds;
}

QofDate *
qof_date_from_qtime (const QofTime *qt)
{
	QofDate *qd;
	gint leap_extra_secs;

	g_return_val_if_fail (qt, NULL);
	/** \todo add check for qt validity */
	qd = qof_date_new ();
	tzset();
	leap_extra_secs = extract_interval (qt);
	qof_date_offset (qt, leap_extra_secs, qd);
	qd->qd_is_dst = 0;
	qd->qd_zone = "GMT";
	qd->qd_gmt_off = 0L;
	if (!qof_date_valid(qd))
		return NULL;
	return qd;
}

static gint64
days_between (gint64 year1, gint64 year2)
{
	gint64 i, start, end, l;

	l = 0;
	if (year1 == year2)
		return l;
	start = (year1 < year2) ? year1 : year2;
	end = (year2 < year1) ? year1: year2;
	for (i = start; i < end; i++)
	{
		l += (qof_date_isleap(i)) ? 366 : 365;
	}
	return l;
}

QofTime*
qof_date_to_qtime (QofDate *qd)
{
	QofTime *time;
	QofTimeSecs c;

	g_return_val_if_fail (qd, NULL);
	g_return_val_if_fail (qof_date_valid(qd), NULL);
	time = qof_time_new ();
	c = 0;
	if (qd->qd_year < 1970)
	{
		c = qd->qd_sec;
		c += QOF_MIN_TO_SEC(qd->qd_min);
		c += QOF_HOUR_TO_SEC(qd->qd_hour);
		c += QOF_DAYS_TO_SEC(qd->qd_yday);
		c -= QOF_DAYS_TO_SEC(days_between (1970, qd->qd_year));
		c += qd->qd_gmt_off;
		time = qof_time_set (c, qd->qd_nanosecs);
	}
	if (qd->qd_year >= 1970)
	{
		c = qd->qd_sec;
		c += QOF_MIN_TO_SEC(qd->qd_min);
		c += QOF_HOUR_TO_SEC(qd->qd_hour);
		c += QOF_DAYS_TO_SEC(qd->qd_yday);
		c += QOF_DAYS_TO_SEC(days_between (1970, qd->qd_year));
		c -= qd->qd_gmt_off;
		time = qof_time_set (c, qd->qd_nanosecs);
	}
	return time;
}

static inline gint64
calc_julian_days (QofDate *date)
{
	gint64 days, leap;

	/*  Used when a GDate is out of range.
	From glib: 
	multiply years * 365 days in the year,
	add the number of years divided by 4, subtract the number of
	years divided by 100 and add the number of years divided by 400,
	which accounts for leap year stuff. Code from Steffen Beyer's
	DateCalc. 
	*/
	days = date->qd_year * 365;
	leap = g_date_is_leap_year (date->qd_year);
	if (date->qd_year > 0)
	{
	/* if year > G_MAXUINT16 */
		/* divide by 4 and add 
		(every fourth year is leap so add a day for each.) */
		days += (date->qd_year >>= 2);
		/* divides original # years by 100 
		(turn of century years are not leap) */
		days -= (date->qd_year /= 25);
		/* divides by 4, which divides original by 400.
		(turn of millenium years are leap) */
		days += date->qd_year >> 2;
		days += days_in_year[leap][date->qd_mon];
		days += date->qd_mday;
	}
	if (date->qd_year < 0)
	{
		/* divide by 4 and subtract */
		days -= (date->qd_year >>= 2);
		/* divides original # years by 100 */		
		days += (date->qd_year /= 25);
		/* divides by 4, which divides original by 400 */
		days -= date->qd_year >> 2;
		days -= days_in_year[leap][date->qd_mon];
		days -= date->qd_mday;
	}
	return days;
}

QofTime *
qof_date_time_difference (QofDate * date1, QofDate * date2)
{
	GDate *d1, *d2;
	gint64 days_between, julian_days;
	QofTime *secs;
	gboolean use_local;

	d1 = g_date_new ();
	d2 = g_date_new ();
	use_local = FALSE;
	julian_days = 0;
	secs = qof_time_new ();
	if(!qof_date_to_gdate (date1, d1))
	{
		use_local = TRUE;
		julian_days = calc_julian_days (date1);
	}
	if(!qof_date_to_gdate (date2, d2) || use_local == TRUE)
	{
		use_local = TRUE;
		julian_days -= calc_julian_days (date2);
	}
	if(!use_local)
	{
		days_between = g_date_days_between (d1, d2);
	}
	else
	{
		days_between = julian_days;
	}
	secs = qof_time_add_secs(secs, QOF_DAYS_TO_SEC(days_between));
	if (days_between >= 0)
	{
		/* positive value, add date2 secs, subtract date1 */
		secs =  qof_time_add_secs(secs, -1 *
				(QOF_HOUR_TO_SEC(date1->qd_hour) -
				QOF_MIN_TO_SEC(date1->qd_min) -
				(date1->qd_sec)));
		secs =  qof_time_add_secs(secs,
				QOF_HOUR_TO_SEC(date2->qd_hour) +
				QOF_MIN_TO_SEC(date2->qd_min) +
				(date2->qd_sec));
		qof_time_set_nanosecs(secs, 
			(date1->qd_nanosecs - date2->qd_nanosecs));
	}
	if (days_between < 0)
	{
		/* negative value*/
		secs =  qof_time_add_secs (secs, 
				QOF_HOUR_TO_SEC(date1->qd_hour) -
				QOF_MIN_TO_SEC(date1->qd_min) -
				(date1->qd_sec));
		secs =  qof_time_add_secs (secs, -1 * 
				(QOF_HOUR_TO_SEC(date2->qd_hour) +
				QOF_MIN_TO_SEC(date2->qd_min) +
				(date2->qd_sec)));
		qof_time_set_nanosecs(secs, 
			(date2->qd_nanosecs - date1->qd_nanosecs));
	}
	return secs;
}

gboolean
qof_date_time_add_days (QofTime * qt, gint days)
{
	g_return_val_if_fail (qt, FALSE);
	qt = qof_time_add_secs (qt, days * SECS_PER_DAY);
	return TRUE;
}

gboolean
qof_date_time_add_months (QofTime * qt, guint8 months,
	gboolean track_last_day)
{
	GDateDay new_last_mday;
	GDate *d, *earlier;
	gint gap;

	g_return_val_if_fail (qt, FALSE);
	d = qof_time_to_gdate (qt);
	earlier = qof_time_to_gdate (qt);
	g_date_add_months (d, months);
	if (track_last_day && g_date_is_last_of_month (d))
	{
		new_last_mday = g_date_get_days_in_month (g_date_get_month (d),
			g_date_get_year (d));
		g_date_set_day (d, new_last_mday);
	}
	gap = g_date_days_between (earlier, d);
	qof_date_time_add_days (qt, gap);
	return TRUE;
}

/********************** END OF FILE *********************************/
