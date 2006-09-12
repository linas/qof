/********************************************************************\
 * qof.h -- Master QOF public include file                          *
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

#ifndef QOF_H_
#define QOF_H_
/** @defgroup QOF Query Object Framework 
 @{
	@addtogroup Book Book:        The QOF Data Set.
	@ingroup QOF
    @addtogroup Object Object:    Describing data structure.
    @ingroup QOF
    @addtogroup Class Class:      Getting and setting entity values.
    @ingroup QOF
    @addtogroup QOFTIME Time:     64bit UTC Time handling.
    @ingroup QOF
    @addtogroup Numeric Numeric:  Rational Number Handling with Rounding Error Control
    @ingroup QOF
    @addtogroup KVP KVP:          Key-Value Pairs
    @ingroup QOF
	@addtogroup Backend Backends: Permanent storage for QOF entities.
	@ingroup QOF
    @addtogroup Query Query:      Querying for Objects
    @ingroup QOF
	@addtogroup Error Error: Extensible error handling.
	@ingroup QOF
    @addtogroup Trace Trace:      Debugging support
    @ingroup QOF
    @addtogroup Event Event:      QOF event handlers.
    @ingroup QOF
    @addtogroup Choice Choice:    One to many links.
    @ingroup QOF
    @addtogroup BookMerge Merge:  Merging QofBook structures
    @ingroup QOF
    @addtogroup Reference Reference: Referring to entities outside a partial book.
    @ingroup QOF
    @addtogroup UNDO Undo:        Track and undo or redo entity changes
    @ingroup QOF
    @addtogroup Utilities Utilities: Miscellany
    @ingroup QOF
@}
*/

#include <glib.h>
#include "qofid.h"
#include "qoflog.h"
#include "qofdate.h"
#include "qoftime.h"
#include "qofnumeric.h"
#include "qofutil.h"
#include "guid.h"
#include "kvp_frame.h"
#include "kvp-util.h"
#include "kvp-util-p.h"
#include "qofbackend.h"
#include "qofid-p.h"
#include "qofinstance-p.h"
#include "qofbook.h"
#include "qofclass.h"
#include "qofevent.h"
#include "qofobject.h"
#include "qofquery.h"
#include "qofquerycore.h"
#include "qoferror.h"
#include "qofsession.h"
#include "qofsql.h"
#include "qofchoice.h"
#include "qofbookmerge.h"
#include "qofreference.h"
#include "qofla-dir.h"
#include "qofundo.h"
#include "deprecated.h"

/** allow easy logging of QSF debug messages */
#define QOF_MOD_QSF "qof-backend-qsf"
/** allow easy loading of the QSF backend */
#define QSF_BACKEND_LIB "libqof-backend-qsf"
/** allow easy loading of the QSF backend */
#define QSF_MODULE_INIT "qsf_provider_init"

#endif /* QOF_H_ */
