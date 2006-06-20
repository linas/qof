/***************************************************************************
 *            test-date.c
 *       Rewritten from scratch for QOF 0.7.0
 *
 *  Copyright (C) 2002, 2004, 2005, 2006 
 *  Free Software Foundation, Inc.
 *
 ****************************************************************************/
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
#include <stdio.h>
#include <ctype.h>
#include <glib.h>
#include <time.h>
#include "qof.h"
#include "test-stuff.h"

static gboolean test_data_is_init = FALSE;
static GList *test_data = NULL;
/** \todo reset debug to false for release */
static gboolean debug = TRUE;

typedef struct
{
	/* time value of the date below */
	QofTime *time;
	/* date value of the above time. */
	QofDate *date;
	/* GList of the date in each QofDateFormat */
	GList   *string_list;
	/* unique ID for this date for error reports. */
	const gchar *id;
}QTestDate;

static void
check_date_cycles (gpointer data, gpointer user_data)
{
	QofDateFormat df;
	gchar *str, *cmp;
	QTestDate *d;
	gint i;

	d = (QTestDate*)data;
	df = qof_date_format_get_current ();
	/* don't test locale-sensitive formats, yet. */
	for (i = 1; i <= 5; i++)
	{
		str = qof_date_print (d->date, i);
		cmp = (gchar*)g_list_nth_data (d->string_list, (i - 1));
		do_test ((0 == safe_strcasecmp (str, cmp)), d->id);
		if (debug && (0 != safe_strcasecmp (str, cmp)))
		{
			fprintf (stderr, "DEBUG: str=%s\ncmp=%s\n", str, cmp);
			fprintf (stderr, "DEBUG: expect:%" G_GINT64_FORMAT "/%d/%d\n",
				d->date->qd_year, d->date->qd_mon, d->date->qd_mday);
		}
		do_test ((0 == qof_time_cmp (qof_date_to_qtime (d->date), d->time)), d->id);
		{
			QofTime *f = qof_date_to_qtime (d->date);
			do_test ((0 == qof_time_cmp (
				qof_date_to_qtime (d->date), d->time)),
				"convert date to time");
			if ((debug) && (0 != qof_time_cmp (
				qof_date_to_qtime (d->date), d->time))) 
			{
				fprintf (stderr, "DEBUG: diff=%" G_GINT64_FORMAT 
					" real=%" G_GINT64_FORMAT " source=%" G_GINT64_FORMAT "\n", 
					qof_time_get_secs(qof_time_diff (d->time, f)),
					qof_time_get_secs(f), qof_time_get_secs (d->time));
				fprintf (stderr, "DEBUG: year=%" G_GINT64_FORMAT " month=%d"
					" day=%d hour=%d min=%d sec=%" G_GINT64_FORMAT "\n",
					d->date->qd_year, d->date->qd_mon, d->date->qd_mday,
					d->date->qd_hour, d->date->qd_min, d->date->qd_sec);
			}
			qof_time_free (f);
		}
		{
			QofDate *e = qof_date_from_qtime (d->time);
			if ((debug) && (0 != qof_date_compare (e, d->date)))
				fprintf (stderr, "compare=%d\n", qof_date_compare(e, d->date));
			do_test ((0 == qof_date_compare (e, d->date)), d->id);
			qof_date_free (e);
		}
		/* now test qofstrptime */
		{
			gint result;
			QofDate *h, *j;
			const gchar * t;
			struct tm base;

			base.tm_year = base.tm_mday = base.tm_mon = 0;
			base.tm_min  = base.tm_hour = base.tm_gmtoff = 0;
			base.tm_sec  = base.tm_isdst = 0;
			result = 0;
			h = qof_date_parse (cmp, i);
			do_test ((h != NULL), "no date could be parsed");
			if (!h)
				fprintf (stderr, "h failed for str=%s, %d\n",
					cmp, i);
			t = qof_date_format_get_format (i);
			strptime (cmp, t, &base);
			j = qof_date_from_struct_tm (&base);
			do_test ((j != NULL), "no value from struct tm");
			if (h)
			{
				j->qd_nanosecs = h->qd_nanosecs;
				result = qof_date_compare (h, j);
				do_test ((0 == qof_date_compare (h, j)),
					"compare with strptime");
			}

			if (h && (result != 0))
			{
				fprintf (stderr, "format=%s\n", t);

				fprintf (stderr, "h->qd_year=%" G_GINT64_FORMAT " ", h->qd_year);
				fprintf (stderr, "h->qd_mon=%d ",  h->qd_mon);
				fprintf (stderr, "h->qd_mday=%d ", h->qd_mday);
				fprintf (stderr, "h->qd_hour=%d ", h->qd_hour);
				fprintf (stderr, "h->qd_min=%d ", h->qd_min);
				fprintf (stderr, "h->qd_sec=%" G_GINT64_FORMAT " ", h->qd_sec);
				fprintf (stderr, "h->qd_nanosecs=%ld\n", h->qd_nanosecs);

				fprintf (stderr, "j->qd_year=%" G_GINT64_FORMAT " ", j->qd_year);
				fprintf (stderr, "j->qd_mon=%d ",  j->qd_mon);
				fprintf (stderr, "j->qd_mday=%d ", j->qd_mday);
				fprintf (stderr, "j->qd_hour=%d ", j->qd_hour);
				fprintf (stderr, "j->qd_min=%d ", j->qd_min);
				fprintf (stderr, "j->qd_sec=%" G_GINT64_FORMAT " ", j->qd_sec);
				fprintf (stderr, "j->qd_nanosecs=%ld\n", j->qd_nanosecs);

				fprintf (stderr, "d->qd_year=%" G_GINT64_FORMAT " ", d->date->qd_year);
				fprintf (stderr, "d->qd_mon=%d ",  d->date->qd_mon);
				fprintf (stderr, "d->qd_mday=%d ", d->date->qd_mday);
				fprintf (stderr, "d->qd_hour=%d ", d->date->qd_hour);
				fprintf (stderr, "d->qd_min=%d ", d->date->qd_min);
				fprintf (stderr, "d->qd_sec=%" G_GINT64_FORMAT "\n", d->date->qd_sec);

				fprintf (stderr, "tm.tm_year=%d ", base.tm_year + 1900);
				fprintf (stderr, "tm.tm_mon=%d ", base.tm_mon + 1);
				fprintf (stderr, "tm.tm_mday=%d ", base.tm_mday);
				fprintf (stderr, "tm.tm_hour=%d ", base.tm_hour);
				fprintf (stderr, "tm.tm_min=%d ", base.tm_min);
				fprintf (stderr, "tm.tm_sec=%d\n", base.tm_sec);

				fprintf (stderr, "strptime compare=%d\n", qof_date_compare(h, d->date));
				fprintf (stderr, "strptime compare (2)=%d\n", qof_date_compare(h, j));
			}

			if (h)
				qof_date_free (h);
		}
	}
	qof_date_format_set_current (df);
}

