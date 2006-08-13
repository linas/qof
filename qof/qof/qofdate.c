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
static QofDateFormat dateFormat = QOF_DATE_FORMAT_LOCALE;

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
	{
		QofDateEntry *d = g_new0(QofDateEntry,1);
		d->format = "%Y-%m-%d %H:%M:%S.%N %z";
		d->name = "iso8601";
		d->separator = '-';
		d->df = QOF_DATE_FORMAT_ISO8601;
		d->locale_specific = FALSE;
		g_hash_table_insert (DateFormatTable, GINT_TO_POINTER(d->df), d);
	}
	QofDateInit = TRUE;
}

static void
hash_value_free (gpointer key __attribute__ ((unused)), gpointer value, 
	gpointer data __attribute__ ((unused)))
{
	g_free (value);
}

void
qof_date_close (void)
{
	if (QofDateInit)
	{
		g_hash_table_foreach (DateFormatTable, hash_value_free, NULL);
		g_hash_table_destroy (DateFormatTable);
	}
	QofDateInit = FALSE;
}

guint16
qof_date_get_yday (gint mday, gint month, gint64 year)
{
	guint8 leap;

	g_return_val_if_fail (mday  != 0, 0);
	g_return_val_if_fail (month != 0, 0);
	g_return_val_if_fail (month <= 12, 0);
	g_return_val_if_fail (month >= 1, 0);
	g_return_val_if_fail (year  != 0, 0);
	leap = qof_date_isleap (year);
	g_return_val_if_fail (mday <= 
		qof_date_get_mday (month, year), 0);
	return days_in_year[leap][month] + mday;
}

guint8
qof_date_get_mday (gint month, gint64 year)
{
	g_return_val_if_fail (month !=  0, 0);
	g_return_val_if_fail (month <= 12, 0);
	g_return_val_if_fail (month >=  1, 0);
	g_return_val_if_fail (year  !=  0, 0);
	return days_in_months[qof_date_isleap (year)][month];
}

gboolean
qof_date_is_last_mday (const QofDate *qd)
{
	g_return_val_if_fail (qd, FALSE);
	g_return_val_if_fail (qd->qd_valid, FALSE);
	return (qd->qd_mday == 
		qof_date_get_mday (qd->qd_mon, qd->qd_year));
}

