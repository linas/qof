/***************************************************************************
 *            qofstrptime.c
 *
 *  Wed May 31 09:34:13 2006
 *  Copyright (C) 2002, 2004, 2005, 2006 
 *  Free Software Foundation, Inc.
 *  This file is modified from the GNU C Library.
 ****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
/* 
Modified version of strptime from GNU C Library.

1. Removed preprocessor directives that are always true or always 
   false within QOF
2. Extended variables to full 64bit ranges, even on 32bit platforms.
3. Replaced time_t with QofTime to prevent overflow in 2038.
4. Replaced struct tm with QofDate to prevent overflow.
5. Implement an error handler to provide more information.
Neil Williams <linux@codehelp.co.uk>
*/

#include "config.h"
#include <ctype.h>
#include <string.h>
#include <glib.h>
#include "qof.h"
#include "qofdate-p.h"

static QofLogModule log_module = QOF_MOD_DATE;

AS_STRING_FUNC (QofDateError , ENUM_ERR_LIST)

#define match_char(ch1, ch2) if (ch1 != ch2) return NULL
# define match_string(cs1, s2) \
  (strncasecmp ((cs1), (s2), strlen (cs1)) ? 0 : ((s2) += strlen (cs1), 1))
/* We intentionally do not use isdigit() for testing because this will
   lead to problems with the wide character version.  */
#define get_number(from, to, n) 						\
	do {												\
	gint __n = n;										\
	val = 0;											\
	while (*rp == ' ')									\
		++rp;											\
	if (*rp < '0' || *rp > '9')							\
	{													\
		*error = ERR_OUT_OF_RANGE; 						\
		PERR (" error=%s", QofDateErrorasString (*error)); \
		return NULL;									\
	}  													\
	do {												\
		val *= 10;										\
		val += *rp++ - '0';								\
	}													\
	while (--__n > 0 && val * 10 <= to && *rp >= '0' && *rp <= '9');	\
	if (val < from || val > to)							\
	{ 													\
		*error = ERR_INVALID_DELIMITER; 				\
		PERR (" error=%s", QofDateErrorasString (*error)); \
		return NULL;									\
	} 													\
	} while (0)

/* If we don't have the alternate representation.  */
# define get_alt_number(from, to, n) \
	get_number(from, to, n)

#define recursive(new_fmt) \
  (*(new_fmt) != '\0' && (rp = strptime_internal (rp, (new_fmt), qd, error)) != NULL)

static gchar const weekday_name[][10] = {
	"Sunday", "Monday", "Tuesday", "Wednesday",
	"Thursday", "Friday", "Saturday"
};
static gchar const ab_weekday_name[][4] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
static gchar const month_name[][10] = {
	"January", "February", "March", "April", "May", "June",
	"July", "August", "September", "October", "November", "December"
};
static gchar const ab_month_name[][4] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

# define HERE_D_T_FMT "%a %b %e %H:%M:%S %Y"
# define HERE_D_FMT "%m/%d/%y"
# define HERE_AM_STR "AM"
# define HERE_PM_STR "PM"
# define HERE_T_FMT_AMPM "%I:%M:%S %p"
# define HERE_T_FMT "%H:%M:%S"
#define raw 1;

