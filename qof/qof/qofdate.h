/********************************************************************
 *       qofdate.h - QofDate, 64bit UTC date handling.
 *       Rewritten from scratch for QOF 0.7.0
 *
 *  Fri May  5 15:05:24 2006
 *  Copyright (C) 2006 Free Software Foundation, Inc.
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
/**	@addtogroup QOFTIME
 @{
*/
/** @addtogroup Date Date: 64bit UTC date handling.

 All dates in QOF use Universal Coordinated Time, (UTC),
 wrapped to allow representation of dates before the epoch.

 localtime is available via GDate but there are limitations.

-# The GDate data structure represents a day between
   January 1, Year 1, and sometime a few thousand years in the
   future, not the full range of QofTime.
-# Right now GDate will go to the year 65535 or so, but
   g_date_set_parse() only parses up to the year 8000 or so -
   just count on "a few thousand".
-# g_date_strftime only deals with date related formats,
   results of using time formats is undefined.

 A QofDateEntry is one a collection of formats for handling 
 dates, including common defaults. New entries need to be 
 registered using qof_date_add_format before being used to print 
 or scan strings containing dates.

 \todo Add support for customised handlers for added formats.

 A QofDate is theoretically able to go forward to the year 
 292,471,206,707 AD and back to the year 292,471,206,708 BC. 
 Whether such dates actually exist is outside the scope of 
 this documentation.

 \note Time moves at a constant rate, dates do not. Dates are
 inherently tied to the rotation of the Earth and that rotation
 is slowing down, causing the need for leapseconds. The incidence
 of leapseconds cannot be accurately predicted and dates far
 into the future may be out by a number of seconds. This is
 important if future dates are based around the start (or end)
 of a particular day - when those dates come to be viewed as
 the past, the actual day calculated from the number of seconds
 may differ to the day originally calculated. When using future
 dates, avoid using dates at the start or end of a particular day.

 \since v0.7.0

 @{
*/
/**
    @file qofdate.h 
    @brief 64bit Date handling routines
    @author Copyright 2006 Neil Williams <linux@codehelp.co.uk>
*/

#ifndef QOFDATE_H
#define QOFDATE_H

#include "qoftime.h"

/** \brief The maximum length of a string used for or created 
by dates.

When setting a custom QofDateFormat, neither the format string 
itself nor any date string created from that format is allowed 
to exceed this length.
*/
#define MAX_DATE_LENGTH 41
/** \brief The maximum length of a QofDate buffer

 \todo rationalise with MAX_DATE_LENGTH
*/
#define MAX_DATE_BUFFER 256
/** number of seconds in one whole day. */
#define SECS_PER_DAY 86400
/** number of seconds in one whole hour. */
#define SECS_PER_HOUR 3600
/** QofLogModule name for QofDate */
#define QOF_MOD_DATE "qof-dates"