static void
test_date_init (void)
{
	test_data_is_init = TRUE;
	test_data = NULL;
	/* A selection of current, old and future dates - all in UTC only. */
	{
		QTestDate *td = g_new0 (QTestDate, 1);
		td->time = qof_time_new ();
		qof_time_set_secs (td->time, 1147621550);
		qof_time_set_nanosecs (td->time, 0);
		td->date = qof_date_new ();
		td->date->qd_year = 2006;
		td->date->qd_mon  = 5;
		td->date->qd_mday = 14;
		td->date->qd_hour = 15;
		td->date->qd_min  = 45;
		td->date->qd_sec  = G_GINT64_CONSTANT(50);
		td->string_list = NULL;
		td->string_list = g_list_append (td->string_list, "05/14/2006");
		td->string_list = g_list_append (td->string_list, "14/05/2006");
		td->string_list = g_list_append (td->string_list, "14.05.2006");
		td->string_list = g_list_append (td->string_list, "2006-05-14");
		td->string_list = g_list_append (td->string_list, "2006-05-14T15:45:50Z");
		td->id = "a current time";
		test_data = g_list_prepend (test_data, td);
	}
	{
		QTestDate *td = g_new0 (QTestDate, 1);
		td->time = qof_time_new ();
		qof_time_set_secs (td->time, 1147132800);
		qof_time_set_nanosecs (td->time, 0);
		td->date = qof_date_new ();
		td->date->qd_year = 2006;
		td->date->qd_mon  = 5;
		td->date->qd_mday = 9;
		td->date->qd_hour = 0;
		td->date->qd_min  = 0;
		td->date->qd_sec  = G_GINT64_CONSTANT(0);
		td->string_list = NULL;
		td->string_list = g_list_prepend (td->string_list, "05/09/2006");
		td->string_list = g_list_prepend (td->string_list, "09/05/2006");
		td->string_list = g_list_prepend (td->string_list, "09.05.2006");
		td->string_list = g_list_prepend (td->string_list, "2006-05-09");
		td->string_list = g_list_prepend (td->string_list, "2006-05-09T00:00:00Z");
		td->string_list = g_list_reverse (td->string_list);
		td->id = "a recent time";
		test_data = g_list_prepend (test_data, td);
	}
	{
		QTestDate *td = g_new0 (QTestDate, 1);
		td->time = qof_time_new ();
		qof_time_set_secs (td->time, 1147186144);
		qof_time_set_nanosecs (td->time, 0);
		td->date = qof_date_new ();
		td->date->qd_year = 2006;
		td->date->qd_mon  = 5;
		td->date->qd_mday = 9;
		td->date->qd_hour = 14;
		td->date->qd_min  = 49;
		td->date->qd_sec  = G_GINT64_CONSTANT(4);
		td->string_list = NULL;
		td->string_list = g_list_prepend (td->string_list, "05/09/2006");
		td->string_list = g_list_prepend (td->string_list, "09/05/2006");
		td->string_list = g_list_prepend (td->string_list, "09.05.2006");
		td->string_list = g_list_prepend (td->string_list, "2006-05-09");
		td->string_list = g_list_prepend (td->string_list, "2006-05-09T14:49:04Z");
		td->string_list = g_list_reverse (td->string_list);
		td->id = "second recent time";
		test_data = g_list_prepend (test_data, td);
	}
	{
		QTestDate *td = g_new0 (QTestDate, 1);
		td->time = qof_time_new ();
		qof_time_set_secs (td->time, 63039600);
		qof_time_set_nanosecs (td->time, 0);
		td->date = qof_date_new ();
		td->date->qd_year = 1971;
		td->date->qd_mon  = 12;
		td->date->qd_mday = 31;
		td->date->qd_hour = 15;
		td->date->qd_min  = 0;
		td->date->qd_sec  = G_GINT64_CONSTANT(0);
		td->string_list = NULL;
		td->string_list = g_list_prepend (td->string_list, "12/31/1971");
		td->string_list = g_list_prepend (td->string_list, "31/12/1971");
		td->string_list = g_list_prepend (td->string_list, "31.12.1971");
		td->string_list = g_list_prepend (td->string_list, "1971-12-31");
		td->string_list = g_list_prepend (td->string_list, "1971-12-31T15:00:00Z");
		td->string_list = g_list_reverse (td->string_list);
		td->id = "New Year's Eve 1971";
		test_data = g_list_prepend (test_data, td);
	}
	{
		QTestDate *td = g_new0 (QTestDate, 1);
		td->time = qof_time_new ();
		qof_time_set_secs (td->time, 315532800);
		qof_time_set_nanosecs (td->time, 0);
		td->date = qof_date_new ();
		td->date->qd_year = 1980;
		td->date->qd_mon  = 1;
		td->date->qd_mday = 1;
		td->date->qd_hour = 0;
		td->date->qd_min  = 0;
		td->date->qd_sec  = G_GINT64_CONSTANT(0);
		td->string_list = NULL;
		td->string_list = g_list_prepend (td->string_list, "01/01/1980");
		td->string_list = g_list_prepend (td->string_list, "01/01/1980");
		td->string_list = g_list_prepend (td->string_list, "01.01.1980");
		td->string_list = g_list_prepend (td->string_list, "1980-01-01");
		td->string_list = g_list_prepend (td->string_list, "1980-01-01T00:00:00Z");
		td->string_list = g_list_reverse (td->string_list);
		td->id = "New Year's Day 1980";
		test_data = g_list_prepend (test_data, td);
	}
	{
/*		QTestDate *td = g_new0 (QTestDate, 1);
		td->time = qof_time_new ();*/
/* normalising error - causes seg fault!??! */
/*		qof_time_set_secs (td->time, 946684799);
// rest is OK.
		qof_time_set_nanosecs (td->time, 0);
		td->date = qof_date_new ();
		td->date->qd_year = 1999;
		td->date->qd_mon  = 12;
		td->date->qd_mday = 31;
		td->date->qd_hour = 23;
		td->date->qd_min  = 59;
		td->date->qd_sec  = G_GINT64_CONSTANT(59);
		td->string_list = NULL;
		td->string_list = g_list_prepend (td->string_list, "12/31/1999");
		td->string_list = g_list_prepend (td->string_list, "31/12/1999");
		td->string_list = g_list_prepend (td->string_list, "31.12.1999");
		td->string_list = g_list_prepend (td->string_list, "1980-01-01");
		td->string_list = g_list_prepend (td->string_list, "1980-01-01T00:00:00Z");
		td->string_list = g_list_reverse (td->string_list);
		td->id = "New Year's Day 1980 - repeat";
		test_data = g_list_prepend (test_data, td);*/
	}
	{
		QTestDate *td = g_new0 (QTestDate, 1);
		td->time = qof_time_new ();
		qof_time_set_secs (td->time, -192776400);
		qof_time_set_nanosecs (td->time, 0);
		td->date = qof_date_new ();
		td->date->qd_year = 1963;
		td->date->qd_mon  = 11;
		td->date->qd_mday = 22;
		td->date->qd_hour = 19;
		td->date->qd_min  = 0;
		td->date->qd_sec  = G_GINT64_CONSTANT(0);
		td->string_list = NULL;
		td->string_list = g_list_prepend (td->string_list, "11/22/1963");
		td->string_list = g_list_prepend (td->string_list, "22/11/1963");
		td->string_list = g_list_prepend (td->string_list, "22.11.1963");
		td->string_list = g_list_prepend (td->string_list, "1963-11-22");
		td->string_list = g_list_prepend (td->string_list, "1963-11-22T19:00:00Z");
		td->string_list = g_list_reverse (td->string_list);
		td->id = "approx JFK, 1963";
		test_data = g_list_prepend (test_data, td);
	}
	{
		QTestDate *td = g_new0 (QTestDate, 1);
		td->time = qof_time_new ();
		qof_time_set_secs (td->time, -767311080);
		qof_time_set_nanosecs (td->time, 0);
		td->date = qof_date_new ();
		td->date->qd_year = 1945;
		td->date->qd_mon  = 9;
		td->date->qd_mday = 8;
		td->date->qd_hour = 2;
		td->date->qd_min  = 2;
		td->date->qd_sec  = G_GINT64_CONSTANT(0);
		td->string_list = NULL;
		td->string_list = g_list_prepend (td->string_list, "09/08/1945");
		td->string_list = g_list_prepend (td->string_list, "08/09/1945");
		td->string_list = g_list_prepend (td->string_list, "08.09.1945");
		td->string_list = g_list_prepend (td->string_list, "1945-09-08");
		td->string_list = g_list_prepend (td->string_list, "1945-09-08T02:02:00Z");
		td->string_list = g_list_reverse (td->string_list);
		td->id = "Nagasaki, 1945";
		test_data = g_list_prepend (test_data, td);
	}
	{
		QTestDate *td = g_new0 (QTestDate, 1);
		td->time = qof_time_new ();
		qof_time_set_secs (td->time, -1613826000);
		qof_time_set_nanosecs (td->time, 0);
		td->date = qof_date_new ();
		td->date->qd_year = 1918;
		td->date->qd_mon  = 11;
		td->date->qd_mday = 11;
		td->date->qd_hour = 11;
		td->date->qd_min  = 0;
		td->date->qd_sec  = G_GINT64_CONSTANT(0);
		td->string_list = NULL;
		td->string_list = g_list_prepend (td->string_list, "11/11/1918");
		td->string_list = g_list_prepend (td->string_list, "11/11/1918");
		td->string_list = g_list_prepend (td->string_list, "11.11.1918");
		td->string_list = g_list_prepend (td->string_list, "1918-11-11");
		td->string_list = g_list_prepend (td->string_list, "1918-11-11T11:00:00Z");
		td->string_list = g_list_reverse (td->string_list);
		td->id = "Armistice 1918";
		test_data = g_list_prepend (test_data, td);
	}
	{
		QTestDate *td = g_new0 (QTestDate, 1);
		td->time = qof_time_new ();
		qof_time_set_secs (td->time, G_GINT64_CONSTANT(-28502726400));
		qof_time_set_nanosecs (td->time, 0);
		td->date = qof_date_new ();
		td->date->qd_year = 1066;
		td->date->qd_mon  = 10;
		td->date->qd_mday = 14;
		td->date->qd_hour = 8;
		td->date->qd_min  = 0;
		td->date->qd_sec  = G_GINT64_CONSTANT(0);
		td->string_list = NULL;
		td->string_list = g_list_prepend (td->string_list, "10/14/1066");
		td->string_list = g_list_prepend (td->string_list, "14/10/1066");
		td->string_list = g_list_prepend (td->string_list, "14.10.1066");
		td->string_list = g_list_prepend (td->string_list, "1066-10-14");
		td->string_list = g_list_prepend (td->string_list, "1066-10-14T08:00:00Z");
		td->string_list = g_list_reverse (td->string_list);
		td->id = "Battle of Hastings, 1066";
		test_data = g_list_prepend (test_data, td);
	}/*
	{
		QTestDate *td = g_new0 (QTestDate, 1);
		td->time = qof_time_new ();
		qof_time_set_secs (td->time, G_GINT64_CONSTANT(-63512071200));
		qof_time_set_nanosecs (td->time, 0);
		td->date = qof_date_new ();
		td->date->qd_year = -43;
		td->date->qd_mon  = 5;
		td->date->qd_mday = 20;
		td->date->qd_hour = 14;
		td->date->qd_min  = 0;
		td->date->qd_sec  = G_GINT64_CONSTANT(0);
		td->string_list = NULL;
		td->string_list = g_list_prepend (td->string_list, "05/20/-043");
		td->string_list = g_list_prepend (td->string_list, "20/05/-043");
		td->string_list = g_list_prepend (td->string_list, "20.05.-043");
		td->string_list = g_list_prepend (td->string_list, "-043-05-20");
		td->string_list = g_list_prepend (td->string_list, "-043-05-20T14:00:00Z");
		td->string_list = g_list_reverse (td->string_list);
		td->id = "approx Roman invasion, 43BC";
		test_data = g_list_prepend (test_data, td);
	}*/
/*
//	scan_and_stamp ("-0751-05-20 00:00:00 +0000", 12, fails to parse with date.
		qd->qd_year = 3005;
		qd->qd_mon = 07;
		qd->qd_mday = 24;
		qd->qd_hour = 06;
		qd->qd_min  = 34;
		qd->qd_sec  = 26;
		qt = qof_date_to_qtime (qd);
		do_test ((0 == safe_strcasecmp ("07/24/3005",
			qofstrftime (QOF_DATE_FORMAT_US, qd))), 
			"strftime:US:forward");
		do_test ((0 == safe_strcasecmp ("24/07/3005",
			qofstrftime (QOF_DATE_FORMAT_UK, qd))), 
			"strftime:UK:forward");
		do_test ((0 == safe_strcasecmp ("24.07.3005",
			qofstrftime (QOF_DATE_FORMAT_CE, qd))), 
			"strftime:CE:forward");
		do_test ((0 == safe_strcasecmp ("3005-07-24",
			qofstrftime (QOF_DATE_FORMAT_ISO, qd))), 
			"strftime:ISO:forward");
		do_test ((0 == safe_strcasecmp ("3005-07-24T06:34:26Z",
			qofstrftime (QOF_DATE_FORMAT_UTC, qd))), 
			"strftime:UTC:forward");
		do_test ((0 != safe_strcasecmp (NULL,
			qofstrftime (QOF_DATE_FORMAT_LOCALE, qd))), 
			"strftime:LOCALE:forward:inrange");
		do_test ((0 != safe_strcasecmp (NULL,
			qofstrftime (QOF_DATE_FORMAT_CUSTOM, qd))), 
			"strftime:CUSTOM:forward:inrange");
		qof_date_free (qd);
		qd = qof_date_new ();
		qd->qd_year = 4500;
		qd->qd_mon = 07;
		qd->qd_mday = 24;
		qd->qd_hour = 06;
		qd->qd_min  = 34;
		qd->qd_sec  = 26;
		qt = qof_date_to_qtime (qd);
		do_test ((0 == safe_strcasecmp ("07/24/4500",
			qofstrftime (QOF_DATE_FORMAT_US, qd))), 
			"strftime:US:fifth");
		do_test ((0 == safe_strcasecmp ("24/07/4500",
			qofstrftime (QOF_DATE_FORMAT_UK, qd))), 
			"strftime:UK:fifth");
		do_test ((0 == safe_strcasecmp ("24.07.4500",
			qofstrftime (QOF_DATE_FORMAT_CE, qd))), 
			"strftime:CE:fifth");
		do_test ((0 == safe_strcasecmp ("4500-07-24",
			qofstrftime (QOF_DATE_FORMAT_ISO, qd))), 
			"strftime:ISO:fifth");
		do_test ((0 == safe_strcasecmp ("4500-07-24T06:34:26Z",
			qofstrftime (QOF_DATE_FORMAT_UTC, qd))), 
			"strftime:UTC:fifth");
		do_test ((0 != safe_strcasecmp (NULL,
			qofstrftime (QOF_DATE_FORMAT_LOCALE, qd))), 
			"strftime:LOCALE:fifth:inrange");
		do_test ((0 != safe_strcasecmp (NULL,
			qofstrftime (QOF_DATE_FORMAT_CUSTOM, qd))), 
			"strftime:CUSTOM:fifth:inrange");
		qof_date_free (qd);
		qd = qof_date_new ();
		qd->qd_year = 45000;
		qd->qd_mon = 07;
		qd->qd_mday = 24;
		qd->qd_hour = 06;
		qd->qd_min  = 34;
		qd->qd_sec  = 26;
		qt = qof_date_to_qtime (qd);
		do_test ((0 == safe_strcasecmp ("07/24/45000",
			qofstrftime (QOF_DATE_FORMAT_US, qd))), 
			"strftime:US:forward2");
		do_test ((0 == safe_strcasecmp ("24/07/45000",
			qofstrftime (QOF_DATE_FORMAT_UK, qd))), 
			"strftime:UK:forward2");
		do_test ((0 == safe_strcasecmp ("24.07.45000",
			qofstrftime (QOF_DATE_FORMAT_CE, qd))), 
			"strftime:CE:forward2");
		do_test ((0 == safe_strcasecmp ("45000-07-24",
			qofstrftime (QOF_DATE_FORMAT_ISO, qd))), 
			"strftime:ISO:forward2");
		do_test ((0 == safe_strcasecmp ("45000-07-24T06:34:26Z",
			qofstrftime (QOF_DATE_FORMAT_UTC, qd))), 
			"strftime:UTC:forward2");
		do_test ((0 != safe_strcasecmp (NULL,
			qofstrftime (QOF_DATE_FORMAT_LOCALE, qd))), 
			"strftime:LOCALE:forward2:outofrange");
		do_test ((0 != safe_strcasecmp (NULL,
			qofstrftime (QOF_DATE_FORMAT_CUSTOM, qd))), 
			"strftime:CUSTOM:forward2:outofrange");
		qof_date_free (qd);
		qd = qof_date_new ();
		qd->qd_year = 1914;
		qd->qd_mon = 11;
		qd->qd_mday = 11;
		qd->qd_hour = 11;
		qt = qof_date_to_qtime (qd);
		fprintf (stderr, "%s\n", qof_date_time_stamp (qt));
		fprintf (stderr, "should have been %s\n",
			"1914-11-11T11:00:00Z");
		do_test ((0 == safe_strcasecmp ("11/11/1914",
			qofstrftime (QOF_DATE_FORMAT_US, qd))), 
			"strftime:US:second");
		do_test ((0 == safe_strcasecmp ("11/11/1914",
			qofstrftime (QOF_DATE_FORMAT_UK, qd))), 
			"strftime:UK:second");
		do_test ((0 == safe_strcasecmp ("11.11.1914",
			qofstrftime (QOF_DATE_FORMAT_CE, qd))), 
			"strftime:CE:second");
		do_test ((0 == safe_strcasecmp ("1914-11-11",
			qofstrftime (QOF_DATE_FORMAT_ISO, qd))), 
			"strftime:ISO:second");
		do_test ((0 == safe_strcasecmp ("1914-11-11T11:00:00Z",
			qofstrftime (QOF_DATE_FORMAT_UTC, qd))), 
			"strftime:UTC:second");
		do_test ((0 != safe_strcasecmp (NULL,
			qofstrftime (QOF_DATE_FORMAT_LOCALE, qd))), 
			"strftime:LOCALE:second:inrange");
		do_test ((0 != safe_strcasecmp (NULL,
			qofstrftime (QOF_DATE_FORMAT_CUSTOM, qd))), 
			"strftime:CUSTOM:second:inrange");
		qof_date_free (qd);
		qof_time_free (qt);
		qd = qof_date_new ();
		qd->qd_year = 1812;
		qd->qd_mon = 07;
		qd->qd_mday = 24;
		qd->qd_hour = 06;
		qd->qd_min  = 34;
		qd->qd_sec  = 26;
		qt = qof_date_to_qtime (qd);
		do_test ((0 == safe_strcasecmp ("07/24/1812",
			qofstrftime (QOF_DATE_FORMAT_US, qd))), 
			"strftime:US:third");
		do_test ((0 == safe_strcasecmp ("24/07/1812",
			qofstrftime (QOF_DATE_FORMAT_UK, qd))), 
			"strftime:UK:third");
		do_test ((0 == safe_strcasecmp ("24.07.1812",
			qofstrftime (QOF_DATE_FORMAT_CE, qd))), 
			"strftime:CE:third");
		do_test ((0 == safe_strcasecmp ("1812-07-24",
			qofstrftime (QOF_DATE_FORMAT_ISO, qd))), 
			"strftime:ISO:third");
		do_test ((0 == safe_strcasecmp ("1812-07-24T06:34:26Z",
			qofstrftime (QOF_DATE_FORMAT_UTC, qd))), 
			"strftime:UTC:third");
		do_test ((0 == safe_strcasecmp (NULL,
			qofstrftime (QOF_DATE_FORMAT_LOCALE, qd))), 
			"strftime:LOCALE:third:outofrange");
		do_test ((0 == safe_strcasecmp (NULL,
			qofstrftime (QOF_DATE_FORMAT_CUSTOM, qd))), 
			"strftime:CUSTOM:third:outofrange");
		qof_date_free (qd);
		qd = qof_date_new ();
		qd->qd_year = 1066;
		qd->qd_mon = 07;
		qd->qd_mday = 24;
		qd->qd_hour = 06;
		qd->qd_min  = 34;
		qd->qd_sec  = 26;
		qt = qof_date_to_qtime (qd);
		do_test ((0 == safe_strcasecmp ("07/24/1066",
			qofstrftime (QOF_DATE_FORMAT_US, qd))), 
			"strftime:US:fourth");
		do_test ((0 == safe_strcasecmp ("24/07/1066",
			qofstrftime (QOF_DATE_FORMAT_UK, qd))), 
			"strftime:UK:fourth");
		do_test ((0 == safe_strcasecmp ("24.07.1066",
			qofstrftime (QOF_DATE_FORMAT_CE, qd))), 
			"strftime:CE:fourth");
		do_test ((0 == safe_strcasecmp ("1066-07-24",
			qofstrftime (QOF_DATE_FORMAT_ISO, qd))), 
			"strftime:ISO:fourth");
		do_test ((0 == safe_strcasecmp ("1066-07-24T06:34:26Z",
			qofstrftime (QOF_DATE_FORMAT_UTC, qd))), 
			"strftime:UTC:fourth");
		do_test ((0 == safe_strcasecmp (NULL,
			qofstrftime (QOF_DATE_FORMAT_LOCALE, qd))), 
			"strftime:LOCALE:fourth:outofrange");
		do_test ((0 == safe_strcasecmp (NULL,
			qofstrftime (QOF_DATE_FORMAT_CUSTOM, qd))), 
			"strftime:CUSTOM:fourth:outofrange");
		qof_date_free (qd);
		qd = qof_date_new ();
		qd->qd_year = -45;
		qd->qd_mon = 07;
		qd->qd_mday = 24;
		qd->qd_hour = 06;
		qd->qd_min  = 34;
		qd->qd_sec  = 26;
		qt = qof_date_to_qtime (qd);
		do_test ((0 == safe_strcasecmp ("07/24/-045",
			qofstrftime (QOF_DATE_FORMAT_US, qd))), 
			"strftime:US:fifth");
		do_test ((0 == safe_strcasecmp ("24/07/-045",
			qofstrftime (QOF_DATE_FORMAT_UK, qd))), 
			"strftime:UK:fifth");
		do_test ((0 == safe_strcasecmp ("24.07.-045",
			qofstrftime (QOF_DATE_FORMAT_CE, qd))), 
			"strftime:CE:fifth");
		do_test ((0 == safe_strcasecmp ("-045-07-24",
			qofstrftime (QOF_DATE_FORMAT_ISO, qd))), 
			"strftime:ISO:fifth");
		do_test ((0 == safe_strcasecmp ("-045-07-24T06:34:26Z",
			qofstrftime (QOF_DATE_FORMAT_UTC, qd))), 
			"strftime:UTC:fifth");
		do_test ((0 == safe_strcasecmp (NULL,
			qofstrftime (QOF_DATE_FORMAT_LOCALE, qd))), 
			"strftime:LOCALE:fifth:outofrange");
		do_test ((0 == safe_strcasecmp (NULL,
			qofstrftime (QOF_DATE_FORMAT_CUSTOM, qd))), 
			"strftime:CUSTOM:fifth:outofrange");
		qof_date_free (qd);
		qd = qof_date_new ();
		qd->qd_year = -4500;
		qd->qd_mon = 07;
		qd->qd_mday = 24;
		qd->qd_hour = 06;
		qd->qd_min  = 34;
		qd->qd_sec  = 26;
		qt = qof_date_to_qtime (qd);
		do_test ((0 == safe_strcasecmp ("07/24/-4500",
			qofstrftime (QOF_DATE_FORMAT_US, qd))), 
			"strftime:US:sixth");
		do_test ((0 == safe_strcasecmp ("24/07/-4500",
			qofstrftime (QOF_DATE_FORMAT_UK, qd))), 
			"strftime:UK:sixth");
		do_test ((0 == safe_strcasecmp ("24.07.-4500",
			qofstrftime (QOF_DATE_FORMAT_CE, qd))), 
			"strftime:CE:sixth");
		do_test ((0 == safe_strcasecmp ("-4500-07-24",
			qofstrftime (QOF_DATE_FORMAT_ISO, qd))), 
			"strftime:ISO:sixth");
		do_test ((0 == safe_strcasecmp ("-4500-07-24T06:34:26Z",
			qofstrftime (QOF_DATE_FORMAT_UTC, qd))), 
			"strftime:UTC:sixth");
		do_test ((0 == safe_strcasecmp (NULL,
			qofstrftime (QOF_DATE_FORMAT_LOCALE, qd))), 
			"strftime:LOCALE:sixth:outofrange");
		do_test ((0 == safe_strcasecmp (NULL,
			qofstrftime (QOF_DATE_FORMAT_CUSTOM, qd))), 
			"strftime:CUSTOM:sixth:outofrange");
		qof_date_free (qd);
		qd = qof_date_new ();
		qd->qd_year = -4500000;
		qd->qd_mon = 07;
		qd->qd_mday = 24;
		qd->qd_hour = 06;
		qd->qd_min  = 34;
		qd->qd_sec  = 26;
		qt = qof_date_to_qtime (qd);
		do_test ((0 == safe_strcasecmp ("07/24/-4500000",
			qofstrftime (QOF_DATE_FORMAT_US, qd))), 
			"strftime:US:seventh");
		do_test ((0 == safe_strcasecmp ("24/07/-4500000",
			qofstrftime (QOF_DATE_FORMAT_UK, qd))), 
			"strftime:UK:seventh");
		do_test ((0 == safe_strcasecmp ("24.07.-4500000",
			qofstrftime (QOF_DATE_FORMAT_CE, qd))), 
			"strftime:CE:seventh");
		do_test ((0 == safe_strcasecmp ("-4500000-07-24",
			qofstrftime (QOF_DATE_FORMAT_ISO, qd))), 
			"strftime:ISO:seventh");
		do_test ((0 == safe_strcasecmp ("-4500000-07-24T06:34:26Z",
			qofstrftime (QOF_DATE_FORMAT_UTC, qd))), 
			"strftime:UTC:seventh");
		do_test ((0 == safe_strcasecmp (NULL,
			qofstrftime (QOF_DATE_FORMAT_LOCALE, qd))), 
			"strftime:LOCALE:seventh:outofrange");
		do_test ((0 == safe_strcasecmp (NULL,
			qofstrftime (QOF_DATE_FORMAT_CUSTOM, qd))), 
			"strftime:CUSTOM:seventh:outofrange");
		qof_date_free (qd);
*/
}

