/********************************************************************
 * qofnumeric.h - A rational number library                         *
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
 *******************************************************************/

/** @addtogroup Numeric

    The 'Numeric' functions provide a way of working with rational
    numbers while maintaining strict control over rounding errors
    when adding rationals with different denominators.  The Numeric
    class is primarily used for working with monetary amounts, 
    where the denominator typically represents the smallest fraction
    of the currency (e.g. pennies, centimes).  The numeric class
    can handle any fraction (e.g. twelfth's) and is not limited
    to fractions that are powers of ten.  

    A 'Numeric' value represents a number in rational form, with a
    64-bit integer as numerator and denominator.  Rationals are
    ideal for many uses, such as performing exact, roundoff-error-free
    addition and multiplication, but 64-bit rationals do not have 
    the dynamic range of floating point numbers.  

See \ref qofnumericexample

@{ */
/** @file qofnumeric.h
    @brief An exact-rational-number library for QOF.
    @author Copyright (C) 2000 Bill Gribble
    @author Copyright (C) 2004 Linas Vepstas <linas@linas.org>
    @author Copyright (c) 2006 Neil Williams <linux@codehelp.co.uk>
*/

#ifndef QOF_NUMERIC_H
#define QOF_NUMERIC_H

struct _QofNumeric
{
	gint64 num;
	gint64 denom;
};

/** @brief An rational-number type
 *
 * This is a rational number, defined by numerator and denominator. */
typedef struct _QofNumeric QofNumeric;

/** @name Arguments Standard Arguments to most functions

    Most of the QofNumeric arithmetic functions take two arguments
    in addition to their numeric args: 'denom', which is the denominator
    to use in the output QofNumeric object, and 'how'. which
    describes how the arithmetic result is to be converted to that
    denominator. This combination of output denominator and rounding policy
    allows the results of financial and other rational computations to be
    properly rounded to the appropriate units.

    Valid values for denom are:
    QOF_DENOM_AUTO  -- compute denominator exactly
    integer n       -- Force the denominator of the result to be this integer
    QOF_DENOM_RECIPROCAL -- Use 1/n as the denominator (???huh???)

    Valid values for 'how' are bitwise combinations of zero or one
    "rounding instructions" with zero or one "denominator types".
    Valid rounding instructions are:
        QOF_HOW_RND_FLOOR
        QOF_HOW_RND_CEIL 
        QOF_HOW_RND_TRUNC
        QOF_HOW_RND_PROMOTE 
        QOF_HOW_RND_ROUND_HALF_DOWN
        QOF_HOW_RND_ROUND_HALF_UP 
        QOF_HOW_RND_ROUND
        QOF_HOW_RND_NEVER

    The denominator type specifies how to compute a denominator if
    QOF_DENOM_AUTO is specified as the 'denom'. Valid 
    denominator types are:
        QOF_HOW_DENOM_EXACT  
        QOF_HOW_DENOM_REDUCE 
        QOF_HOW_DENOM_LCD   
        QOF_HOW_DENOM_FIXED 
        QOF_HOW_DENOM_SIGFIGS(N)

   To use traditional rational-number operational semantics (all results
   are exact and are reduced to relatively-prime fractions) pass the
   argument QOF_DENOM_AUTO as 'denom' and 
   QOF_HOW_DENOM_REDUCE| QOF_HOW_RND_NEVER as 'how'.

   To enforce strict financial semantics (such that all operands must have
   the same denominator as each other and as the result), use
   QOF_DENOM_AUTO as 'denom' and 
   QOF_HOW_DENOM_FIXED | QOF_HOW_RND_NEVER as 'how'.
@{
*/

/** \brief bitmasks for HOW flags.

 * bits 8-15 of 'how' are reserved for the number of significant
 * digits to use in the output with QOF_HOW_DENOM_SIGFIG 
 */
#define QOF_NUMERIC_RND_MASK     0x0000000f
#define QOF_NUMERIC_DENOM_MASK   0x000000f0
#define QOF_NUMERIC_SIGFIGS_MASK 0x0000ff00

/** \brief Rounding/Truncation modes for operations.

 *  Rounding instructions control how fractional parts in the specified
 *  denominator affect the result. For example, if a computed result is
 *  "3/4" but the specified denominator for the return value is 2, should
 *  the return value be "1/2" or "2/2"?  
 *
 * Possible rounding instructions are:
 */
enum
{
  /** Round toward -infinity */
	QOF_HOW_RND_FLOOR = 0x01,