/** \brief Full range replacement for struct tm.

Based on struct tm but using signed integers. The year value
uses a signed 64bit value to prevent overflows. (A glong
is insufficient by two orders of magnitude.) To retain precision, 
a QofDate includes a nanoseconds value that can be used 
with a QofTime and a 64bit value for seconds. 

 \note All QofDate values can be negative.
The normalising cascade handles rollovers. 
e.g. If a QofDate qd_min value is 5
initially, setting qd_sec to 68 causes qd_sec to actually
hold the value 8 and qd_min to hold the value 6. Alternatively, 
setting qd_sec to -64 with qd_min set to 5 causes qd_sec to hold 
the value 56 and qd_min to hold the value 3.

 \todo check - years work like this, days don't!!
Only qd_year retains a negative value once set. Adding
one year to a negative QofDate causes the QofDate to be set to
one year further into the past. This follows the same pattern
as typical BC dates: the 1st of May 501BC is further into the
past than the 5th of May 500BC.

Why is this a date? Because it represents a date,
broken down into the component variables. A QofTime
always (and only) relates to seconds, a QofDate always
relates to how that number of seconds can be represented
as a sequence of days, months, years etc.

 \todo Reorganise the qof_time_* functions to reflect this
statement. qof_time_set_day_end should be qof_date_set_day_end
and the various qof_date_time functions need to be reviewed.

*/
typedef struct QofDate_s
{
	/** From QofTime */
	glong qd_nanosecs;
	/** Enlarged replacement of struct tm.tm_sec */
	gint64 qd_sec;
	/** \brief Signed replacement of struct tm.tm_min.

Setting qd_min to a negative value adjusts the other date
values when the QofDate is validated and normalised to
create an earlier time. 
 */
	gshort qd_min;
	/** \brief Signed replacement of struct tm.tm_hour.

Setting qd_hour to a negative value adjusts the other date
values when the QofDate is validated and normalised to
create an earlier time. 
*/
	gshort qd_hour;
	/** \brief Signed replacement of struct tm.tm_mday.

Setting qd_mday to a negative value adjusts the other date
values when the QofDate is validated and normalised to
create an earlier time. 
 */
	gshort qd_mday;
	/** \brief Signed replacement of struct tm.tm_mon.

Setting qd_mon to a negative value adjusts the other date
values when the QofDate is validated and normalised to
create an earlier time. 
 */
	gshort qd_mon;
	/** \brief Extended version to cope with full range of dates. 

 \warning QofDate does not use 1900 or 1970 as a base, all 
 years in QofDate are true values. qd_year == 0 is the only
 invalid value (validation converts 0 to 1BC). A value of
 qd_year == 106 means 106AD, not 2006: A value of qd_year
 == 2006 means 2006, not 3906. Use negative values for dates
 in 1BC or earlier: qd_year == -43 means 43BC.
*/
	gint64 qd_year;
	/** Signed replacement of struct tm.tm_wday. 
qd_wday is a calculated value and will be overridden when
the QofDate is validated. */
	gshort qd_wday;
	/** Signed replacement of struct tm.tm_yday. 
qd_yday is a calculated value and will be overridden when
the QofDate is validated. */
	gshort qd_yday;
	/** Signed replacement of struct tm.tm_isdst. 
qd_is_dst is a calculated value and will be set to UTC when
the QofDate is validated. */
	gshort qd_is_dst;
	/** \brief Calculated value based on struct tm.tm_gmtoff.

 \note qd_gmt_off \b WILL be overridden to UTC when the QofDate
is validated. This can be used to convert a localtime to UTC -
set the value from struct tm.tm_gmtoff for the localtime and
validate the QofDate to get UTC.
*/
	glong qd_gmt_off;
	/** \brief Calculated value based on struct tm.tm_zone.

 \note qd_zone \b WILL be overridden to "GMT" when the QofDate
is validated. This can be used to convert a localtime to UTC -
set the value from struct tm.tm_zone for the localtime and
validate the QofDate to get UTC.

 \todo check this works.
 */
	const gchar *qd_zone;
	/** \brief If the QofDate is valid or merely initialised. 

Some QofDate values are invalid when initialised
to zero (e.g. qm_mday).
*/
	gboolean qd_valid;
} QofDate;

/** Nonzero if YEAR is a leap year (every 4 years,
except every 100th isn't, and every 400th is). */
# define qof_date_isleap(year)	\
  ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))

/** \brief initialise the QofDate tables */
void qof_date_init (void);

/** \brief close down the QofDate tables */
void qof_date_close (void);

/** \brief UTC date format string.

Timezone independent, date and time inclusive, as used in the
QSF backend. The T and Z characters are from xsd:dateTime format
in coordinated universal time, UTC. You can reproduce the string
from the GNU/Linux command line using the date utility:
 \verbatim
$ date -u +%Y-%m-%dT%H:M:SZ 
2004-12-12T23:39:11Z
 \endverbatim
The datestring must be timezone independent and include all
specified fields. Remember to use gmtime() NOT localtime()! 
*/
#define QOF_UTC_DATE_FORMAT     "%Y-%m-%dT%H:%M:%SZ"