static void
free_test_data (gpointer data, gpointer user_data)
{
	QTestDate *td;

	td = (QTestDate*)data;
	qof_date_free (td->date);
	qof_time_free (td->time);
	td->string_list = NULL;
	g_free (td);
	td = NULL;
}

static void
test_date_close (void)
{
	g_list_foreach (test_data, free_test_data, NULL);
}

static void
scan_and_stamp (const gchar * str, QofDateFormat df, QofTimeSecs check)
{
	QofDateFormat old;
	QofTime *scan;
	gchar *stamp;

	old = qof_date_format_get_current ();
	qof_date_format_set_current (df);
	scan = qof_date_to_qtime (qof_date_parse (str, df));
	do_test ((scan != NULL), "scan failed");
	if (scan == NULL)
		return;
	do_test ((qof_time_get_secs (scan) == check), "wrong time value");
	stamp = qof_date_print (qof_date_from_qtime(scan), df);
	do_test ((stamp != NULL), "stamp failed");
	/* timezone tests mean stamp cannot be compared to str */
	qof_time_free (scan);
	qof_date_format_set_current (old);
}

static void
stamp_and_scan (QofTimeSecs start, glong nanosecs, 
	QofDateFormat df)
{
	gchar *str1, *str2;
	QofDate *check;
	QofTime *time, *scan;
	QofDateFormat old;

	time = qof_time_new ();
	qof_time_set_secs (time, start);
	qof_time_set_nanosecs (time, nanosecs);
	old = qof_date_format_get_current ();
	qof_date_format_set_current (df);
	str1 = qof_date_print (qof_date_from_qtime(time), df);
	do_test ((str1 != NULL), "stamp failed");
	if (!str1)
		return;
	check = qof_date_parse (str1, df);
	do_test ((check != NULL), "parse failed");
	if (!check)
		fprintf (stderr, "tried to parse %s\n", str1);
	scan = qof_date_to_qtime (check);
	qof_date_free (check);
	do_test ((scan != NULL), "scan failed");
	if (!scan)
		return;
	/* depending on the format, data may be lost here
	as some formats do not encode the time. So only
	test that the returned stamp is the same. */
	str2 = qof_date_print (qof_date_from_qtime (scan), df);
	/* 2 digit year errors with format 6  */
	if (str2 == NULL)
		fprintf (stderr, "str2 is null if str1=%s df=%d\n", 
			str1, df);
	if (qof_date_from_qtime (scan) == NULL)
		fprintf (stderr, "from_qtime failed\n");
/*	do_test ((str2 != NULL), "print failed");
	do_test ((0 == safe_strcasecmp (str1, str2)), 
		"stamp different to scan");*/
	qof_time_free (scan);
	qof_time_free (time);
	g_free (str1);
	g_free (str2);
	qof_date_format_set_current (old);
}