  /** Round toward +infinity */
	QOF_HOW_RND_CEIL = 0x02,

  /** Truncate fractions (round toward zero) */
	QOF_HOW_RND_TRUNC = 0x03,

  /** Promote fractions (round away from zero) */
	QOF_HOW_RND_PROMOTE = 0x04,

  /** Round to the nearest integer, rounding toward zero 
   *  when there are two equidistant nearest integers.
   */
	QOF_HOW_RND_ROUND_HALF_DOWN = 0x05,

  /** Round to the nearest integer, rounding away from zero 
   *  when there are two equidistant nearest integers.
   */
	QOF_HOW_RND_ROUND_HALF_UP = 0x06,

  /** Use unbiased ("banker's") rounding. This rounds to the 
   *  nearest integer, and to the nearest even integer when there 
   *  are two equidistant nearest integers. This is generally the 
   *  one you should use for financial quantities.
   */
	QOF_HOW_RND_ROUND = 0x07,

  /** Never round at all, and signal an error if there is a 
   *  fractional result in a computation.
   */
	QOF_HOW_RND_NEVER = 0x08
};

/** How to compute a denominator, or'ed into the "how" field. */
enum
{
  /** Use any denominator which gives an exactly correct ratio of 
   *  numerator to denominator. Use EXACT when you do not wish to 
   *  lose any information in the result but also do not want to 
   *  spend any time finding the "best" denominator.
   */
	QOF_HOW_DENOM_EXACT = 0x10,

  /** Reduce the result value by common factor elimination, 
   *  using the smallest possible value for the denominator that 
   *  keeps the correct ratio. The numerator and denominator of 
   *  the result are relatively prime. 
   */
	QOF_HOW_DENOM_REDUCE = 0x20,

  /** Find the least common multiple of the arguments' denominators 
   *  and use that as the denominator of the result.
   */
	QOF_HOW_DENOM_LCD = 0x30,

  /** All arguments are required to have the same denominator,
   *  that denominator is to be used in the output, and an error 
   *  is to be signaled if any argument has a different denominator.
   */
	QOF_HOW_DENOM_FIXED = 0x40,

  /** Round to the number of significant figures given in the rounding
   *  instructions by the QOF_HOW_DENOM_SIGFIGS () macro.
   */
	QOF_HOW_DENOM_SIGFIG = 0x50
};

/** Build a 'how' value that will generate a denominator that will 
 *  keep at least n significant figures in the result.
 */
#define QOF_HOW_DENOM_SIGFIGS( n ) ( ((( n ) & 0xff) << 8) | QOF_HOW_DENOM_SIGFIG)
#define QOF_HOW_GET_SIGFIGS( a ) ( (( a ) & 0xff00 ) >> 8)

/** Error codes */
typedef enum
{
	QOF_ERROR_OK = 0,	   /**< No error */
	QOF_ERROR_ARG = -1,	   /**< Argument is not a valid number */
	QOF_ERROR_OVERFLOW = -2,   /**< Intermediate result overflow */

  /** QOF_HOW_DENOM_FIXED was specified, but argument denominators differed.  */
	QOF_ERROR_DENOM_DIFF = -3,

  /** QOF_HOW_RND_NEVER  was specified, but the result could not be
   *  converted to the desired denominator without a remainder. */
	QOF_ERROR_REMAINDER = -4
} QofNumericErrorCode;


/** Values that can be passed as the 'denom' argument.  
 *  The include a positive number n to be used as the 
 *  denominator of the output value.  Other possibilities 
 *  include the list below:
 */

/** Compute an appropriate denominator automatically. Flags in 
 *  the 'how' argument will specify how to compute the denominator.
 */
#define QOF_DENOM_AUTO 0

/** Use the value 1/n as the denominator of the output value. */
#define QOF_DENOM_RECIPROCAL( a ) (- ( a ))

/**  @} */

/** @name Constructors
@{
*/
/** Make a QofNumeric from numerator and denominator */
static inline QofNumeric
qof_numeric_create (gint64 num, gint64 denom)
{
	QofNumeric out;
	out.num = num;
	out.denom = denom;
	return out;
}

/** create a zero-value QofNumeric */
static inline QofNumeric
qof_numeric_zero (void)
{
	return qof_numeric_create (0, 1);
}

/** Convert a floating-point number to a QofNumeric. 
 *  Both 'denom' and 'how' are used as in arithmetic, 
 *  but QOF_DENOM_AUTO is not recognized; a denominator
 *  must be specified either explicitctly or through sigfigs.
 */