/** \name Default QofDate formats
 @{
*/
/** \brief Continental US default. "%m/%d/%Y"

9th May 2006 == 05/09/2006
*/
#define QOF_DATE_FORMAT_US      1
/** \brief United Kingdom default. "%d/%m/%Y"

9th May 2006 == 09/05/2006
*/
#define QOF_DATE_FORMAT_UK      2
/** \brief Contintental European default. "%d.%m.%Y"

9th May 2006 == 09.05.2006
*/
#define QOF_DATE_FORMAT_CE      3
/** \brief Short ISO form. "%F"

9th May 2006 == 2006-05-09
*/
#define QOF_DATE_FORMAT_ISO     4
/** \brief QOF UTC format, xsd:date compatible. 
QOF_UTC_DATE_FORMAT

xsd:date is recommended for any XML data storage of dates and 
times.

9th May 2006 == 2006-05-09T14:49:04Z
*/
#define QOF_DATE_FORMAT_UTC     5
/** \brief GNU locale default. "%x"

QOF_DATE_FORMAT_LOCALE and QOF_DATE_FORMAT_CUSTOM are only 
suitable for date / time display - storing these values 
in any kind of file is a recipe for disaster as the exact 
format can be affected by environment variables and other 
imponderables.

One example: 9th May 2006 gives 09/05/06

 \note QOF_DATE_FORMAT_LOCALE includes locale-specific format 
 specifiers and therefore cannot support the full range of
 QofDate. see \ref datelocales
*/
#define QOF_DATE_FORMAT_LOCALE  6
/** \brief Date and time for the current locale "%c"

QOF_DATE_FORMAT_LOCALE and QOF_DATE_FORMAT_CUSTOM are only 
suitable for date / time display - storing these values in 
any kind of file is a recipe for disaster as the exact 
format can be affected by environment variables and other 
imponderables.

One example: 9th May 2006 gives Tue 09 May 2006 14:50:10 UTC

 \note QOF_DATE_FORMAT_CUSTOM includes locale-specific format 
 specifiers and therefore cannot support the full range of
 QofDate. see \ref datelocales
*/
#define QOF_DATE_FORMAT_CUSTOM  7

/** New identifiers must be larger than this. */
#define DATE_FORMAT_LAST  QOF_DATE_FORMAT_CUSTOM

/** @} */

/** convenience macro to turn hours into seconds. */
#define QOF_HOUR_TO_SEC(x) (x * SECS_PER_HOUR)
/** convenience macro to turn minutes into seconds. */
#define QOF_MIN_TO_SEC(x) (x * 60)
/** convenience macro to turn days into seconds. */
#define QOF_DAYS_TO_SEC(x) (x * SECS_PER_DAY)

/** Index value of the selected QofDateFormat in the 
DateFormatTable */
typedef gint QofDateFormat;

/** \name QofDateFormat - standardised date formats

To simplify usage of strftime and strptime (especially 
checking error states), QofDate uses a set of standard 
date formats. You can also register your own format 
strings as long as they are strftime compatible.

  see also \ref datelocales
 @{
*/
/** \brief Add a specific strftime compatible string as a new 
QofDateFormat

Unlike GDate, QofDate allows time-related formats.

 \param str A pre-formatted string, suitable to be passed 
directly to strftime.
 \param identifier Positive integer value to be used to 
identify this date format later. Must be greater than 
DATE_FORMAT_LAST. If the identifier is greater than 
DATE_FORMAT_LAST but already exists, the previous value
will be overridden.
 \return TRUE on success, otherwise FALSE
*/
gboolean 
qof_date_format_add (const gchar * str, QofDateFormat identifier);

/** \brief Retrieve the shorthand name for the selected date 
format.

If the selected QofDateFormat is one of the defaults, a shorthand
"name" is used. If it is a string added using 
qof_date_add_format, the string itself is returned.

 \param format The QofDateFormat to lookup.
 \return FALSE on success and TRUE on failure.
*/
const gchar *
qof_date_format_to_name (QofDateFormat format);

/** \brief Returns the default date format for a known shorthand 
name.

If the selected QofDateFormat is one of the defaults, the 
shorthand "name" is returned. If format is not a default, 
returns negative one.

 \param name Shorthand "name" of this format.
 \return the QofDateFormat on success, negative one on failure.
*/
QofDateFormat 
qof_date_format_from_name (const gchar * name);