static void
run_print_scan_tests (void)
{
	gint i;
	gint64 secs;

	/* Set a secs value at the limit of the range of time_t 
	   on 32bit systems. */
	secs = G_MAXINT32;
	/* add ten days */
	secs += SECS_PER_DAY * 10;
	/** \todo 2 digit year errors with format 6  */
	for (i = 1; i <= 5; i++)
/*	for (i = 1; i <= DATE_FORMAT_LAST; i++)*/
	{
		stamp_and_scan (796179600, 0, i);
		stamp_and_scan (796179500, 72000, i);
		stamp_and_scan (152098136, 0, i);
		stamp_and_scan (1964049931, 0, i);
		stamp_and_scan (1162088421, 12548000, i);
		stamp_and_scan (325659000 - 6500, 0, i);
		stamp_and_scan (1143943200, 0, i);
		stamp_and_scan (1603591171, 595311000, i);
		stamp_and_scan (1738909365, 204102000, i);
		stamp_and_scan (1603591171, 595311000, i);
		stamp_and_scan (1143943200 - 1, 0, i);
		stamp_and_scan (1143943200, 0, i);
		stamp_and_scan (1143943200 + (7 * 60 * 60), 0, i);
		stamp_and_scan (1143943200 + (8 * 60 * 60), 0, i);
		stamp_and_scan (1841443200, 0, i);
		/* work with early dates */

		stamp_and_scan (G_GINT64_CONSTANT (-796179600),   0, i);
		stamp_and_scan (G_GINT64_CONSTANT (-152098136),   0, i);
		stamp_and_scan (G_GINT64_CONSTANT (-1143943200),  0, i);
		stamp_and_scan (G_GINT64_CONSTANT (-1964049931),  0, i);
		stamp_and_scan (G_GINT64_CONSTANT (-2463880447),  0, i);
		stamp_and_scan (G_GINT64_CONSTANT (-28502726400), 0, i);
		stamp_and_scan (G_GINT64_CONSTANT (-60798211200), 0, i);
		stamp_and_scan (G_GINT64_CONSTANT (-32727638740), 0, i);
//		stamp_and_scan (G_GINT64_CONSTANT (-86956848000), 0, i);
		stamp_and_scan (secs, 0, i);
		/* Wed 29 Jan 2048 03:14:07 UTC */
		secs = G_GINT64_CONSTANT (2463880447);
		stamp_and_scan (secs, 0, i);
		/* Sat 29 Jan 2050 03:14:07 UTC */
		secs = G_GINT64_CONSTANT (2527038847);
		stamp_and_scan (secs, 0, i);
		/* work with far future dates */
		/* 1 year = 31536000 seconds */
		/* enable once QofDate is sorted */
		/* 32,727,638,740 approx 3007*/
		secs = G_GINT64_CONSTANT (32727638740);
		stamp_and_scan (secs, 0, i);
		/* 88,313,632,867 approx 4770 */
		secs = G_GINT64_CONSTANT (88313632867);
		stamp_and_scan (secs, 0, i);
		/* 189,216,632,865 approx 7970 */
		secs = G_GINT64_CONSTANT (189216632865);
		stamp_and_scan (secs, 0, i);
		/* 378,432,632,864 approx 13,970  */
/*		secs = G_GINT64_CONSTANT (378432632864);
		stamp_and_scan (secs, 0, i);*/
		/* 3165071328567 approx 102,333 */
/*		secs = G_GINT64_CONSTANT (3165071328567);
		stamp_and_scan (secs, 0, i);*/
	}
	scan_and_stamp ("05/09/2006", QOF_DATE_FORMAT_US, 
		1147132800);
	scan_and_stamp ("09/05/2006", QOF_DATE_FORMAT_UK, 
		1147132800);
	scan_and_stamp ("09.05.2006", QOF_DATE_FORMAT_CE, 
		1147132800);
	scan_and_stamp ("2006-05-09", QOF_DATE_FORMAT_ISO, 
		1147132800);
	scan_and_stamp ("2006-05-09T00:00:00Z", QOF_DATE_FORMAT_UTC, 
		1147132800);
	scan_and_stamp ("2006-05-09T14:49:04Z", QOF_DATE_FORMAT_UTC, 
		1147186144);

	/* test a custom format */
	do_test ((qof_date_format_add ("%Y-%m-%d %H:%M:%S %z", 
		12) == TRUE), "failed to add scan suitable format");
	/* 1972-01-01T00:00:00Z */
	scan_and_stamp ("1971-12-31 15:00:00 -0900", 12, 63072000);
	/* 1980-01-01T00:00:00Z */
	scan_and_stamp ("1979-12-31 15:00:00 -0900", 12, 315532800);
	scan_and_stamp ("1980-01-01 00:00:00 -0000", 12, 315532800);
	scan_and_stamp ("1980-01-01 00:00:00 +0000", 12, 315532800);
	scan_and_stamp ("1980-01-01 09:00:00 +0900", 12, 315532800);
	scan_and_stamp ("1980-01-01 08:30:00 +0830", 12, 315532800);
	/* pre-1970 dates */
	/* enable once time_t replaced */
//	scan_and_stamp ("1963-11-22 14:00:00 -0500", 12, -192776400);
//	scan_and_stamp ("1945-09-08 11:02:00 +0900", 12, -767311080);
	scan_and_stamp ("1918-11-11 11:00:00 +0000", 12, -1613826000);
	/* work with really early dates */
	/* 14th October 1066 (time is just a guess) */
	scan_and_stamp ("1066-10-14 08:00:00 +0000", 12, 
		G_GINT64_CONSTANT (-28502726400));
	/* May 43AD Roman invasion (day and time guessed) */
/*	scan_and_stamp ("0043-05-20 14:00:00 +0000", 12, 
		G_GINT64_CONSTANT (-60798211200));*/
	/* 751BC - end of the Bronze Age. (day and time arbitrary) */
/*	scan_and_stamp ("-0751-05-20 00:00:00 +0000", 12, fails to parse with date.*/
	/* should be around -86956848000 */
	{
		QofTime *time;
		gint64 secs;

		time = qof_time_new ();
		qof_time_set_secs (time, 1147186210);
		do_test ((qof_date_time_add_days (time, 45) == TRUE),
				 "add_days failed");
		secs = 1147186210 + (45 * SECS_PER_DAY);
		do_test ((secs == qof_time_get_secs (time)),
				 "add_days gave incorrect result.");
		qof_time_set_secs (time, 1147186210);
		do_test ((qof_date_time_add_months (time, 50, TRUE) == TRUE),
				 "add_months failed");
		do_test ((1278687010 == qof_time_get_secs (time)),
				 "add_months gave incorrect result.");
		qof_time_free (time);
	}
}