QofNumeric
qof_numeric_from_double (gdouble in, gint64 denom, gint how);

/** Read a QofNumeric from str, skipping any leading whitespace.
 *  Return TRUE on success and store the resulting value in "n".
 *  Return NULL on error. */
gboolean
qof_numeric_from_string (const gchar * str, QofNumeric * n);

/** Create a QofNumeric object that signals the error condition
 *  noted by error_code, rather than a number. 
 */
QofNumeric
qof_numeric_error (QofNumericErrorCode error_code);
/** @} */

/** @name Value Accessors
 @{
*/
/** Return numerator */
static inline gint64
qof_numeric_num (QofNumeric a)
{
	return a.num;
}

/** Return denominator */
static inline gint64
qof_numeric_denom (QofNumeric a)
{
	return a.denom;
}

/** Convert numeric to floating-point value. */
gdouble
qof_numeric_to_double (QofNumeric in);

/** Convert to string. The returned buffer is to be g_free'd by the
 *  caller (it was allocated through g_strdup) */
gchar *
qof_numeric_to_string (QofNumeric n);

/** Convert to string. Uses a static, non-thread-safe buffer.
 *  For internal use only. */
gchar *
qof_numeric_dbg_to_string (QofNumeric n);
/** @}*/

/** @name Comparisons and Predicates 
 @{
*/
/** Check for error signal in value. Returns QOF_ERROR_OK (==0) if
 *  the number appears to be valid, otherwise it returns the
 *  type of error.  Error values always have a denominator of zero.
 */
QofNumericErrorCode
qof_numeric_check (QofNumeric a);

/** Returns 1 if a>b, -1 if b>a, 0 if a == b  */
gint 
qof_numeric_compare (QofNumeric a, QofNumeric b);

/** Returns 1 if the given QofNumeric is 0 (zero), else returns 0. */
gboolean 
qof_numeric_zero_p (QofNumeric a);

/** Returns 1 if a < 0, otherwise returns 0. */
gboolean 
qof_numeric_negative_p (QofNumeric a);

/** Returns 1 if a > 0, otherwise returns 0. */
gboolean 
qof_numeric_positive_p (QofNumeric a);

/** Equivalence predicate: Returns TRUE (1) if a and b are 
 *  exactly the same (have the same numerator and denominator)
 */
gboolean 
qof_numeric_eq (QofNumeric a, QofNumeric b);

/** Equivalence predicate: Returns TRUE (1) if a and b represent
 *  the same number.  That is, return TRUE if the ratios, when 
 *  reduced by eliminating common factors, are identical.
 */
gboolean 
qof_numeric_equal (QofNumeric a, QofNumeric b);

/** Equivalence predicate: 
 *  Convert both a and b to denom using the 
 *  specified DENOM and method HOW, and compare numerators 
 *  the results using QofNumeric_equal.
 *
  For example, if a == 7/16 and b == 3/4,
  QofNumeric_same(a, b, 2, QOF_HOW_RND_TRUNC) == 1
  because both 7/16 and 3/4 round to 1/2 under truncation. However,
  QofNumeric_same(a, b, 2, QOF_HOW_RND_ROUND) == 0
  because 7/16 rounds to 1/2 under unbiased rounding but 3/4 rounds
  to 2/2.
 */
gint 
qof_numeric_same (QofNumeric a, QofNumeric b, gint64 denom, gint how);
/** @} */

/** @name Arithmetic Operations 
 @{ 
*/
/** Return a+b. */
QofNumeric 
qof_numeric_add (QofNumeric a, QofNumeric b,
							 gint64 denom, gint how);

/** Return a-b. */
QofNumeric 
qof_numeric_sub (QofNumeric a, QofNumeric b,
							 gint64 denom, gint how);

/** Multiply a times b, returning the product.  An overflow
 *  may occur if the result of the multiplication can't
 *  be represented as a ratio of 64-bit int's after removing
 *  common factors.
 */
QofNumeric 
qof_numeric_mul (QofNumeric a, QofNumeric b,
							 gint64 denom, gint how);

/** Division.  Note that division can overflow, in the following 
 *  sense: if we write x=a/b and y=c/d  then x/y = (a*d)/(b*c)  
 *  If, after eliminating all common factors between the numerator 
 *  (a*d) and the denominator (b*c),  then if either the numerator 
 *  and/or the denominator are *still* greater than 2^63, then 
 *  the division has overflowed.
 */
QofNumeric 
qof_numeric_div (QofNumeric x, QofNumeric y,
							 gint64 denom, gint how);