/** \brief Set a shorthand name for a custom date format.

Used alongside ::qof_date_format_add to allow any date format
to have a shorthand name. 

 \param name Shorthand name for a date format added with 
qof_date_format_add. The string becomes the property of QofDate 
and should not be freed.
 \param format identifier used previously with 
qof_date_format_add
 \return TRUE if the shorthand name can be set, FALSE on error or 
if the chosen QofDateFormat is one of the defaults.
*/
gboolean 
qof_date_format_set_name (const gchar * name, QofDateFormat format);

/** \brief returns the current date format. */
QofDateFormat 
qof_date_format_get_current (void);

/** \brief Selects one registered date format as the current 
default.

 \param df QofDateFormat identifier indicating preferred format.
 \return TRUE on success, else FALSE.
*/
gboolean 
qof_date_format_set_current (QofDateFormat df);

/** \brief Retrieve the strftime format string for a registered 
date format.
 @param df The QofDateFormat identifier for the registered date 
format.

 @return The format string for this date format or NULL on error.
*/
const gchar *
qof_date_format_get_format (QofDateFormat df);

/** \brief Return the field separator for the current date format

 \note The separator only relates to the date portion of any 
date format string, i.e. the separator used between day, month 
and year. Separators used between time fields like hour, minute, 
second in any date format are not available.

 \return date single non-digit character to separate fields 
within the date section of a date format or a null on error.
*/
gchar 
qof_date_format_get_date_separator (QofDateFormat df);

/** \brief Set a locale-specific separator.

Sets the date separator for a date format added using 
::qof_date_format_add.

 \return FALSE if date format is not one of the QOF defaults
or if the character is a digit, TRUE on success.
*/
gboolean
qof_date_format_set_date_separator (const gchar sep, QofDateFormat df);
/** @} */

/** \name QofDate handlers
 @{
*/
/** create a new QofDate */
QofDate *
qof_date_new (void);

/** free a QofDate */
void 
qof_date_free (QofDate * date);

/** Calculate the QofTime between two QofDates */ 
QofTime*
qof_date_time_difference (QofDate * date1, QofDate * date2);

/** Check two QofDates for equality */
gboolean
qof_date_equal (const QofDate *d1, const QofDate *d2);

/** Compare two QofDates */
gint
qof_date_compare (const QofDate * d1, const QofDate * d2);

/** \brief Validate a QofDate 

If the QofDate is already valid, just returns TRUE.
If the QofDate is not valid but can be normalised, the QofDate
is normalised and the function returns TRUE.
If the QofDate cannot be normalised, returns FALSE.

Year Zero does not exist in the Christian Era, the Gregorian 
calendar or the Julian calendar. A year zero does exist in 
ISO 8601:2004 and in the astronomical year numbering with a 
defined year zero equal to 1 BC, as well as in some Buddhist 
and Hindu lunar calendars. 

In QofDate, 1BC is immediately followed by 1AD
and months are numbered from 1 to 12, not from zero.

Normalising a QofDate tries to use sensible defaults:
- if qd_mon  == 0, validating sets qd_mon  to  1 (January)
- if qd_year == 0, validating sets qd_year to -1 (1BC).
- if qd_mday == 0, validating sets qd_mday to  1.
*/
gboolean
qof_date_valid (QofDate *date);

/** @} */

/** \name Conversion handlers for QofDate
 @{
*/
/** Return a QofDate from a QofTime in UTC */
QofDate *
qof_date_from_qtime (const QofTime *qt);

/** not const because validation normalises date */
QofTime *
qof_date_to_qtime (QofDate *qd);

/** \brief Convert a struct tm to a QofDate.

 \param tm A pointer to a valid struct tm.
 \return Newly allocated QofDate or NULL if tm is NULL.
*/
QofDate *
qof_date_from_struct_tm (struct tm *tm);

/** \brief Convert a QofDate to a struct tm

 \warning Check the return value - a QofDate has
a larger range than a struct tm. The struct tm
will be unchanged if a conversion would have been
out of range.

 \todo return nanoseconds to prevent data loss.

The QofDate will be normalised before conversion.

 \param qt A valid QofDate.
 \param tm Pointer to a struct tm to store the result.
 \param nanosecs Pointer to a glong to store the nanoseconds.
 \return FALSE on error or if the QofDate is out of the
range of a struct tm, otherwise TRUE.
*/
gboolean 
qof_date_to_struct_tm (QofDate * qt, struct tm *tm, glong * nanosecs);

