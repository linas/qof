/***************************************************************************
 *            qofstrftime.c
 *
 *  Sun May 21 15:59:32 2006
 *  Copyright (C) 1991-1999, 2000, 2001, 2003, 2004, 2005, 2006 
 *    Free Software Foundation, Inc. 
 *
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
Modified version of strftime from Debian coreutils package.

(note that the GNU date command includes only strftime,
the QOF strptime code comes direct from the GNU glibc.)

1. Removed preprocessor directives that are always true or always false within QOF
2. Extended variables to full 64bit ranges, even on 32bit platforms.
3. Replaced time_t with qt_time to prevent overflow in 2038.
4. Replaced struct tm with QofDate to prevent overflow.
Neil Williams <linux@codehelp.co.uk>
*/

#include "config.h"
#include <stdio.h>
#include <ctype.h>
#include <sys/time.h>
#include <time.h>
#include <wchar.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "qof.h"
#include "qofdate-p.h"

static QofLogModule log_module = QOF_MOD_DATE;

#define memset_space(P, Len) (memset (P, ' ', Len), (P) += (Len))
#define memset_zero(P, Len) (memset (P, '0', Len), (P) += (Len))
/** \todo legacy value, remove */
#define FPRINTFTIME 0

/* Shift A right by B bits portably, by dividing A by 2**B and
truncating towards minus infinity.  A and B should be free of side
effects, and B should be in the range 0 <= B <= INT_BITS - 2, where
INT_BITS is the number of useful bits in an int.  GNU code can
assume that INT_BITS is at least 32.

ISO C99 says that A >> B is implementation-defined if A < 0.  Some
implementations (e.g., UNICOS 9.0 on a Cray Y-MP EL) don't shift
right in the usual way when A < 0, so SHR falls back on division if
ordinary A >> B doesn't seem to be the usual signed shift.  */
#define SHR(a, b)		\
	(-1 >> 1 == -1		\
	? (a) >> (b)		\
	: (a) / (1 << (b)) - ((a) % (1 << (b)) < 0))

/* Bound on length of the string representing an integer 
type or expression T. Subtract 1 for the sign bit if t is signed; 
log10 (2.0) < 146/485; add 1 for integer division truncation; 
add 1 more for a minus sign if needed.  */
#define INT_strlen _BOUND(t) \
	((sizeof (t) * CHAR_BIT - 1) * 146 / 485 + 2)

/* IMPORTANT: QofDate does not use 1900 or 1970 as a base, all 
 years in QofDate are true values. */
#define TM_YEAR_BASE 0

#define add(n, f) 										\
	do													\
	{													\
		gint _n = (n);									\
		gint _delta = width - _n;						\
		gint _incr = _n + (_delta > 0 ? _delta : 0);	\
		if ((size_t) _incr >= maxsize - i)				\
			return 0;									\
		if (p)											\
		{												\
			if (digits == 0 && _delta > 0)				\
			{											\
				if (pad == ('0'))						\
					memset_zero (p, _delta);			\
				else									\
					memset_space (p, _delta);			\
			}											\
			f;											\
			p += FPRINTFTIME ? 0 : _n;					\
		}												\
		i += _incr; 									\
	} while (0)

# define add1(C) add (1, *p = C)

# define cpy(n, s)									\
	add ((n),										\
	 if (to_lowcase)								\
	   memcpy_lowcase (p, (s), _n);					\
	 else if (to_uppcase)							\
	   memcpy_uppcase (p, (s), _n);					\
	 else											\
	   memcpy ((void *) p, (void const *) (s), _n))

#define TOUPPER(Ch, L) (islower (Ch) ? toupper (Ch) : (Ch))
#define TOLOWER(Ch, L) (isupper (Ch) ? tolower (Ch) : (Ch))

/* We don't use `isdigit' here since the locale dependent
interpretation is not what we want here. We only need to accept
the arabic digits in the ASCII range.  One day there is perhaps a
more reliable way to accept other sets of digits.  */
#define ISDIGIT(Ch) ((guint) (Ch) - ('0') <= 9)
/* The number of days from the first day of the first ISO week of this
year to the year day YDAY with week day WDAY.  ISO weeks start on
Monday; the first ISO week has the year's first Thursday.  YDAY may
be as small as YDAY_MINIMUM.  */
#define ISO_WEEK_START_WDAY 1	/* Monday */
#define ISO_WEEK1_WDAY 4		/* Thursday */
#define YDAY_MINIMUM (-366)