/** Negate the argument  */
QofNumeric 
qof_numeric_neg (QofNumeric a);

/** Return the absolute value of the argument */
QofNumeric 
qof_numeric_abs (QofNumeric a);

/** 
 * Shortcut for common case: QofNumeric_add(a, b, QOF_DENOM_AUTO,
 *                        QOF_HOW_DENOM_FIXED | QOF_HOW_RND_NEVER);
 */
static inline QofNumeric
qof_numeric_add_fixed (QofNumeric a, QofNumeric b)
{
	return qof_numeric_add (a, b, QOF_DENOM_AUTO,
				QOF_HOW_DENOM_FIXED | QOF_HOW_RND_NEVER);
}

/** 
 * Shortcut for most common case: QofNumeric_sub(a, b, QOF_DENOM_AUTO,
 *                        QOF_HOW_DENOM_FIXED | QOF_HOW_RND_NEVER);
 */
static inline QofNumeric
qof_numeric_sub_fixed (QofNumeric a, QofNumeric b)
{
	return qof_numeric_sub (a, b, QOF_DENOM_AUTO,
				QOF_HOW_DENOM_FIXED | QOF_HOW_RND_NEVER);
}

/** @} */

/** @name Arithmetic Functions with Exact Error Returns 
 @{
*/
/** The same as QofNumeric_add, but uses 'error' for accumulating
 *  conversion roundoff error. */
QofNumeric 
qof_numeric_add_with_error (QofNumeric a, QofNumeric b,
										gint64 denom, gint how,
					QofNumeric * error);

/** The same as QofNumeric_sub, but uses error for accumulating
 *  conversion roundoff error. */
QofNumeric 
qof_numeric_sub_with_error (QofNumeric a, QofNumeric b,
										gint64 denom, gint how,
					QofNumeric * error);

/** The same as QofNumeric_mul, but uses error for 
 *  accumulating conversion roundoff error.
 */
QofNumeric 
qof_numeric_mul_with_error (QofNumeric a, QofNumeric b,
										gint64 denom, gint how,
					QofNumeric * error);

/** The same as QofNumeric_div, but uses error for 
 *  accumulating conversion roundoff error.
 */
QofNumeric 
qof_numeric_div_with_error (QofNumeric a, QofNumeric b,
										gint64 denom, gint how,
					QofNumeric * error);
/** @} */

/** @name Change Denominator 
 @{
*/
/** Change the denominator of a QofNumeric value to the 
 *  specified denominator under standard arguments 
 *  'denom' and 'how'. 
 */
QofNumeric 
qof_numeric_convert (QofNumeric in, gint64 denom, gint how);

/** Same as QofNumeric_convert, but return a remainder 
 *  value for accumulating conversion error. 
*/
QofNumeric 
qof_numeric_convert_with_error (QofNumeric in, gint64 denom,
								gint how, QofNumeric * error);

/** Return input after reducing it by Greated Common Factor (GCF) 
 *  elimination */
QofNumeric qof_numeric_reduce (QofNumeric in);
/** @} */

/** @name Deprecated, backwards-compatible definitions 
  @{
*/
#define QOF_RND_FLOOR			QOF_HOW_RND_FLOOR
#define QOF_RND_CEIL 			QOF_HOW_RND_CEIL
#define QOF_RND_TRUNC			QOF_HOW_RND_TRUNC
#define QOF_RND_PROMOTE 		QOF_HOW_RND_PROMOTE
#define QOF_RND_ROUND_HALF_DOWN	QOF_HOW_RND_ROUND_HALF_DOWN
#define QOF_RND_ROUND_HALF_UP 	QOF_HOW_RND_ROUND_HALF_UP
#define QOF_RND_ROUND			QOF_HOW_RND_ROUND
#define QOF_RND_NEVER			QOF_HOW_RND_NEVER

#define QOF_DENOM_EXACT  		QOF_HOW_DENOM_EXACT
#define QOF_DENOM_REDUCE 		QOF_HOW_DENOM_REDUCE
#define QOF_DENOM_LCD   		QOF_HOW_DENOM_LCD
#define QOF_DENOM_FIXED 		QOF_HOW_DENOM_FIXED
#define QOF_DENOM_SIGFIG 		QOF_HOW_DENOM_SIGFIG

#define QOF_DENOM_SIGFIGS(X)  	QOF_HOW_DENOM_SIGFIGS(X)
#define QOF_NUMERIC_GET_SIGFIGS(X) QOF_HOW_GET_SIGFIGS(X)
/** @} */
/** @} */
#endif