static void
run_qofdate_test (void)
{
	QofDateFormat df, test;

	df = qof_date_format_get_current ();
	/* default date format tests */
	{
		qof_date_format_set_current (QOF_DATE_FORMAT_UK);
		test = qof_date_format_get_current ();
		do_test ((test == QOF_DATE_FORMAT_UK), "setting current format");
		do_test ((safe_strcasecmp (qof_date_format_to_name (test), "uk") ==
				  0), "getting the shorthand name");
		do_test ((FALSE ==
				  qof_date_format_set_name ("foo", QOF_DATE_FORMAT_UK)),
				 "default name should not be overridden");
		do_test ((QOF_DATE_FORMAT_UK == qof_date_format_from_name ("uk")),
				 "getting date format from shorthand name");
		do_test (('/' == qof_date_format_get_date_separator (test)),
				 "getting date format separator from date format");
		do_test ((FALSE == qof_date_format_set_date_separator (':', test)),
				 "default separator should not be overridden");
	}
	/* custom date format tests */
	{
		do_test ((qof_date_format_add ("%T", 10) == TRUE),
				 "adding a valid format");
		do_test ((qof_date_format_add ("%a%A%b%B%c%C%d%D%e%F%f%r", 11) ==
				  FALSE), "adding an invalid format");
		qof_date_format_set_current (10);
		test = qof_date_format_get_current ();
		do_test ((test == 10), "setting current format");
		do_test ((safe_strcasecmp (qof_date_format_to_name (test), "%T") ==
				  0), "getting the shorthand name");
		do_test ((TRUE == qof_date_format_set_name ("foo", test)),
				 "custom name should be overridden");
		do_test ((test == qof_date_format_from_name ("foo")),
				 "getting date format from shorthand name");
		do_test (('\0' == qof_date_format_get_date_separator (test)),
				 "getting date format separator from date format");
		do_test ((TRUE == qof_date_format_set_date_separator (':', test)),
				 "custom separator should be overridden");
		do_test ((':' == qof_date_format_get_date_separator (test)),
				 "getting modified date format separator from date format");
	}
	qof_date_format_set_current (df);
	/* run tests on date_normalise */
	{
		QofDate *date;

		date = qof_date_new ();
		date->qd_sec = SECS_PER_DAY * 2 + 30;
		date->qd_year = 2000;
		do_test ((qof_date_valid (date) == TRUE), "date 1 was invalid");
		do_test ((date->qd_sec == 30), "normalised seconds incorrect - 1");
		do_test ((date->qd_mday == 2), "normalised day incorrect - 1");
		date->qd_mday = 54;
		do_test ((qof_date_valid (date) == TRUE), "date 2 was invalid");
		do_test ((date->qd_sec == 30), "normalised seconds incorrect - 2");
		do_test ((date->qd_mday == 23), "normalised day incorrect - 2");
		date->qd_hour = 34;
		qof_date_valid (date);
		do_test ((date->qd_hour == 10), "normalised hour incorrect");
		do_test ((date->qd_mday == 24), "normalised day incorrect - 3");
		date->qd_mon = 17;
		qof_date_valid (date);
		do_test ((date->qd_mon == 5), "normalised month incorrect");
		do_test ((date->qd_year == 2001), "normalised year incorrect");
		date->qd_hour = 27;
		qof_date_valid (date);
		do_test ((date->qd_hour == 3), "normalised hour incorrect - 1");
		do_test ((date->qd_mday == 25), "normalised day incorrect - 4");
		date->qd_hour = -53;
		qof_date_valid (date);
		do_test ((date->qd_hour == 19), "normalised hour incorrect - 2");
		do_test ((date->qd_mday == 22), "normalised day incorrect - 5");
		date->qd_min = -225;
		qof_date_valid (date);
		do_test ((date->qd_min == 15), "normalised min incorrect");
		do_test ((date->qd_hour == 15), "normalised hour incorrect - 3");
		date->qd_min = 5;
		date->qd_sec = 68;
		qof_date_valid (date);
		do_test ((date->qd_sec == 8), "doxygen sec example incorrect - 1");
		do_test ((date->qd_min == 6),  "doxygen min example incorrect - 1");
		date->qd_min = 5;
		date->qd_sec = -64;
		qof_date_valid (date);
		do_test ((date->qd_sec == 56), "doxygen sec example incorrect - 2");
		do_test ((date->qd_min == 3),  "doxygen min example incorrect - 2");
	}
	/* run tests against QofDate<->QofTime conversions. */
	{
		QofDate *qd;
		QofTime *qt;

		qd = qof_date_new ();
		qd->qd_year = 2006;
		qd->qd_mon = 5;
		/* normalising mishandles qd_yday as
		qd_mday and then messes that up too */
		qd->qd_mday = 30;
		qd->qd_hour = 18;
		qd->qd_min = 24;
		qd->qd_sec = 17;
		qd->qd_nanosecs = 123456789;
		do_test ((qof_date_valid (qd)), "date not valid");
		qt = qof_date_to_qtime (qd);
		fprintf (stderr, "%s\n", qof_date_print (qd, 
			QOF_DATE_FORMAT_UTC));
		fprintf (stderr, "should have been %s\n",
			"2006-05-30T18:24:17Z");
		do_test ((qof_time_get_nanosecs (qt) ==
			123456789L), "nanosecs mismatched.");
		do_test ((0 == safe_strcasecmp ("05/30/2006",
			qof_date_print (qd, QOF_DATE_FORMAT_US))), 
			"strftime:US:first");
		do_test ((0 == safe_strcasecmp ("30/05/2006",
			qof_date_print (qd, QOF_DATE_FORMAT_UK))), 
			"strftime:UK:first");
		do_test ((0 == safe_strcasecmp ("30.05.2006",
			qof_date_print (qd, QOF_DATE_FORMAT_CE))), 
			"strftime:CE:first");
		do_test ((0 == safe_strcasecmp ("2006-05-30",
			qof_date_print (qd, QOF_DATE_FORMAT_ISO))), 
			"strftime:ISO:first");
		do_test ((0 == safe_strcasecmp ("2006-05-30T18:24:17Z",
			qof_date_print (qd, QOF_DATE_FORMAT_UTC))), 
			"strftime:UTC:first");
		do_test ((0 != safe_strcasecmp (NULL,
			qof_date_print (qd, QOF_DATE_FORMAT_LOCALE))), 
			"strftime:LOCALE:first:inrange");
		do_test ((0 != safe_strcasecmp (NULL,
			qof_date_print (qd, QOF_DATE_FORMAT_CUSTOM))), 
			"strftime:CUSTOM:first:inrange");
		qof_date_free (qd);
		qof_time_free (qt);
		qd = qof_date_new ();
		qd->qd_year = 3005;
		qd->qd_mon = 07;
		qd->qd_mday = 24;
		qd->qd_hour = 06;
		qd->qd_min  = 34;
		qd->qd_sec  = 26;
		qt = qof_date_to_qtime (qd);
		do_test ((0 == safe_strcasecmp ("07/24/3005",
			qof_date_print (qd, QOF_DATE_FORMAT_US))), 
			"strftime:US:forward");
		do_test ((0 == safe_strcasecmp ("24/07/3005",
			qof_date_print (qd, QOF_DATE_FORMAT_UK))), 
			"strftime:UK:forward");
		do_test ((0 == safe_strcasecmp ("24.07.3005",
			qof_date_print (qd, QOF_DATE_FORMAT_CE))), 
			"strftime:CE:forward");
		do_test ((0 == safe_strcasecmp ("3005-07-24",
			qof_date_print (qd, QOF_DATE_FORMAT_ISO))), 
			"strftime:ISO:forward");
		do_test ((0 == safe_strcasecmp ("3005-07-24T06:34:26Z",
			qof_date_print (qd, QOF_DATE_FORMAT_UTC))), 
			"strftime:UTC:forward");
		do_test ((0 != safe_strcasecmp (NULL,
			qof_date_print (qd, QOF_DATE_FORMAT_LOCALE))), 
			"strftime:LOCALE:forward:inrange");
		do_test ((0 != safe_strcasecmp (NULL,
			qof_date_print (qd, QOF_DATE_FORMAT_CUSTOM))), 
			"strftime:CUSTOM:forward:inrange");
		qof_date_free (qd);
		qd = qof_date_new ();
		qd->qd_year = 4500;
		qd->qd_mon = 07;
		qd->qd_mday = 24;
		qd->qd_hour = 06;
		qd->qd_min  = 34;
		qd->qd_sec  = 26;
		qt = qof_date_to_qtime (qd);
		do_test ((0 == safe_strcasecmp ("07/24/4500",
			qof_date_print (qd, QOF_DATE_FORMAT_US))), 
			"strftime:US:fifth");
		do_test ((0 == safe_strcasecmp ("24/07/4500",
			qof_date_print (qd, QOF_DATE_FORMAT_UK))), 
			"strftime:UK:fifth");
		do_test ((0 == safe_strcasecmp ("24.07.4500",
			qof_date_print (qd, QOF_DATE_FORMAT_CE))), 
			"strftime:CE:fifth");
		do_test ((0 == safe_strcasecmp ("4500-07-24",
			qof_date_print (qd, QOF_DATE_FORMAT_ISO))), 
			"strftime:ISO:fifth");
		do_test ((0 == safe_strcasecmp ("4500-07-24T06:34:26Z",
			qof_date_print (qd, QOF_DATE_FORMAT_UTC))), 
			"strftime:UTC:fifth");
		/* future values are being parsed wrongly. */
		do_test ((0 != safe_strcasecmp (NULL,
			qof_date_print (qd, QOF_DATE_FORMAT_LOCALE))), 
			"strftime:LOCALE:fifth:inrange");
		do_test ((0 != safe_strcasecmp (NULL,
			qof_date_print (qd, QOF_DATE_FORMAT_CUSTOM))), 
			"strftime:CUSTOM:fifth:inrange");
		qof_date_free (qd);
		qd = qof_date_new ();
		qd->qd_year = 45000;
		qd->qd_mon = 07;
		qd->qd_mday = 24;
		qd->qd_hour = 06;
		qd->qd_min  = 34;
		qd->qd_sec  = 26;
		qt = qof_date_to_qtime (qd);
		do_test ((0 == safe_strcasecmp ("07/24/45000",
			qof_date_print (qd, QOF_DATE_FORMAT_US))), 
			"strftime:US:forward2");
		do_test ((0 == safe_strcasecmp ("24/07/45000",
			qof_date_print (qd, QOF_DATE_FORMAT_UK))), 
			"strftime:UK:forward2");
		do_test ((0 == safe_strcasecmp ("24.07.45000",
			qof_date_print (qd, QOF_DATE_FORMAT_CE))), 
			"strftime:CE:forward2");
		do_test ((0 == safe_strcasecmp ("45000-07-24",
			qof_date_print (qd, QOF_DATE_FORMAT_ISO))), 
			"strftime:ISO:forward2");
		do_test ((0 == safe_strcasecmp ("45000-07-24T06:34:26Z",
			qof_date_print (qd, QOF_DATE_FORMAT_UTC))), 
			"strftime:UTC:forward2");
		do_test ((0 != safe_strcasecmp (NULL,
			qof_date_print (qd, QOF_DATE_FORMAT_LOCALE))), 
			"strftime:LOCALE:forward2:outofrange");
		do_test ((0 != safe_strcasecmp (NULL,
			qof_date_print (qd, QOF_DATE_FORMAT_CUSTOM))), 
			"strftime:CUSTOM:forward2:outofrange");
		qof_date_free (qd);
		qd = qof_date_new ();
		qd->qd_year = 1914;
		qd->qd_mon = 11;
		qd->qd_mday = 11;
		qd->qd_hour = 11;
		qt = qof_date_to_qtime (qd);
		fprintf (stderr, "%s\n", qof_date_print (qd, 
			QOF_DATE_FORMAT_UTC));
		fprintf (stderr, "should have been %s\n",
			"1914-11-11T11:00:00Z");
		do_test ((0 == safe_strcasecmp ("11/11/1914",
			qof_date_print (qd, QOF_DATE_FORMAT_US))), 
			"strftime:US:second");
		do_test ((0 == safe_strcasecmp ("11/11/1914",
			qof_date_print (qd, QOF_DATE_FORMAT_UK))), 
			"strftime:UK:second");
		do_test ((0 == safe_strcasecmp ("11.11.1914",
			qof_date_print (qd, QOF_DATE_FORMAT_CE))), 
			"strftime:CE:second");
		do_test ((0 == safe_strcasecmp ("1914-11-11",
			qof_date_print (qd, QOF_DATE_FORMAT_ISO))), 
			"strftime:ISO:second");
		do_test ((0 == safe_strcasecmp ("1914-11-11T11:00:00Z",
			qof_date_print (qd, QOF_DATE_FORMAT_UTC))), 
			"strftime:UTC:second");
		do_test ((0 != safe_strcasecmp (NULL,
			qof_date_print (qd, QOF_DATE_FORMAT_LOCALE))), 
			"strftime:LOCALE:second:inrange");
		do_test ((0 != safe_strcasecmp (NULL,
			qof_date_print (qd, QOF_DATE_FORMAT_CUSTOM))), 
			"strftime:CUSTOM:second:inrange");
		qof_date_free (qd);
		qof_time_free (qt);
		qd = qof_date_new ();
		qd->qd_year = 1812;
		qd->qd_mon = 07;
		qd->qd_mday = 24;
		qd->qd_hour = 06;
		qd->qd_min  = 34;
		qd->qd_sec  = 26;
		qt = qof_date_to_qtime (qd);
		do_test ((0 == safe_strcasecmp ("07/24/1812",
			qof_date_print (qd, QOF_DATE_FORMAT_US))), 
			"strftime:US:third");
		do_test ((0 == safe_strcasecmp ("24/07/1812",
			qof_date_print (qd, QOF_DATE_FORMAT_UK))), 
			"strftime:UK:third");
		do_test ((0 == safe_strcasecmp ("24.07.1812",
			qof_date_print (qd, QOF_DATE_FORMAT_CE))), 
			"strftime:CE:third");
		do_test ((0 == safe_strcasecmp ("1812-07-24",
			qof_date_print (qd, QOF_DATE_FORMAT_ISO))), 
			"strftime:ISO:third");
		do_test ((0 == safe_strcasecmp ("1812-07-24T06:34:26Z",
			qof_date_print (qd, QOF_DATE_FORMAT_UTC))), 
			"strftime:UTC:third");
		do_test ((0 == safe_strcasecmp (NULL,
			qof_date_print (qd, QOF_DATE_FORMAT_LOCALE))), 
			"strftime:LOCALE:third:outofrange");
		do_test ((0 == safe_strcasecmp (NULL,
			qof_date_print (qd, QOF_DATE_FORMAT_CUSTOM))), 
			"strftime:CUSTOM:third:outofrange");
		qof_date_free (qd);
		qd = qof_date_new ();
		qd->qd_year = 1066;
		qd->qd_mon = 07;
		qd->qd_mday = 24;
		qd->qd_hour = 06;
		qd->qd_min  = 34;
		qd->qd_sec  = 26;
		qt = qof_date_to_qtime (qd);
		do_test ((0 == safe_strcasecmp ("07/24/1066",
			qof_date_print (qd, QOF_DATE_FORMAT_US))), 
			"strftime:US:fourth");
		do_test ((0 == safe_strcasecmp ("24/07/1066",
			qof_date_print (qd, QOF_DATE_FORMAT_UK))), 
			"strftime:UK:fourth");
		do_test ((0 == safe_strcasecmp ("24.07.1066",
			qof_date_print (qd, QOF_DATE_FORMAT_CE))), 
			"strftime:CE:fourth");
		do_test ((0 == safe_strcasecmp ("1066-07-24",
			qof_date_print (qd, QOF_DATE_FORMAT_ISO))), 
			"strftime:ISO:fourth");
		do_test ((0 == safe_strcasecmp ("1066-07-24T06:34:26Z",
			qof_date_print (qd, QOF_DATE_FORMAT_UTC))), 
			"strftime:UTC:fourth");
		do_test ((0 == safe_strcasecmp (NULL,
			qof_date_print (qd, QOF_DATE_FORMAT_LOCALE))), 
			"strftime:LOCALE:fourth:outofrange");
		do_test ((0 == safe_strcasecmp (NULL,
			qof_date_print (qd, QOF_DATE_FORMAT_CUSTOM))), 
			"strftime:CUSTOM:fourth:outofrange");
		qof_date_free (qd);
		qd = qof_date_new ();
		qd->qd_year = -45;
		qd->qd_mon = 07;
		qd->qd_mday = 24;
		qd->qd_hour = 06;
		qd->qd_min  = 34;
		qd->qd_sec  = 26;
		qt = qof_date_to_qtime (qd);
		do_test ((0 == safe_strcasecmp ("07/24/-045",
			qof_date_print (qd, QOF_DATE_FORMAT_US))), 
			"strftime:US:fifth");
		do_test ((0 == safe_strcasecmp ("24/07/-045",
			qof_date_print (qd, QOF_DATE_FORMAT_UK))), 
			"strftime:UK:fifth");
		do_test ((0 == safe_strcasecmp ("24.07.-045",
			qof_date_print (qd, QOF_DATE_FORMAT_CE))), 
			"strftime:CE:fifth");
		do_test ((0 == safe_strcasecmp ("-045-07-24",
			qof_date_print (qd, QOF_DATE_FORMAT_ISO))), 
			"strftime:ISO:fifth");
		do_test ((0 == safe_strcasecmp ("-045-07-24T06:34:26Z",
			qof_date_print (qd, QOF_DATE_FORMAT_UTC))), 
			"strftime:UTC:fifth");
		do_test ((0 == safe_strcasecmp (NULL,
			qof_date_print (qd, QOF_DATE_FORMAT_LOCALE))), 
			"strftime:LOCALE:fifth:outofrange");
		do_test ((0 == safe_strcasecmp (NULL,
			qof_date_print (qd, QOF_DATE_FORMAT_CUSTOM))), 
			"strftime:CUSTOM:fifth:outofrange");
		qof_date_free (qd);
		qd = qof_date_new ();
		qd->qd_year = -4500;
		qd->qd_mon = 07;
		qd->qd_mday = 24;
		qd->qd_hour = 06;
		qd->qd_min  = 34;
		qd->qd_sec  = 26;
		qt = qof_date_to_qtime (qd);
		do_test ((0 == safe_strcasecmp ("07/24/-4500",
			qof_date_print (qd, QOF_DATE_FORMAT_US))), 
			"strftime:US:sixth");
		do_test ((0 == safe_strcasecmp ("24/07/-4500",
			qof_date_print (qd, QOF_DATE_FORMAT_UK))), 
			"strftime:UK:sixth");
		do_test ((0 == safe_strcasecmp ("24.07.-4500",
			qof_date_print (qd, QOF_DATE_FORMAT_CE))), 
			"strftime:CE:sixth");
		do_test ((0 == safe_strcasecmp ("-4500-07-24",
			qof_date_print (qd, QOF_DATE_FORMAT_ISO))), 
			"strftime:ISO:sixth");
		do_test ((0 == safe_strcasecmp ("-4500-07-24T06:34:26Z",
			qof_date_print (qd, QOF_DATE_FORMAT_UTC))), 
			"strftime:UTC:sixth");
		do_test ((0 == safe_strcasecmp (NULL,
			qof_date_print (qd, QOF_DATE_FORMAT_LOCALE))), 
			"strftime:LOCALE:sixth:outofrange");
		do_test ((0 == safe_strcasecmp (NULL,
			qof_date_print (qd, QOF_DATE_FORMAT_CUSTOM))), 
			"strftime:CUSTOM:sixth:outofrange");
		qof_date_free (qd);
		qd = qof_date_new ();
		qd->qd_year = -4500000;
		qd->qd_mon = 07;
		qd->qd_mday = 24;
		qd->qd_hour = 06;
		qd->qd_min  = 34;
		qd->qd_sec  = 26;
		qt = qof_date_to_qtime (qd);
		do_test ((0 == safe_strcasecmp ("07/24/-4500000",
			qof_date_print (qd, QOF_DATE_FORMAT_US))), 
			"strftime:US:seventh");
		do_test ((0 == safe_strcasecmp ("24/07/-4500000",
			qof_date_print (qd, QOF_DATE_FORMAT_UK))), 
			"strftime:UK:seventh");
		do_test ((0 == safe_strcasecmp ("24.07.-4500000",
			qof_date_print (qd, QOF_DATE_FORMAT_CE))), 
			"strftime:CE:seventh");
		do_test ((0 == safe_strcasecmp ("-4500000-07-24",
			qof_date_print (qd, QOF_DATE_FORMAT_ISO))), 
			"strftime:ISO:seventh");
		do_test ((0 == safe_strcasecmp ("-4500000-07-24T06:34:26Z",
			qof_date_print (qd, QOF_DATE_FORMAT_UTC))), 
			"strftime:UTC:seventh");
		do_test ((0 == safe_strcasecmp (NULL,
			qof_date_print (qd, QOF_DATE_FORMAT_LOCALE))), 
			"strftime:LOCALE:seventh:outofrange");
		do_test ((0 == safe_strcasecmp (NULL,
			qof_date_print (qd, QOF_DATE_FORMAT_CUSTOM))), 
			"strftime:CUSTOM:seventh:outofrange");
		qof_date_free (qd);
	}
}

