/***************************************************************************
 *            deprecated.h
 *
 *  Mon Nov 21 14:08:25 2005
 *  Copyright  2005  Neil Williams
 *  linux@codehelp.co.uk
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifndef _DEPRECATED_H
#define _DEPRECATED_H

/** @file deprecated.h
	@brief transitional header from libqof1 to libqof2
*/

/** \deprecated use QofLogLevel instead */
#define gncLogLevel QofLogLevel

/** \deprecated use qof_log_init_filename instead */
#define gnc_log_init qof_log_init

/** \deprecated use qof_log_set_level insead. */
#define gnc_set_log_level qof_log_set_level

/** \deprecated use qof_log_set_level_global instead. */
#define gnc_set_log_level_global qof_log_set_level_global

/** \deprecated use qof_log_set_file instead. */
#define gnc_set_logfile qof_log_set_file

/** \deprecated use qof_log_prettify instead. */
#define gnc_log_prettify qof_log_prettify

/** \deprecated use qof_log_check instead. */
#define gnc_should_log qof_log_check

/** \deprecated */
#define GNC_LOG_FATAL   QOF_LOG_FATAL
/** \deprecated */
#define GNC_LOG_ERROR   QOF_LOG_ERROR
/** \deprecated */
#define GNC_LOG_WARNING QOF_LOG_WARNING
/** \deprecated */
#define GNC_LOG_INFO    QOF_LOG_INFO
/** \deprecated */
#define GNC_LOG_DEBUG   QOF_LOG_DEBUG
/** \deprecated */
#define GNC_LOG_DETAIL  QOF_LOG_DETAIL
/** \deprecated */
#define GNC_LOG_TRACE   QOF_LOG_TRACE


#endif /* _DEPRECATED_H */