static const mbstate_t mbstate_zero;
const gchar *format_end = NULL;

static gchar *
memcpy_lowcase (gchar * dest, const gchar * src, size_t len)
{
	while (len-- > 0)
		dest[len] = TOLOWER ((guchar) src[len], loc);
	return dest;
}

static gchar *
memcpy_uppcase (gchar * dest, const gchar * src, size_t len)
{
	while (len-- > 0)
		dest[len] = TOUPPER ((guchar) src[len], loc);
	return dest;
}

static gint
iso_week_days (gint yday, gint wday)
{
	/* Add enough to the first operand of % to make it nonnegative.  */
	gint big_enough_multiple_of_7 = (-YDAY_MINIMUM / 7 + 2) * 7;
	return (yday
		- (yday - wday + ISO_WEEK1_WDAY + big_enough_multiple_of_7) % 7
		+ ISO_WEEK1_WDAY - ISO_WEEK_START_WDAY);
}

size_t
strftime_case (gboolean upcase, gchar * s,
	size_t maxsize, const gchar *format, const QofDate *qd, 
	gint ut, glong ns)
{
	const gchar *zone;
	gint hour12 = qd->qd_hour;
	size_t i = 0;
	gchar *p = s;
	const gchar *f;
	QofDate copy = *qd;
	qd = &copy;
	zone = (const gchar *) qd->qd_zone;
	if (ut)
	{
		if (!(zone && *zone))
		{
			setenv ("TZ", "GMT", 1);
			zone = "GMT";
		}
	}
	else
	{
		/* POSIX.1 requires that local time zone information be 
		used as though strftime called tzset.  */
		tzset ();
	}

	if (hour12 > 12)
		hour12 -= 12;
	else if (hour12 == 0)
		hour12 = 12;

	for (f = format; *f != '\0'; ++f)
	{
		gint pad = 0;			/* Padding for number ('-', '_', or 0).  */
		gint modifier;			/* Field modifier ('E', 'O', or 0).  */
		gint digits = 0;		/* Max digits for numeric format.  */
		glong number_value;		/* Numeric value to be printed.  */
		guint u_number_value;	/* (unsigned int) number_value.  */
		gboolean negative_number;	/* The number is negative.  */
		gboolean always_output_a_sign;	/* +/- should always be output.  */
		gint tz_colon_mask;		/* Bitmask of where ':' should appear.  */
		const gchar *subfmt;
		gchar sign_char;
		gchar *bufp;
		gchar buf[MAX_DATE_BUFFER];
		gint width = -1;
		gboolean to_lowcase = FALSE;
		gboolean to_uppcase = upcase;
		size_t colons;
		gboolean change_case = FALSE;
		gint format_char;

		switch (*f)
		{
		case ('%'):
			break;

		case ('\b'):
		case ('\t'):
		case ('\n'):
		case ('\v'):
		case ('\f'):
		case ('\r'):
		case (' '):
		case ('!'):
		case ('"'):
		case ('#'):
		case ('&'):
		case ('\''):
		case ('('):
		case (')'):
		case ('*'):
		case ('+'):
		case (','):
		case ('-'):
		case ('.'):
		case ('/'):
		case ('0'):
		case ('1'):
		case ('2'):
		case ('3'):
		case ('4'):
		case ('5'):
		case ('6'):
		case ('7'):
		case ('8'):
		case ('9'):
		case (':'):
		case (';'):
		case ('<'):
		case ('='):
		case ('>'):
		case ('?'):
		case ('A'):
		case ('B'):
		case ('C'):
		case ('D'):
		case ('E'):
		case ('F'):
		case ('G'):
		case ('H'):
		case ('I'):
		case ('J'):
		case ('K'):
		case ('L'):
		case ('M'):
		case ('N'):
		case ('O'):
		case ('P'):
		case ('Q'):
		case ('R'):
		case ('S'):
		case ('T'):
		case ('U'):
		case ('V'):
		case ('W'):
		case ('X'):
		case ('Y'):
		case ('Z'):
		case ('['):
		case ('\\'):
		case (']'):
		case ('^'):
		case ('_'):
		case ('a'):
		case ('b'):
		case ('c'):
		case ('d'):
		case ('e'):
		case ('f'):
		case ('g'):
		case ('h'):
		case ('i'):
		case ('j'):
		case ('k'):
		case ('l'):
		case ('m'):
		case ('n'):
		case ('o'):
		case ('p'):
		case ('q'):
		case ('r'):
		case ('s'):
		case ('t'):
		case ('u'):
		case ('v'):
		case ('w'):
		case ('x'):
		case ('y'):
		case ('z'):
		case ('{'):
		case ('|'):
		case ('}'):
		case ('~'):
			/* The C Standard requires these 98 characters (plus '%') 
			to be in the basic execution character set.  None of 
			these characters can start a multibyte sequence, so they 
			need not be analyzed further.  */
			add1 (*f);
			continue;

		default:
			/* Copy this multibyte sequence until we reach its end, 
			find an error, or come back to the initial shift state.
			*/
			{
				mbstate_t mbstate = mbstate_zero;
				size_t len = 0;
				size_t fsize;

				if (!format_end)
					format_end = f + strlen (f) + 1;
				fsize = format_end - f;

				do
				{
					size_t bytes = mbrlen (f + len, fsize - len, &mbstate);

					if (bytes == 0)
						break;

					if (bytes == (size_t) - 2)
					{
						len += strlen (f + len);
						break;
					}

					if (bytes == (size_t) - 1)
					{
						len++;
						break;
					}

					len += bytes;
				}
				while (!mbsinit (&mbstate));

				cpy (len, f);
				f += len - 1;
				continue;
			}
		}

		/* Check for flags that can modify a format.  */
		while (1)
		{
			switch (*++f)
			{
				/* This influences the number formats.  */
			case ('_'):
			case ('-'):
			case ('0'):
				pad = *f;
				continue;

				/* This changes textual output.  */
			case ('^'):
				to_uppcase = TRUE;
				continue;
			case ('#'):
				change_case = TRUE;
				continue;

			default:
				break;
			}
			break;
		}

		/* As a GNU extension we allow to specify the field width.  */
		if (ISDIGIT (*f))
		{
			width = 0;
			do
			{
				if (width > INT_MAX / 10
					|| (width == INT_MAX / 10
						&& *f - ('0') > INT_MAX % 10))
					/* Avoid overflow.  */
					width = INT_MAX;
				else
				{
					width *= 10;
					width += *f - ('0');
				}
				++f;
			}
			while (ISDIGIT (*f));
		}

		/* Check for modifiers.  */
		switch (*f)
		{
		case ('E'):
		case ('O'):
			modifier = *f++;
			break;

		default:
			modifier = 0;
			break;
		}

		/* Now do the specified format.  */
		format_char = *f;
		switch (format_char)
		{
#define DO_NUMBER(d, v) \
	  digits = d;	  \
	  number_value = v; goto do_number
#define DO_SIGNED_NUMBER(d, negative, v) \
	  digits = d;	    \
	  negative_number = negative;  \
	  u_number_value = v; goto do_signed_number

/* The mask is not what you might think.
When the ordinal i'th bit is set, insert a colon
before the i'th digit of the time zone representation.  */
#define DO_TZ_OFFSET(d, negative, mask, v)	\
	digits = d;								\
	negative_number = negative;				\
	tz_colon_mask = mask;					\
	u_number_value = v; goto do_tz_offset
#define DO_NUMBER_SPACEPAD(d, v)			\
digits = d;									\
	number_value = v; goto do_number_spacepad

		case ('%'):
			if (modifier != 0)
				goto bad_format;
			add1 (*f);
			break;

		case ('a'):
			if (modifier != 0)
				goto bad_format;
			if (change_case)
			{
				to_uppcase = TRUE;
				to_lowcase = FALSE;
			}
			goto underlying_strftime;

		case 'A':
			if (modifier != 0)
				goto bad_format;
			if (change_case)
			{
				to_uppcase = TRUE;
				to_lowcase = FALSE;
			}
			goto underlying_strftime;

		case ('b'):
		case ('h'):
			if (change_case)
			{
				to_uppcase = TRUE;
				to_lowcase = FALSE;
			}
			if (modifier != 0)
				goto bad_format;
			goto underlying_strftime;

		case ('B'):
			if (modifier != 0)
				goto bad_format;
			if (change_case)
			{
				to_uppcase = TRUE;
				to_lowcase = FALSE;
			}
			goto underlying_strftime;

		case ('c'):
			if (modifier == ('O'))
				goto bad_format;
			goto underlying_strftime;

			subformat:
			{
				size_t len = strftime_case (to_uppcase,
					NULL, ((size_t) - 1),
					subfmt,	qd, ut, ns);
				add (len, strftime_case (to_uppcase, p,
						(maxsize - i), subfmt, qd, ut, ns));
			}
			break;

		  underlying_strftime:
			{
				/* try to handle locale-specific formats */
				gchar ufmt[5];
				gchar *u = ufmt;
				gchar ubuf[1024];	/* enough for any single format in practice */
				size_t len;
				/* Make sure we're calling the actual underlying strftime.
				In some cases, config.h contains something like
				"#define strftime rpl_strftime".  */
# ifdef strftime
#  undef strftime
				size_t strftime ();
#endif

				/* The space helps distinguish strftime failure from 
				empty output.  */
				*u++ = ' ';
				*u++ = '%';
				if (modifier != 0)
					*u++ = modifier;
				*u++ = format_char;
				*u = '\0';
				{
					glong nanosecs;
					struct tm bad;
					if(!qof_date_to_struct_tm ((QofDate*)qd, &bad, &nanosecs))
					{
						PERR (" locale format out of range.");
						break;
					}
					len = strftime (ubuf, sizeof ubuf, ufmt, &bad);
				}
				if (len != 0)
					cpy (len - 1, ubuf + 1);
			}
			break;

		case ('C'):
			if (modifier == ('O'))
				goto bad_format;
			if (modifier == ('E'))
			{
				goto underlying_strftime;
			}

			{
				/* convert to use QofDate->qd_year which is 64bit */
				gint century = qd->qd_year / 100 + TM_YEAR_BASE / 100;
				century -= qd->qd_year % 100 < 0 && 0 < century;
				DO_SIGNED_NUMBER (2, 
					qd->qd_year < -TM_YEAR_BASE, century);
			}

		case ('x'):
			if (modifier == ('O'))
				goto bad_format;
			goto underlying_strftime;
		case ('D'):
			if (modifier != 0)
				goto bad_format;
			subfmt = ("%m/%d/%y");
			goto subformat;

		case ('d'):
			if (modifier == ('E'))
				goto bad_format;

			DO_NUMBER (2, qd->qd_mday);

		case ('e'):
			if (modifier == ('E'))
				goto bad_format;

			DO_NUMBER_SPACEPAD (2, qd->qd_mday);

			/* All numeric formats set DIGITS and NUMBER_VALUE (or U_NUMBER_VALUE)
			and then jump to one of these labels.  */
			do_tz_offset:
			always_output_a_sign = TRUE;
			goto do_number_body;

			do_number_spacepad:
			/* Force `_' flag unless overridden by `0' or `-' flag.  */
			if (pad != ('0') && pad != ('-'))
				pad = ('_');

			do_number:
			/* Format NUMBER_VALUE according to the MODIFIER flag.  */
			negative_number = number_value < 0;
			u_number_value = number_value;

			do_signed_number:
			always_output_a_sign = FALSE;
			tz_colon_mask = 0;

			do_number_body:
			/* Format U_NUMBER_VALUE according to the MODIFIER flag.
			NEGATIVE_NUMBER is nonzero if the original number was
			negative; in this case it was converted directly to
			unsigned int (i.e., modulo (UINT_MAX + 1)) without
			negating it.  */
			if (modifier == ('O') && !negative_number)
			{
				goto underlying_strftime;
			}

			bufp = buf + sizeof (buf) / sizeof (buf[0]);

			if (negative_number)
				u_number_value = -u_number_value;

			do
			{
				if (tz_colon_mask & 1)
					*--bufp = ':';
				tz_colon_mask >>= 1;
				*--bufp = u_number_value % 10 + ('0');
				u_number_value /= 10;
			}
			while (u_number_value != 0 || tz_colon_mask != 0);

		  do_number_sign_and_padding:
			if (digits < width)
				digits = width;

			sign_char = (negative_number ? ('-')
				: always_output_a_sign ? ('+') : 0);

			if (pad == ('-'))
			{
				if (sign_char)
					add1 (sign_char);
			}
			else
			{
				gint padding =
					digits - (buf + (sizeof (buf) / sizeof (buf[0])) -
					bufp) - !!sign_char;

				if (padding > 0)
				{
					if (pad == ('_'))
					{
						if ((size_t) padding >= maxsize - i)
							return 0;

						if (p)
							memset_space (p, padding);
						i += padding;
						width = width > padding ? width - padding : 0;
						if (sign_char)
							add1 (sign_char);
					}
					else
					{
						if ((size_t) digits >= maxsize - i)
							return 0;

						if (sign_char)
							add1 (sign_char);

						if (p)
							memset_zero (p, padding);
						i += padding;
						width = 0;
					}
				}
				else
				{
					if (sign_char)
						add1 (sign_char);
				}
			}

			cpy (buf + sizeof (buf) / sizeof (buf[0]) - bufp, bufp);
			break;

		case ('F'):
			if (modifier != 0)
				goto bad_format;
			subfmt = ("%Y-%m-%d");
			goto subformat;

		case ('H'):
			if (modifier == ('E'))
				goto bad_format;

			DO_NUMBER (2, qd->qd_hour);

		case ('I'):
			if (modifier == ('E'))
				goto bad_format;

			DO_NUMBER (2, hour12);

		case ('k'):			/* GNU extension.  */
			if (modifier == ('E'))
				goto bad_format;

			DO_NUMBER_SPACEPAD (2, qd->qd_hour);

		case ('l'):			/* GNU extension.  */
			if (modifier == ('E'))
				goto bad_format;

			DO_NUMBER_SPACEPAD (2, hour12);

		case ('j'):
			if (modifier == ('E'))
				goto bad_format;

			DO_SIGNED_NUMBER (3, qd->qd_yday < -1, qd->qd_yday + 1U);

		case ('M'):
			if (modifier == ('E'))
				goto bad_format;

			DO_NUMBER (2, qd->qd_min);

		case ('m'):
			if (modifier == ('E'))
				goto bad_format;

			DO_SIGNED_NUMBER (2, qd->qd_mon < -1, qd->qd_mon);

		case ('N'):			/* GNU extension.  */
			if (modifier == ('E'))
				goto bad_format;

			number_value = ns;
			if (width == -1)
				width = 9;
			else
			{
				/* Take an explicit width less than 9 as a precision.  */
				gint j;
				for (j = width; j < 9; j++)
					number_value /= 10;
			}

			DO_NUMBER (width, number_value);

		case ('n'):
			add1 (('\n'));
			break;

		case ('P'):
			to_lowcase = TRUE;
			format_char = ('p');

		case ('p'):
			if (change_case)
			{
				to_uppcase = FALSE;
				to_lowcase = TRUE;
			}
			goto underlying_strftime;

		case ('R'):
			subfmt = ("%H:%M");
			goto subformat;

		case ('r'):
			goto underlying_strftime;

		case ('S'):
			if (modifier == ('E'))
				goto bad_format;

			DO_NUMBER (2, qd->qd_sec);

		case ('s'):			/* GNU extension.
		number of seconds since the epoch.
		basically QofTimeSecs as a string.
		*/
			{
				glong nanosecs;
				QofTime *time;
				QofTimeSecs t;

				time = qof_date_to_qtime ((QofDate*)qd);
				t = qof_time_get_secs (time);
				nanosecs = qof_time_get_nanosecs (time);

				/* Generate string value for T using time_t arithmetic;
				   this works even if sizeof (long) < sizeof (time_t).  */

				bufp = buf + sizeof (buf) / sizeof (buf[0]);
				negative_number = t < 0;

				do
				{
					gint d = t % 10;
					t /= 10;
					*--bufp = (negative_number ? -d : d) + ('0');
				}
				while (t != 0);

				digits = 1;
				always_output_a_sign = FALSE;
				goto do_number_sign_and_padding;
			}

		case ('X'):
			if (modifier == ('O'))
				goto bad_format;
			goto underlying_strftime;
		case ('T'):
			subfmt = ("%H:%M:%S");
			goto subformat;

		case ('t'):
			add1 (('\t'));
			break;

		case ('u'):
			DO_NUMBER (1, (qd->qd_wday - 1 + 7) % 7 + 1);

		case ('U'):
			if (modifier == ('E'))
				goto bad_format;

			DO_NUMBER (2, (qd->qd_yday - qd->qd_wday + 7) / 7);

		case ('V'):
		case ('g'):
		case ('G'):
			if (modifier == ('E'))
				goto bad_format;
			{
				gint year_adjust = 0;
				gint days = iso_week_days (qd->qd_yday, qd->qd_wday);

				if (days < 0)
				{
					/* This ISO week belongs to the previous year.  */
					year_adjust = -1;
					days =
						iso_week_days (qd->qd_yday +
						(365 + qof_date_isleap (qd->qd_year - 1)), 
						qd->qd_wday);
				}
				else
				{
					gint d =
						iso_week_days (qd->qd_yday - (365 +
							qof_date_isleap (qd->qd_year)),
						qd->qd_wday);
					if (0 <= d)
					{
						/* This ISO week belongs to the next year.  */
						year_adjust = 1;
						days = d;
					}
				}

				switch (*f)
				{
				case ('g'):
					{
						/* use QofDate->qd_year */
						gint yy = (qd->qd_year % 100 + year_adjust) % 100;
						DO_NUMBER (2, (0 <= yy
								? yy : qd->qd_year <
								-TM_YEAR_BASE -
								year_adjust ? -yy : yy + 100));
					}

				case ('G'):
					/* use QofDate->qd_year */
					DO_SIGNED_NUMBER (4,
						qd->qd_year <
						-TM_YEAR_BASE - year_adjust,
						(qd->qd_year + (guint) TM_YEAR_BASE +
							year_adjust));

				default:
					DO_NUMBER (2, days / 7 + 1);
				}
			}

		case ('W'):
			if (modifier == ('E'))
				goto bad_format;

			DO_NUMBER (2,
				(qd->qd_yday - (qd->qd_wday - 1 + 7) % 7 + 7) / 7);

		case ('w'):
			if (modifier == ('E'))
				goto bad_format;

			DO_NUMBER (1, qd->qd_wday);

		case ('Y'):
			if (modifier == 'E')
			{
				goto underlying_strftime;
			}
			if (modifier == ('O'))
				goto bad_format;
			else
				/* use QofDate->qd_year */
				DO_SIGNED_NUMBER (4, qd->qd_year < -TM_YEAR_BASE,
					qd->qd_year + TM_YEAR_BASE);

		case ('y'):
			if (modifier == ('E'))
			{
				goto underlying_strftime;
			}

			{
				gint64 yy = qd->qd_year % 100;
				if (yy < 0)
					yy = qd->qd_year < -TM_YEAR_BASE ? -yy : yy + 100;
				DO_NUMBER (2, yy);
			}

		case ('Z'):
			if (change_case)
			{
				to_uppcase = FALSE;
				to_lowcase = TRUE;
			}

			/* The tzset() call might have changed the value.  */
			if (!(zone && *zone) && qd->qd_is_dst >= 0)
				zone = tzname[qd->qd_is_dst != 0];
			if (!zone)
				zone = "";

			cpy (strlen (zone), zone);
			break;

		case (':'):
			/* :, ::, and ::: are valid only just before 'z'.
			   :::: etc. are rejected later.  */
			for (colons = 1; f[colons] == (':'); colons++)
				continue;
			if (f[colons] != ('z'))
				goto bad_format;
			f += colons;
			goto do_z_conversion;

		case ('z'):
			colons = 0;

		  do_z_conversion:
			if (qd->qd_is_dst < 0)
				break;

			{
				gint diff;
				gint hour_diff;
				gint min_diff;
				gint sec_diff;
				diff = qd->qd_gmt_off;
				hour_diff = diff / 60 / 60;
				min_diff = diff / 60 % 60;
				sec_diff = diff % 60;

				switch (colons)
				{
				case 0:		/* +hhmm */
					DO_TZ_OFFSET (5, diff < 0, 0,
						hour_diff * 100 + min_diff);

				case 1:
				  tz_hh_mm:	/* +hh:mm */
					DO_TZ_OFFSET (6, diff < 0, 04,
						hour_diff * 100 + min_diff);

				case 2:
				  tz_hh_mm_ss:	/* +hh:mm:ss */
					DO_TZ_OFFSET (9, diff < 0, 024,
						hour_diff * 10000 + min_diff * 100 + sec_diff);

				case 3:		/* +hh if possible, else +hh:mm, else +hh:mm:ss */
					if (sec_diff != 0)
						goto tz_hh_mm_ss;
					if (min_diff != 0)
						goto tz_hh_mm;
					DO_TZ_OFFSET (3, diff < 0, 0, hour_diff);

				default:
					goto bad_format;
				}
			}

		case ('\0'):			/* GNU extension: % at end of format.  */
			--f;
			/* Fall through.  */
		default:
			/* Unknown format; output the format, including the '%',
			   since this is most likely the right thing to do if a
			   multibyte string has been misparsed.  */
		  bad_format:
			{
				gint flen;
				for (flen = 1; f[1 - flen] != ('%'); flen++)
					continue;
				cpy (flen, &f[1 - flen]);
			}
			break;
		}
	}
	return i;
}