/* retained for a few areas where qd_mon and qd_mday are unknown.
*/
static const gushort yeardays[2][13] = {
	{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
	{0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
};

/* Compute the day of the week.  */
void
set_day_of_the_week (QofDate * qd)
{
	gint64 days;
	/* We know that January 1st 1970 was a Thursday (= 4). */
	days = days_between (1970, qd->qd_year);
	/* qd_wday is always positive. */
	if (days < 0)
		days *= -1;
	days--;
	days += qof_date_get_yday (qd->qd_mday, 
		qd->qd_mon, qd->qd_year) + 4;
	qd->qd_wday = ((days % 7) + 7) % 7;
}

gchar *
strptime_internal (const gchar * rp, const gchar * fmt, 
	QofDate * qd, QofDateError * error)
{
	const gchar *rp_backup;
	gint64 val, century, want_century;
	gint want_era, have_wday, want_xday, have_yday;
	gint have_mon, have_mday, have_uweek, have_wweek;
	gint week_no, have_I, is_pm, cnt, decided, era_cnt;
	struct era_entry *era;

	have_I = is_pm = 0;
	century = -1;
	decided = raw;
	era_cnt = -1;
	want_century = 0;
	want_era = 0;
	era = NULL;
	week_no = 0;
	*error = ERR_NO_ERROR;

	have_wday = want_xday = have_yday = have_mon = 0;
	have_mday = have_uweek = have_wweek = 0;

	while (*fmt != '\0')
	{
		/* A white space in the format string matches 0 more 
		or white space in the input string.  */
		if (isspace (*fmt))
		{
			while (isspace (*rp))
				++rp;
			++fmt;
			continue;
		}

		/* Any character but `%' must be matched by the 
		same character in the iput string. */
		if (*fmt != '%')
		{
			match_char (*fmt++, *rp++);
			continue;
		}

		++fmt;
		/* We need this for handling the `E' modifier.  */
	  start_over:

		/* Make back up of current processing pointer.  */
		rp_backup = rp;

		switch (*fmt++)
		{
		case '%':
			/* Match the `%' character itself.  */
			match_char ('%', *rp++);
			break;
		case 'a':
		case 'A':
			/* Match day of week.  */
			for (cnt = 0; cnt < 7; ++cnt)
			{
			  if (match_string (weekday_name[cnt], rp)
		      || match_string (ab_weekday_name[cnt], rp))
			  break;
			}
			if (cnt == 7)
			{
				/* Does not match a weekday name.  */
				*error = ERR_WEEKDAY_NAME;
				PERR (" error=%s", QofDateErrorasString (*error));
				return NULL;
			}
			qd->qd_wday = cnt;
			have_wday = 1;
			break;
		case 'b':
		case 'B':
		case 'h':
			/* Match month name.  */
			for (cnt = 0; cnt < 12; ++cnt)
			{
				if (match_string (month_name[cnt], rp)
					|| match_string (ab_month_name[cnt], rp))
				{
					decided = raw;
					break;
				}
			}
			if (cnt == 12)
			{
				/* Does not match a month name.  */
				*error = ERR_MONTH_NAME;
				PERR (" error=%s", QofDateErrorasString (*error));
				return NULL;
			}
			qd->qd_mon = cnt;
			want_xday = 1;
			break;
		case 'c':
			/* Match locale's date and time format.  */
			if (!recursive (HERE_D_T_FMT))
			{
				*error = ERR_LOCALE_DATE_TIME;
				PERR (" error=%s", QofDateErrorasString (*error));
				return NULL;
			}
			want_xday = 1;
			break;
		case 'C':
			/* Match century number.  */
			get_number (0, 99, 2);
			century = val;
			want_xday = 1;
			break;
		case 'd':
		case 'e':
			/* Match day of month.  */
			get_number (1, 31, 2);
			qd->qd_mday = val;
			have_mday = 1;
			want_xday = 1;
			break;
		case 'F':
			if (!recursive ("%Y-%m-%d"))
				return NULL;
			want_xday = 1;
			break;
		case 'x':
			/* Fall through.  */
		case 'D':
			/* Match standard day format.  */
			if (!recursive (HERE_D_FMT))
			{
				*error = ERR_STANDARD_DAY;
				PERR (" error=%s", QofDateErrorasString (*error));
				return NULL;
			}
			want_xday = 1;
			break;
		case 'k':
		case 'H':
			/* Match hour in 24-hour clock. */
			get_number (0, 23, 2);
			qd->qd_hour = val;
			have_I = 0;
			break;
		case 'l':
			/* Match hour in 12-hour clock. GNU extension. */
		case 'I':
			/* Match hour in 12-hour clock. */
			get_number (1, 12, 2);
			qd->qd_hour = val % 12;
			have_I = 1;
			break;
		case 'j':
			/* Match day number of year.  */
			get_number (1, 366, 3);
			qd->qd_yday = val - 1;
			have_yday = 1;
			break;
		case 'm':
			/* Match number of month.  */
			get_number (1, 12, 2);
			qd->qd_mon = val;
			have_mon = 1;
			want_xday = 1;
			break;
		case 'M':
			/* Match minute.  */
			get_number (0, 59, 2);
			qd->qd_min = val;
			break;
		case 'N':
		{
			/* match nanoseconds */
			gint n;
			n = val = 0;
			while (n < 9 && *rp >= '0' && *rp <= '9')
			{
				val = val * 10 + *rp++ - '0';
				++n;
			}
			qd->qd_nanosecs = val;
			break;
		}
		case 'n':
		case 't':
			/* Match any white space.  */
			while (isspace (*rp))
				++rp;
			break;
		case 'p':
			/* Match locale's equivalent of AM/PM.  */
			if (!match_string (HERE_AM_STR, rp))
			{
				if (match_string (HERE_PM_STR, rp))
					is_pm = 1;
				else
				{
					*error = ERR_LOCALE_AMPM;
					PERR (" error=%s", QofDateErrorasString (*error));
					return NULL;
				}
			}
			break;
		case 'r':
			if (!recursive (HERE_T_FMT_AMPM))
			{
				*error = ERR_TIME_AMPM;
				PERR (" error=%s", QofDateErrorasString (*error));
				return NULL;
			}
			break;
		case 'R':
			if (!recursive ("%H:%M"))
			{
				*error = ERR_RECURSIVE_R;
				PERR (" error=%s", QofDateErrorasString (*error));
				return NULL;
			}
			break;
		case 's':
			{
				/* The number of seconds may be very high so we 
				cannot use the `get_number' macro.  Instead read 
				the number character for character and construct 
				the result while doing this. */
				QofTimeSecs secs = 0;
				if (*rp < '0' || *rp > '9')
					/* We need at least one digit.  */
				{
					*error = ERR_SECS_NO_DIGITS;
					PERR (" error=%s", QofDateErrorasString (*error));
					return NULL;
				}
				do
				{
					secs *= 10;
					secs += *rp++ - '0';
				}
				while (*rp >= '0' && *rp <= '9');
				/** \todo implement a test for %s */
				qd->qd_sec = secs;
				if (!qof_date_valid (qd))
					return NULL;
			}
			break;
		case 'S':
			get_number (0, 61, 2);
			qd->qd_sec = val;
			break;
		case 'X':
		/* Fall through.  */
		case 'T':
			if (!recursive (HERE_T_FMT))
			{
				*error = ERR_RECURSIVE_T;
				PERR (" error=%s", QofDateErrorasString (*error));
				return NULL;
			}
			break;
		case 'u':
			get_number (1, 7, 1);
			qd->qd_wday = val % 7;
			have_wday = 1;
			break;
		case 'g':
			get_number (0, 99, 2);
			/* XXX This cannot determine any field in TM.  */
			break;
		case 'G':
			if (*rp < '0' || *rp > '9')
			{
				*error = ERR_G_INCOMPLETE;
				PERR (" error=%s", QofDateErrorasString (*error));
				return NULL;
			}
			/* XXX Ignore the number since we would need 
			some more information to compute a real date. */
			do
				++rp;
			while (*rp >= '0' && *rp <= '9');
			break;
		case 'U':
			get_number (0, 53, 2);
			week_no = val;
			have_uweek = 1;
			break;
		case 'W':
			get_number (0, 53, 2);
			week_no = val;
			have_wweek = 1;
			break;
		case 'V':
			get_number (0, 53, 2);
			/* XXX This cannot determine any field without some
			information. */
			break;
		case 'w':
			/* Match number of weekday.  */
			get_number (0, 6, 1);
			qd->qd_wday = val;
			have_wday = 1;
			break;
		case 'y':
			/* Match year within century.  */
			get_number (0, 99, 2);
			/* The "Year 2000: The Millennium Rollover" paper suggests that
			values in the range 69-99 refer to the twentieth century.  */
			qd->qd_year = val >= 69 ? val + 2000 : val + 1900;
			/* Indicate that we want to use the century, if specified.  */
			want_century = 1;
			want_xday = 1;
			break;
		case 'Y':
			/* Match year including century number.  */
			get_number (0, 999999999, 9);
			qd->qd_year = val;
			want_century = 0;
			want_xday = 1;
			break;
		case 'Z':
			/* XXX How to handle this?  */
			PINFO (" Z format - todo?");
			break;
		case 'z':
			/* We recognize two formats: if two digits are given, these
			specify hours. If fours digits are used, minutes are
			also specified. */
			{
				gboolean neg;
				gint n;
				val = 0;
				while (*rp == ' ')
					++rp;
				if (*rp != '+' && *rp != '-')
				{
					*error = ERR_INVALID_Z;
					PERR (" error=%s", QofDateErrorasString (*error));
					return NULL;
				}
				neg = *rp++ == '-';
				n = 0;
				while (n < 4 && *rp >= '0' && *rp <= '9')
				{
					val = val * 10 + *rp++ - '0';
					++n;
				}
				if (n == 2)
					val *= 100;
				else if (n != 4)
				{
					/* Only two or four digits recognized. */
					*error = ERR_YEAR_DIGITS;
					PERR (" error=%s", QofDateErrorasString (*error));
					return NULL;
				}
				else
				{
					/* We have to convert the minutes into decimal.  */
					if (val % 100 >= 60)
					{
						*error = ERR_MIN_TO_DECIMAL;
						PERR (" error=%s", QofDateErrorasString (*error));
						return NULL;
					}
					val = (val / 100) * 100 + ((val % 100) * 50) / 30;
				}
				if (val > 1200)
				{
					*error = ERR_GMTOFF;
					PERR (" error=%s", QofDateErrorasString (*error));
					return NULL;
				}
				qd->qd_gmt_off = (val * 3600) / 100;
				if (neg)
					qd->qd_gmt_off = -qd->qd_gmt_off;
			}
			break;
		case 'E':
			/* We have no information about the era format. 
			Just use the normal format. */
			if (*fmt != 'c' && *fmt != 'C' && *fmt != 'y' && *fmt != 'Y'
				&& *fmt != 'x' && *fmt != 'X')
			{
				/* This is an illegal format.  */
				*error = ERR_INVALID_FORMAT;
				PERR (" error=%s", QofDateErrorasString (*error));
				return NULL;
			}

			goto start_over;
		case 'O':
			switch (*fmt++)
			{
			case 'd':
			case 'e':
				/* Match day of month using alternate numeric symbols.  */
				get_alt_number (1, 31, 2);
				qd->qd_mday = val;
				have_mday = 1;
				want_xday = 1;
				break;
			case 'H':
				/* Match hour in 24-hour clock using alternate 
				numeric symbols. */
				get_alt_number (0, 23, 2);
				qd->qd_hour = val;
				have_I = 0;
				break;
			case 'I':
				/* Match hour in 12-hour clock using alternate 
				numeric symbols. */
				get_alt_number (1, 12, 2);
				qd->qd_hour = val % 12;
				have_I = 1;
				break;
			case 'm':
				/* Match month using alternate numeric symbols. */
				get_alt_number (1, 12, 2);
				qd->qd_mon = val - 1;
				have_mon = 1;
				want_xday = 1;
				break;
			case 'M':
				/* Match minutes using alternate numeric symbols.  */
				get_alt_number (0, 59, 2);
				qd->qd_min = val;
				break;
			case 'S':
				/* Match seconds using alternate numeric symbols.  */
				get_alt_number (0, 61, 2);
				qd->qd_sec = val;
				break;
			case 'U':
				get_alt_number (0, 53, 2);
				week_no = val;
				have_uweek = 1;
				break;
			case 'W':
				get_alt_number (0, 53, 2);
				week_no = val;
				have_wweek = 1;
				break;
			case 'V':
				get_alt_number (0, 53, 2);
				/* XXX This cannot determine any field without
				further information.  */
				break;
			case 'w':
				/* Match number of weekday using alternate numeric symbols. */
				get_alt_number (0, 6, 1);
				qd->qd_wday = val;
				have_wday = 1;
				break;
			case 'y':
				/* Match year within century using alternate numeric symbols. */
				get_alt_number (0, 99, 2);
				qd->qd_year = val >= 69 ? val : val + 100;
				want_xday = 1;
				break;
			default:
				{
					*error = ERR_UNKNOWN_ERR;
					PERR (" error=%s (first default)", 
						QofDateErrorasString (*error));
					return NULL;
				}
			}
			break;
		default:
			{
				*error = ERR_UNKNOWN_ERR;
				PERR (" error=%s val=%s (second default)", 
					QofDateErrorasString (*error), rp);
				return NULL;
			}
		}
	}

	if (have_I && is_pm)
		qd->qd_hour += 12;

	if (century != -1)
	{
		if (want_century)
			/** \todo watch year base */
			qd->qd_year = qd->qd_year % 100 + (century - 19) * 100;
		else
			/* Only the century, but not the year.  */
			qd->qd_year = (century - 19) * 100;
	}

	if (era_cnt != -1)
	{
		if (era == NULL)
		{
			*error = ERR_INVALID_ERA;
			PERR (" error=%s", QofDateErrorasString (*error));
			return NULL;
		}
	}
	else if (want_era)
	{
	/* No era found but we have seen an E modifier.
	Rectify some values. */
	/** \todo oops! this could be bad! */
		if (want_century && century == -1 && qd->qd_year < 69)
			qd->qd_year += 100;
	}

	if (want_xday && !have_wday)
	{
		if (!(have_mon && have_mday) && have_yday)
		{
			/* We don't have qd_mon and/or qd_mday, compute them.  */
			gint t_mon = 0;
			gint leap = qof_date_isleap (qd->qd_year);
			while (yeardays[leap][t_mon] <=
				qd->qd_yday)
				t_mon++;
			if (!have_mon)
				qd->qd_mon = t_mon;
			if (!have_mday)
				qd->qd_mday = qd->qd_yday - 
					yeardays[leap][t_mon - 1] + 1;
		}
		set_day_of_the_week (qd);
	}

	if (want_xday && !have_yday)
		qd->qd_yday = qof_date_get_yday (qd->qd_mday,
			qd->qd_mon, qd->qd_year);

	if ((have_uweek || have_wweek) && have_wday)
	{
		gint save_wday = qd->qd_wday;
		gint save_mday = qd->qd_mday;
		gint save_mon = qd->qd_mon;
		gint w_offset = have_uweek ? 0 : 1;

		qd->qd_mday = 1;
		qd->qd_mon = 0;
		set_day_of_the_week (qd);
		if (have_mday)
			qd->qd_mday = save_mday;
		if (have_mon)
			qd->qd_mon = save_mon;

		if (!have_yday)
			qd->qd_yday = ((7 - (qd->qd_wday - w_offset)) % 7
				+ (week_no - 1) * 7 + save_wday - w_offset);

		if (!have_mday || !have_mon)
		{
			gint t_mon = 0;

			while (qof_date_get_yday (1, t_mon, qd->qd_year) <=
				qd->qd_yday)
				t_mon++;
			if (!have_mon)
				qd->qd_mon = t_mon - 1;
			if (!have_mday)
				qd->qd_mday = (qd->qd_yday -
					qof_date_get_yday (1, t_mon, qd->qd_year));
		}

		qd->qd_wday = save_wday;
	}

	return (gchar *) rp;
}