/** \brief Convert a GDate to a QofDate

 \param qd a new QofDate to store the converted value
 \param gd a valid GDate
*/
gboolean
qof_date_to_gdate (QofDate *qd, GDate *gd);

/** \brief Convert a QofDate to a GDate

A GDate is always within the range of a QofDate.
*/
QofDate *
qof_date_from_gdate (GDate *gd);

/** @} */
/** \name Manipulate QofTime as a date

Shorthand routines to modify a QofTime using date-type values, 
instead of having to always use seconds.
 @{
*/
/** \brief Add a number of days to a QofTime and normalise.

Together with qof_date_time_add_months, replaces date_add_months.

 \return FALSE on error, otherwise TRUE.
*/
gboolean 
qof_date_time_add_days (QofTime * ts, gint days);

/** \brief Add a number of months to a QofTime.

Optionally track the last day of the month so that adding one
month to 31st January returns 28th February 
(29th in a leap year) and adding three months returns 
30th April.

Safe for all dates within the range of GDate.

 \return FALSE on error, otherwise TRUE.
*/
gboolean 
qof_date_time_add_months (QofTime * ts, guint8 months,
			gboolean track_last_day);

/**@} */

/** \name Date Printing/Scanning functions

 QofDate supports a wider range of dates than either strftime or
 GDate and supports all non-locale-specific strftime format
 specifiers over the full range of QofDate.

 \note Locale-specific formats cannot be extended to the full
 range of QofDate dates because the locale data for these
 formats is only available to the underlying strftime 
 implementation. The formats affected are those involving 
 the %E and %O modifiers and other format specifiers that use 
 the current locale. e.g. Japanese Emperor reigns, local 
 numeric specifiers, translated days of the week / month etc.
 If these are used, only dates within the range of the 
 locale-sensitive strftime on that platform can be
 supported (either inside or outside QofDate).

 The full list of affected format specifiers is:

 \verbatim
'a', 'A', 'b', 'h', 'B', 'c', 'C', 'x', 'p', 'P', 'r', 'X', 'E', 'O'.
 \endverbatim

 QofDate will attempt to fallback to a usable format 
 if the date is out of range of the underlying strftime. 
 e.g. QOF_DATE_FORMAT_UTC, QOF_DATE_FORMAT_UK, 
 QOF_DATE_FORMAT_US, QOF_DATE_FORMAT_CE or QOF_DATE_FORMAT_ISO.

 \note It is not particularly sensible to write locale-sensitive 
 date strings to any kind of permanent storage. Locale-specific
 format specifiers should only be used for displaying dates to
 the user.

 For more information, see \ref datelocales

- Printing: Convert a QofTime* into a timestamp.
  -# To print a string yourself, use 
    ::qof_date_format_get_format to get the format to pass to 
    strftime or g_date_strftime.
  -# To use a registered date format, use 
    ::qof_date_format_set_current and ::qof_date_print.

- Scanning: Convert a timestamp into a QofTime*
  -# To scan a string yourself, use ::qof_date_format_get_format 
    to get the format to pass to strptime or use g_date_set_parse
  -# To scan a stamp created with a registered date format, use
    ::qof_date_parse
 @{
*/

/** \brief Convert a QofDate to a timestamp according to the 
specified date format. 

Unlike qof_time_stamp_now, any supported QofDate can be converted
in any registered QofDateFormat. 

 \param date A valid QofDate.
 \param df a registered QofDateFormat to use to create 
 the string.

 \return NULL on error, otherwise a string which should be freed 
when no longer needed.
*/
gchar *
qof_date_print (QofDate * date, QofDateFormat df);

/** \brief Convert a timestamp to a QofTime

Safe for all dates within the range of QofDate.

 \param str a timestamp created with one of the registered 
 QofDateFormat formats.
 \param df The registered QofDateFormat that produced the
 string.
 \return a newly allocated, valid, QofDate or NULL on error.
*/
QofDate *
qof_date_parse (const gchar * str, QofDateFormat df);

/** @} */
/** @} */
/** @} */

#endif /* QOFDATE_H */