static void
run_qoftime_test (void)
{
	QofTime *time, *cmp, *diff;

	time = qof_time_new ();
	do_test ((time != NULL), "failed to initialise QofTime.");
	/* basic tests */
	{
		qof_time_set_secs (time, 796179600);
		do_test ((qof_time_get_secs (time) > 0), "failed to set secs");
		do_test ((time != NULL), "error found");
		qof_time_set_secs (time, 0);
		qof_time_set_nanosecs (time, 2041020040);
		do_test ((qof_time_get_secs (time) > 0), "failed to normalise.");
		do_test ((time != NULL), "error found");
		do_test ((qof_time_get_nanosecs (time) > 0),
				 "failed to set nanosecs");
	}
	/* calculation and comparison tests */
	{
		qof_time_add_secs (time, -1143943200);
		do_test ((qof_time_get_secs (time) < 0), "failed to subtract.");
		do_test ((time != NULL), "error found");
		cmp = qof_time_new ();
		qof_time_set_secs (cmp, 0);
		do_test ((qof_time_equal (time, cmp) == FALSE), "test equal failed.");
		do_test ((qof_time_cmp (time, cmp) == -1), "compare cmp test");
		diff = qof_time_new ();
		qof_time_set_secs (diff, qof_time_get_secs (cmp));
		do_test ((qof_time_cmp (diff, cmp) == 0), "compare diff test");
		qof_time_free (diff);
		diff = qof_time_diff (time, cmp);
		do_test ((qof_time_get_secs (diff) < 0), "diff of negative value");
		qof_time_free (diff);
		diff = qof_time_diff (cmp, time);
		do_test ((qof_time_get_secs (diff) > 0), "diff of negative value");
		time = qof_time_abs (time);
		do_test ((qof_time_get_nanosecs (time) != 0), "abs failed");
		qof_time_set_secs (cmp, qof_time_get_secs (time));
		qof_time_set_nanosecs (cmp, qof_time_get_nanosecs (time));
		do_test ((qof_time_equal (cmp, time) == TRUE),
				 "equality test failed");
		qof_time_free (cmp);
		qof_time_free (time);
	}
	/* gdate basic tests */
	{
		GDate *date, *test;
		GTimeVal gtv;

		date = g_date_new_dmy (15, 5, 2006);
		time = qof_time_from_gdate (date);
		test = qof_time_to_gdate (time);
		do_test ((g_date_get_day (test) == 15), "gdate day fail");
		do_test ((g_date_get_month (test) == 5), "gdate month fail");
		do_test ((g_date_get_year (test) == 2006), "gdate year fail");
		do_test ((qof_time_to_gtimeval (time, &gtv)), "gtimeval fail");
		do_test ((qof_time_add_secs (time, 26451) != NULL),
				 "add seconds failed");
		g_date_free (date);
		g_date_free (test);
		qof_time_free (time);
	}
	/* gdate comparison tests */
	{
		GDate *date, *cmp;
		gint64 time_secs, diff;
		glong time_nano;

		date = g_date_new_dmy (15, 5, 2006);
		time = qof_time_from_gdate (date);
		cmp = qof_time_to_gdate (time);
		do_test ((g_date_compare (date, cmp) == 0),
				 "convert to and from gdate failed");
		g_date_free (cmp);
		time_secs = qof_time_get_secs (time);
		time_nano = qof_time_get_nanosecs (time);
		qof_time_add_secs (time, 7252);
		qof_time_set_nanosecs (time, 123456);
		do_test ((qof_time_set_day_start (time)), "set_day_start failed");
		do_test ((qof_time_get_secs (time) == time_secs),
				 "start of day incorrect");
		do_test ((qof_time_get_nanosecs (time) == 0),
				 "set nano at day start incorrect");
		do_test ((qof_time_set_day_middle (time) == TRUE),
				 "set_day_middle failed");
		diff = qof_time_get_secs (time) - time_secs;
		do_test (diff == (SECS_PER_DAY / 2), "middle of day incorrect");
		/* convert middle of day back to date */
		cmp = qof_time_to_gdate (time);
		do_test ((g_date_compare (date, cmp) == 0),
				 "middle of day not the same as original day");
		g_date_free (cmp);

		do_test ((qof_time_set_day_end (time)), "set_day_end failed");
		do_test ((qof_time_get_secs (time) - time_secs)
				 == (SECS_PER_DAY - 1), "end of day incorrect");

		/* convert end of day back to date */
		cmp = qof_time_to_gdate (time);

		do_test ((g_date_compare (date, cmp) == 0),
				 "end of day not the same as original day");
		qof_time_free (time);
		g_date_free (cmp);
		g_date_free (date);
	}
	/* QofTime today tests */
	{
		GTimeVal *current;
		gint64 time_secs;
		QofTime *diff;

		current = qof_time_get_current_start ();
		diff = qof_time_new ();
		qof_time_from_gtimeval (diff, current);
		time_secs = qof_time_get_secs (diff);
		time = qof_time_get_today_start ();
		do_test ((qof_time_get_secs (time) == time_secs),
				 "start of day incorrect");
		do_test ((qof_time_get_nanosecs (time) == 0),
				 "today start nanosecs non zero");
		qof_time_free (time);
		time = qof_time_get_today_end ();
		time_secs += SECS_PER_DAY - 1;
		do_test ((qof_time_get_secs (time) == time_secs),
				 "start of today incorrect");
		do_test ((qof_time_get_nanosecs (time) == 0),
				 "today start nanosecs non zero");
		qof_time_free (time);
	}
	/* last mday test */
	{
		GDate *date;
		guint8 mday;

		date = g_date_new_dmy (15, 5, 2006);
		time = qof_time_from_gdate (date);
		mday = qof_time_last_mday (time);
		do_test ((mday == 31), " wrong last day of May");
	}
	/* time to dmy test */
	{
		gboolean success;
		guint8 day, month;
		guint16 year;
		GDate *date;
		QofTime *reverse;

		date = g_date_new_dmy (15, 5, 2006);
		time = qof_time_from_gdate (date);
		success = qof_time_to_dmy (time, &day, &month, &year);
		do_test ((success == TRUE), "time to dmy failed");
		do_test ((day == 15), "wrong day calculated");
		do_test ((month == 5), "wrong month calculated");
		do_test ((year == 2006), "wrong year calculated");
		reverse = qof_time_dmy_to_time (day, month, year);
		do_test ((qof_time_cmp (time, reverse) == 0), "dmy to time failed");
		g_date_free (date);
		qof_time_free (time);
		qof_time_free (reverse);
	}
}

int
main (int argc, char **argv)
{
	qof_init ();
	test_date_init ();
	run_qoftime_test ();
	run_qofdate_test ();
	run_print_scan_tests ();
	g_list_foreach (test_data, check_date_cycles, NULL);
	print_test_results ();
	test_date_close ();
	qof_close ();
	exit (get_rv ());
}