gboolean
qof_date_format_add (const gchar * str, QofDateFormat * identifier)
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
	*identifier = g_hash_table_size (DateFormatTable) + 1;
	{
		QofDateEntry *d = g_new0 (QofDateEntry, 1);
		d->format = str;
		d->name = str;
		d->separator = locale_separator;
		d->df = *identifier;
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
lookup_name (gpointer key __attribute__ ((unused)), gpointer value, 
	gpointer data)
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

static QofDate*
date_normalise (QofDate * date)
{
	gint days;

	g_return_val_if_fail (date, NULL);
	date->qd_sec -= date->qd_gmt_off;
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
	if ((date->qd_mon > 12) || (date->qd_mon < -12))
	{
		date->qd_year += date->qd_mon / 12;
		date->qd_mon   = date->qd_mon % 12;
		if (date->qd_mon < 0)
		{
			/* -1 == Dec, -4 == Sep */
			date->qd_mon += 12 + 1;
			date->qd_year = (date->qd_year < 0) ? 
				date->qd_year++ : date->qd_year--;
		}
	}
	/* qd_mon starts at 1, not zero */
	if (date->qd_mon == 0)
		date->qd_mon = 1;
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
	set_day_of_the_week (date);
	/* qd_year has no realistic limits */
	date->qd_valid = TRUE;
	date->qd_zone = "GMT";
	date->qd_is_dst = 0;
	date->qd_gmt_off = 0L;
	return date;
}

QofDate *
qof_date_parse (const gchar * str, QofDateFormat df)
{
	const gchar *format;
	QofDateError error;
	QofDate *date;
	gchar *check;

	check = NULL;
	error = ERR_NO_ERROR;
	date = qof_date_new ();
	format = qof_date_format_get_format (df);
	check = strptime_internal (str, format, date, &error);
	if (error != ERR_NO_ERROR)
	{
		qof_date_free (date);
		return NULL;
	}
	date = date_normalise (date);
	return date;
}

gchar *
qof_date_print (const QofDate * date, QofDateFormat df)
{
	size_t result;
	gchar temp[MAX_DATE_BUFFER];
	QofDateEntry *d;

	g_return_val_if_fail (QofDateInit, NULL);
	g_return_val_if_fail (date, NULL);
	g_return_val_if_fail (date->qd_valid, NULL);
	d = g_hash_table_lookup (DateFormatTable, 
		GINT_TO_POINTER (df));
	g_return_val_if_fail (d, NULL);
	temp[0] = '\1';
	result = strftime_case (FALSE, temp, MAX_DATE_BUFFER, 
		d->format, date, 1, date->qd_nanosecs);
	if (result == 0 && temp[0] != '\0')
	{
		PERR (" qof extended strftime failed");
		return NULL;
	}
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

QofDate *
qof_date_get_current (void)
{
	QofTime *qt;
	QofDate *qd;

	qt = qof_time_get_current ();
	qd = qof_date_from_qtime (qt);
	qof_time_free (qt);
	return qd;
}

QofDate *
qof_date_new_dmy (gint day, gint month, gint64 year)
{
	QofDate *qd;

	qd = g_new0 (QofDate, 1);
	qd->qd_mday = day;
	qd->qd_mon  = month;
	qd->qd_year = year;
	if(!qof_date_valid (qd))
		return NULL;
	return qd;
}

void
qof_date_free (QofDate * date)
{
	g_return_if_fail (date);
	g_free (date);
	date = NULL;
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
qof_date_from_struct_tm (const struct tm *stm)
{
	QofDate *d;

	g_return_val_if_fail (stm, NULL);
	d = g_new0 (QofDate, 1);
	d->qd_sec  = stm->tm_sec;
	d->qd_min  = stm->tm_min;
	d->qd_hour = stm->tm_hour;
	d->qd_mday = stm->tm_mday;
	d->qd_mon  = stm->tm_mon + 1;
	d->qd_year = stm->tm_year + 1900;
	d->qd_wday = stm->tm_wday;
	d->qd_yday = stm->tm_yday;
	d->qd_is_dst = stm->tm_isdst;
	d->qd_gmt_off = stm->tm_gmtoff;
	d->qd_zone = stm->tm_zone;
	d->qd_valid = TRUE;
	d = date_normalise(d);
	return d;
}

gboolean
qof_date_to_struct_tm (const QofDate * qd, struct tm * stm, 
					   glong *nanosecs)
{
	g_return_val_if_fail (qd, FALSE);
	g_return_val_if_fail (stm, FALSE);
	g_return_val_if_fail (qd->qd_valid, FALSE);
	if ((qd->qd_year > G_MAXINT) || (qd->qd_year < 1900))
	{
		PERR (" date too large for struct tm");
		return FALSE;
	}
	stm->tm_sec  = qd->qd_sec;
	stm->tm_min  = qd->qd_min;
	stm->tm_hour = qd->qd_hour;
	stm->tm_mday = qd->qd_mday;
	stm->tm_mon  = qd->qd_mon - 1;
	stm->tm_year = qd->qd_year - 1900;
	stm->tm_wday = qd->qd_wday;
	stm->tm_yday = qd->qd_yday;
	stm->tm_isdst = qd->qd_is_dst;
	stm->tm_gmtoff = qd->qd_gmt_off;
	stm->tm_zone = qd->qd_zone;
	if (nanosecs != NULL)
		*nanosecs = qd->qd_nanosecs;
	return TRUE;
}

gboolean
qof_date_to_gdate (const QofDate *qd, GDate *gd)
{
	g_return_val_if_fail (qd, FALSE);
	g_return_val_if_fail (gd, FALSE);
	g_return_val_if_fail (qd->qd_valid, FALSE);
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
qof_date_from_gdate (const GDate *date)
{
	QofDate * qd;

	g_return_val_if_fail (g_date_valid (date), NULL);
	qd = qof_date_new ();
	qd->qd_year = g_date_get_year (date);
	qd->qd_mon  = g_date_get_month (date);
	qd->qd_mday = g_date_get_day (date);
	qd = date_normalise (qd);
	return qd;
}

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
	return altered - interval;
}

/*static inline gint*/
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

	/* may not want to create a new time or date - it
	complicates memory management. */
	g_return_val_if_fail (qt, NULL);
	g_return_val_if_fail (qof_time_is_valid (qt), NULL);
	qd = qof_date_new ();
	leap_extra_secs = 0;
	tzset();
	leap_extra_secs = extract_interval (qt);
	qof_date_offset (qt, leap_extra_secs, qd);
	qd->qd_nanosecs = qof_time_get_nanosecs (qt);
	qd->qd_is_dst = 0;
	qd->qd_zone = "GMT";
	qd->qd_gmt_off = 0L;
	if (!qof_date_valid(qd))
		return NULL;
	return qd;
}

gint64
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
qof_date_to_qtime (const QofDate *qd)
{
	QofTime *qt;
	QofTimeSecs c;

	g_return_val_if_fail (qd, NULL);
	g_return_val_if_fail (qd->qd_valid, NULL);
	c = 0;
	qt = NULL;
	if (qd->qd_year < 1970)
	{
		c = qd->qd_sec;
		c += QOF_MIN_TO_SEC(qd->qd_min);
		c += QOF_HOUR_TO_SEC(qd->qd_hour);
		c += QOF_DAYS_TO_SEC(qd->qd_yday);
		c -= QOF_DAYS_TO_SEC(days_between (1970, qd->qd_year));
		c -= qd->qd_gmt_off;
		qt = qof_time_set (c, qd->qd_nanosecs);
	}
	if (qd->qd_year >= 1970)
	{
		c = qd->qd_sec;
		c += QOF_MIN_TO_SEC(qd->qd_min);
		c += QOF_HOUR_TO_SEC(qd->qd_hour);
		c += QOF_DAYS_TO_SEC(qd->qd_yday);
		c += QOF_DAYS_TO_SEC(days_between (1970, qd->qd_year));
		c -= qd->qd_gmt_off;
		qt = qof_time_set (c, qd->qd_nanosecs);
	}
	return qt;
}

QofTime *
qof_date_time_difference (const QofDate * date1, 
	const QofDate * date2)
{
	gint64 days;
	QofTime *secs;

	secs = qof_time_new ();
	days = days_between (date1->qd_year, date2->qd_year);
	qof_time_add_secs(secs, QOF_DAYS_TO_SEC(days));
	if (days >= 0)
	{
		/* positive value, add date2 secs, subtract date1 */
		qof_time_add_secs(secs, -1 *
				(QOF_HOUR_TO_SEC(date1->qd_hour) -
				QOF_MIN_TO_SEC(date1->qd_min) -
				(date1->qd_sec)));
		qof_time_add_secs(secs,
				QOF_HOUR_TO_SEC(date2->qd_hour) +
				QOF_MIN_TO_SEC(date2->qd_min) +
				(date2->qd_sec));
		qof_time_set_nanosecs(secs, 
			(date1->qd_nanosecs - date2->qd_nanosecs));
	}
	if (days < 0)
	{
		/* negative value*/
		qof_time_add_secs (secs, 
				QOF_HOUR_TO_SEC(date1->qd_hour) -
				QOF_MIN_TO_SEC(date1->qd_min) -
				(date1->qd_sec));
		qof_time_add_secs (secs, -1 * 
				(QOF_HOUR_TO_SEC(date2->qd_hour) +
				QOF_MIN_TO_SEC(date2->qd_min) +
				(date2->qd_sec)));
		qof_time_set_nanosecs(secs, 
			(date2->qd_nanosecs - date1->qd_nanosecs));
	}
	return secs;
}

gboolean
qof_date_adddays (QofDate * qd, gint days)
{
	g_return_val_if_fail (qd, FALSE);
	g_return_val_if_fail (qof_date_valid (qd), FALSE);
	qd->qd_mday += days;
	return qof_date_valid (qd);
}

gboolean
qof_date_addmonths (QofDate * qd, gint months,
	gboolean track_last_day)
{
	g_return_val_if_fail (qd, FALSE);
	g_return_val_if_fail (qof_date_valid (qd), FALSE);
	qd->qd_mon += months % 12;
	qd->qd_year += months / 12;
	g_return_val_if_fail (qof_date_valid (qd), FALSE);
	if (track_last_day && qof_date_is_last_mday (qd))
	{
		qd->qd_mday = qof_date_get_mday (qd->qd_mon,
			qd->qd_year);
	}
	return TRUE;
}

inline gboolean
qof_date_set_day_end (QofDate * qd)
{
	qd->qd_hour = 23;
	qd->qd_min  = 59;
	qd->qd_sec  = 59;
	qd->qd_nanosecs = (QOF_NSECS - 1);
	return qof_date_valid (qd);
}

inline gboolean
qof_date_set_day_start (QofDate * qd)
{
	g_return_val_if_fail (qd, FALSE);
	qd->qd_hour = 0;
	qd->qd_min  = 0;
	qd->qd_sec  = 0;
	qd->qd_nanosecs = G_GINT64_CONSTANT(0);
	return qof_date_valid (qd);
}

inline gboolean
qof_date_set_day_middle (QofDate * qd)
{
	g_return_val_if_fail (qd, FALSE);
	qd->qd_hour = 12;
	qd->qd_min  = 0;
	qd->qd_sec = 0;
	qd->qd_nanosecs = G_GINT64_CONSTANT(0);
	return qof_date_valid (qd);
}

/******************** END OF FILE *************************/
