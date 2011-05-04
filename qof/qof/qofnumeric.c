/********************************************************************
 * qofnumeric.c -- an exact-number library for accounting use       *
 * Copyright (C) 2000 Bill Gribble                                  *
 * Copyright (C) 2004 Linas Vepstas <linas@linas.org>               *
 * Copyright (c) 2006-2008 Neil Williams <linux@codehelp.co.uk>     *
 *                                                                  *
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

#include "config.h"

#include <glib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qof.h"
#include "qofnumeric.h"
#include "qofmath128.c"

/* =============================================================== */
QofNumericErrorCode
qof_numeric_check (QofNumeric in)
{
	if (in.denom != 0)
		return QOF_ERROR_OK;
	else if (in.num)
	{
		if ((0 < in.num) || (-4 > in.num))
			in.num = (gint64) QOF_ERROR_OVERFLOW;
		return (QofNumericErrorCode) in.num;
	}
	else
		return QOF_ERROR_ARG;
}

/*
 *  Find the least common multiple of the denominators of a and b.
 */

static gint64
qof_numeric_lcd (QofNumeric a, QofNumeric b)
{
	QofInt128 lcm;
	if (qof_numeric_check (a) || qof_numeric_check (b))
		return QOF_ERROR_ARG;
	if (b.denom == a.denom)
		return a.denom;
	/* Special case: smaller divides smoothly into larger */
	if ((b.denom < a.denom) && ((a.denom % b.denom) == 0))
		return a.denom;
	if ((a.denom < b.denom) && ((b.denom % a.denom) == 0))
		return b.denom;
	lcm = lcm128 (a.denom, b.denom);
	if (lcm.isbig)
		return QOF_ERROR_ARG;
	return lcm.lo;
}

/* Return the ratio n/d reduced so that there are no common factors. */
static QofNumeric
reduce128 (QofInt128 n, gint64 d)
{
	gint64 t;
	gint64 num;
	gint64 denom;
	QofNumeric out;
	QofInt128 red;

	t = rem128 (n, d);
	num = d;
	denom = t;

	/* The strategy is to use Euclid's algorithm */
	while (denom > 0)
	{
		t = num % denom;
		num = denom;
		denom = t;
	}
	/* num now holds the GCD (Greatest Common Divisor) */

	red = div128 (n, num);
	if (red.isbig)
		return qof_numeric_error (QOF_ERROR_OVERFLOW);
	out.num = red.lo;
	if (red.isneg)
		out.num = -out.num;
	out.denom = d / num;
	return out;
}

/* *******************************************************************
 *  qof_numeric_zero_p
 ********************************************************************/

gboolean
qof_numeric_zero_p (QofNumeric a)
{
	if (qof_numeric_check (a))
		return 0;
	else
	{
		if ((a.num == 0) && (a.denom != 0))
			return 1;
		else
			return 0;
		}
}

/* *******************************************************************
 *  qof_numeric_negative_p
 ********************************************************************/

gboolean
qof_numeric_negative_p (QofNumeric a)
{
	if (qof_numeric_check (a))
		return 0;
	else
	{
		if ((a.num < 0) && (a.denom != 0))
			return 1;
		else
			return 0;
		}
}

/* *******************************************************************
 *  qof_numeric_positive_p
 ********************************************************************/

gboolean
qof_numeric_positive_p (QofNumeric a)
{
	if (qof_numeric_check (a))
		return 0;
	else
	{
		if ((a.num > 0) && (a.denom != 0))
			return 1;
		else
			return 0;
		}
}

/* *******************************************************************
 *  qof_numeric_compare
 *  returns 1 if a>b, -1 if b>a, 0 if a == b 
 ********************************************************************/

gint
qof_numeric_compare (QofNumeric a, QofNumeric b)
{
	gint64 aa, bb;
	QofInt128 l, r;

	if (qof_numeric_check (a) || qof_numeric_check (b))
		return 0;

	if (a.denom == b.denom)
	{
		if (a.num == b.num)
			return 0;
		if (a.num > b.num)
			return 1;
		return -1;
	}

	if ((a.denom > 0) && (b.denom > 0))
	{
		/* Avoid overflows using 128-bit intermediate math */
		l = mult128 (a.num, b.denom);
		r = mult128 (b.num, a.denom);
		return cmp128 (l, r);
	}

	if (a.denom < 0)
		a.denom *= -1;
	if (b.denom < 0)
		b.denom *= -1;

	/* BUG: Possible overflow here..  Also, doesn't properly deal with
	 * reciprocal denominators.
	 */
	aa = a.num * a.denom;
	bb = b.num * b.denom;

	if (aa == bb)
		return 0;
	if (aa > bb)
		return 1;
	return -1;
}


/* *******************************************************************
 *  qof_numeric_eq
 ********************************************************************/

gboolean
qof_numeric_eq (QofNumeric a, QofNumeric b)
{
	return ((a.num == b.num) && (a.denom == b.denom));
}

/* *******************************************************************
 *  QofNumeric_equal
 ********************************************************************/

gboolean
qof_numeric_equal (QofNumeric a, QofNumeric b)
{
	QofInt128 l, r;

	if ((a.denom == b.denom) && (a.denom > 0))
		return (a.num == b.num);
	if ((a.denom > 0) && (b.denom > 0))
	{
		// return (a.num*b.denom == b.num*a.denom);
		l = mult128 (a.num, b.denom);
		r = mult128 (b.num, a.denom);
		return equal128 (l, r);

#if ALT_WAY_OF_CHECKING_EQUALITY
		QofNumeric ra = QofNumeric_reduce (a);
		QofNumeric rb = QofNumeric_reduce (b);
		if (ra.denom != rb.denom)
			return 0;
		if (ra.num != rb.num)
			return 0;
		return 1;
#endif
	}
	if ((a.denom < 0) && (b.denom < 0))
	{
		l = mult128 (a.num, -a.denom);
		r = mult128 (b.num, -b.denom);
		return equal128 (l, r);
	}
	else
	{
		/* BUG: One of the numbers has a reciprocal denom, and the other
		   does not. I just don't know to handle this case in any
		   reasonably overflow-proof yet simple way.  So, this funtion
		   will simply get it wrong whenever the three multiplies
		   overflow 64-bits.  -CAS */
		if (a.denom < 0)
			return ((a.num * -a.denom * b.denom) == b.num);
		else
			return (a.num == (b.num * a.denom * -b.denom));
		}
	return ((a.num * b.denom) == (a.denom * b.num));
}


/* *******************************************************************
 *  qof_numeric_same
 *  would a and b be equal() if they were both converted to the same 
 *  denominator? 
 ********************************************************************/

gint
qof_numeric_same (QofNumeric a, QofNumeric b, gint64 denom, gint how)
{
	QofNumeric aconv, bconv;

	aconv = qof_numeric_convert (a, denom, how);
	bconv = qof_numeric_convert (b, denom, how);

	return (qof_numeric_equal (aconv, bconv));
}

/* *******************************************************************
 *  qof_numeric_add
 ********************************************************************/

QofNumeric
qof_numeric_add (QofNumeric a, QofNumeric b, gint64 denom, gint how)
{
	QofNumeric sum;

	if (qof_numeric_check (a) || qof_numeric_check (b))
		return qof_numeric_error (QOF_ERROR_ARG);

	if ((denom == QOF_DENOM_AUTO) &&
		(how & QOF_NUMERIC_DENOM_MASK) == QOF_HOW_DENOM_FIXED)
	{
		if (a.denom == b.denom)
			denom = a.denom;
		else if (b.num == 0)
		{
			denom = a.denom;
			b.denom = a.denom;
		}
		else if (a.num == 0)
		{
			denom = b.denom;
			a.denom = b.denom;
		}
		else
			return qof_numeric_error (QOF_ERROR_DENOM_DIFF);
	}

	if (a.denom < 0)
	{
		a.num *= -a.denom;		/* BUG: overflow not handled.  */
		a.denom = 1;
	}

	if (b.denom < 0)
	{
		b.num *= -b.denom;		/* BUG: overflow not handled.  */
		b.denom = 1;
	}

	/* Get an exact answer.. same denominator is the common case. */
	if (a.denom == b.denom)
	{
		sum.num = a.num + b.num;	/* BUG: overflow not handled.  */
		sum.denom = a.denom;
	}
	else
	{
		/* We want to do this:
		 *    sum.num = a.num*b.denom + b.num*a.denom;
		 *    sum.denom = a.denom*b.denom;
		 * but the multiply could overflow.  
		 * Computing the LCD minimizes likelihood of overflow
		 */
		gint64 lcd;
		QofInt128 ca, cb, cab;

		lcd = qof_numeric_lcd (a, b);
		if (QOF_ERROR_ARG == lcd)
			return qof_numeric_error (QOF_ERROR_OVERFLOW);
		ca = mult128 (a.num, lcd / a.denom);
		if (ca.isbig)
			return qof_numeric_error (QOF_ERROR_OVERFLOW);
		cb = mult128 (b.num, lcd / b.denom);
		if (cb.isbig)
			return qof_numeric_error (QOF_ERROR_OVERFLOW);
		cab = add128 (ca, cb);
		if (cab.isbig)
			return qof_numeric_error (QOF_ERROR_OVERFLOW);
		sum.num = cab.lo;
		if (cab.isneg)
			sum.num = -sum.num;
		sum.denom = lcd;
	}

	if ((denom == QOF_DENOM_AUTO) &&
		((how & QOF_NUMERIC_DENOM_MASK) == QOF_HOW_DENOM_LCD))
	{
		denom = qof_numeric_lcd (a, b);
		how = how & QOF_NUMERIC_RND_MASK;
	}

	return qof_numeric_convert (sum, denom, how);
}

/* ***************************************************************
 *  qof_numeric_sub
 *****************************************************************/

QofNumeric
qof_numeric_sub (QofNumeric a, QofNumeric b, gint64 denom, gint how)
{
	QofNumeric nb;

	if (qof_numeric_check (a) || qof_numeric_check (b))
		return qof_numeric_error (QOF_ERROR_ARG);

	nb = b;
	nb.num = -nb.num;
	return qof_numeric_add (a, nb, denom, how);
}

/* ***************************************************************
 *  qof_numeric_mul
 *****************************************************************/

QofNumeric
qof_numeric_mul (QofNumeric a, QofNumeric b, gint64 denom, gint how)
{
	QofNumeric product, result;
	QofInt128 bignume, bigdeno;

	if (qof_numeric_check (a) || qof_numeric_check (b))
		return qof_numeric_error (QOF_ERROR_ARG);

	if ((denom == QOF_DENOM_AUTO) &&
		(how & QOF_NUMERIC_DENOM_MASK) == QOF_HOW_DENOM_FIXED)
	{
		if (a.denom == b.denom)
			denom = a.denom;
		else if (b.num == 0)
			denom = a.denom;
		else if (a.num == 0)
			denom = b.denom;
		else
			return qof_numeric_error (QOF_ERROR_DENOM_DIFF);
	}

	if ((denom == QOF_DENOM_AUTO) &&
		((how & QOF_NUMERIC_DENOM_MASK) == QOF_HOW_DENOM_LCD))
	{
		denom = qof_numeric_lcd (a, b);
		how = how & QOF_NUMERIC_RND_MASK;
	}

	if (a.denom < 0)
	{
		a.num *= -a.denom;		/* BUG: overflow not handled.  */
		a.denom = 1;
	}

	if (b.denom < 0)
	{
		b.num *= -b.denom;		/* BUG: overflow not handled.  */
		b.denom = 1;
	}

	bignume = mult128 (a.num, b.num);
	bigdeno = mult128 (a.denom, b.denom);
	product.num = a.num * b.num;
	product.denom = a.denom * b.denom;

	/* If it looks to be overflowing, try to reduce the fraction ... */
	if (bignume.isbig || bigdeno.isbig)
	{
		gint64 tmp;

		a = qof_numeric_reduce (a);
		b = qof_numeric_reduce (b);
		tmp = a.num;
		a.num = b.num;
		b.num = tmp;
		a = qof_numeric_reduce (a);
		b = qof_numeric_reduce (b);
		bignume = mult128 (a.num, b.num);
		bigdeno = mult128 (a.denom, b.denom);
		product.num = a.num * b.num;
		product.denom = a.denom * b.denom;
	}

	/* If it its still overflowing, and rounding is allowed then round */
	if (bignume.isbig || bigdeno.isbig)
	{
		/* If rounding allowed, then shift until there's no 
		 * more overflow. The conversion at the end will fix 
		 * things up for the final value. Else overflow. */
		if ((how & QOF_NUMERIC_RND_MASK) == QOF_HOW_RND_NEVER)
		{
			if (bigdeno.isbig)
				return qof_numeric_error (QOF_ERROR_OVERFLOW);
			product = reduce128 (bignume, product.denom);
			if (qof_numeric_check (product))
				return qof_numeric_error (QOF_ERROR_OVERFLOW);
		}
		else
		{
			while (bignume.isbig || bigdeno.isbig)
			{
				bignume = shift128 (bignume);
				bigdeno = shift128 (bigdeno);
			}
			product.num = bignume.lo;
			if (bignume.isneg)
				product.num = -product.num;

			product.denom = bigdeno.lo;
			if (0 == product.denom)
				return qof_numeric_error (QOF_ERROR_OVERFLOW);
		}
	}

#if 0							/* currently, product denom won't ever be zero */
	if (product.denom < 0)
	{
		product.num = -product.num;
		product.denom = -product.denom;
	}
#endif

	result = qof_numeric_convert (product, denom, how);
	return result;
}

/* *******************************************************************
 *  qof_numeric_div
 ********************************************************************/

QofNumeric
qof_numeric_div (QofNumeric a, QofNumeric b, gint64 denom, gint how)
{
	QofNumeric quotient;
	QofInt128 nume, deno;

	if (qof_numeric_check (a) || qof_numeric_check (b))
		return qof_numeric_error (QOF_ERROR_ARG);

	if ((denom == QOF_DENOM_AUTO) &&
		(how & QOF_NUMERIC_DENOM_MASK) == QOF_HOW_DENOM_FIXED)
	{
		if (a.denom == b.denom)
			denom = a.denom;
		else if (a.denom == 0)
			denom = b.denom;
		else
			return qof_numeric_error (QOF_ERROR_DENOM_DIFF);
		}

	if (a.denom < 0)
	{
		a.num *= -a.denom;		/* BUG: overflow not handled.  */
		a.denom = 1;
	}

	if (b.denom < 0)
	{
		b.num *= -b.denom;		/* BUG: overflow not handled.  */
		b.denom = 1;
	}

	if (a.denom == b.denom)
	{
		quotient.num = a.num;
		quotient.denom = b.num;
	}
	else
	{
		gint64 sgn = 1;
		if (0 > a.num)
		{
			sgn = -sgn;
			a.num = -a.num;
		}
		if (0 > b.num)
		{
			sgn = -sgn;
			b.num = -b.num;
		}
		nume = mult128 (a.num, b.denom);
		deno = mult128 (b.num, a.denom);

		/* Try to avoid overflow by removing common factors */
		if (nume.isbig && deno.isbig)
		{
			QofNumeric ra = qof_numeric_reduce (a);
			QofNumeric rb = qof_numeric_reduce (b);
			gint64 gcf_nume = gcf64 (ra.num, rb.num);
			gint64 gcf_deno = gcf64 (rb.denom, ra.denom);

			nume = mult128 (ra.num / gcf_nume, rb.denom / gcf_deno);
			deno = mult128 (rb.num / gcf_nume, ra.denom / gcf_deno);
		}

		if ((0 == nume.isbig) && (0 == deno.isbig))
		{
			quotient.num = sgn * nume.lo;
			quotient.denom = deno.lo;
			goto dive_done;
		}
		else if (0 == deno.isbig)
		{
			quotient = reduce128 (nume, deno.lo);
			if (0 == qof_numeric_check (quotient))
			{
				quotient.num *= sgn;
				goto dive_done;
			}
		}

		/* If rounding allowed, then shift until there's no 
		 * more overflow. The conversion at the end will fix 
		 * things up for the final value. */
		if ((how & QOF_NUMERIC_RND_MASK) == QOF_HOW_RND_NEVER)
			return qof_numeric_error (QOF_ERROR_OVERFLOW);
		while (nume.isbig || deno.isbig)
		{
			nume = shift128 (nume);
			deno = shift128 (deno);
		}
		quotient.num = sgn * nume.lo;
		quotient.denom = deno.lo;
		if (0 == quotient.denom)
		{
			return qof_numeric_error (QOF_ERROR_OVERFLOW);
		}
	}

	if (quotient.denom < 0)
	{
		quotient.num = -quotient.num;
		quotient.denom = -quotient.denom;
	}

  dive_done:
	if ((denom == QOF_DENOM_AUTO) &&
		((how & QOF_NUMERIC_DENOM_MASK) == QOF_HOW_DENOM_LCD))
	{
		denom = qof_numeric_lcd (a, b);
		how = how & QOF_NUMERIC_RND_MASK;
	}

	return qof_numeric_convert (quotient, denom, how);
}

/* *******************************************************************
 *  qof_numeric_neg
 *  negate the argument 
 ********************************************************************/

QofNumeric
qof_numeric_neg (QofNumeric a)
{
	if (qof_numeric_check (a))
		return qof_numeric_error (QOF_ERROR_ARG);
	return qof_numeric_create (-a.num, a.denom);
}

/* *******************************************************************
 *  qof_numeric_neg
 *  return the absolute value of the argument 
 ********************************************************************/

QofNumeric
qof_numeric_abs (QofNumeric a)
{
	if (qof_numeric_check (a))
		return qof_numeric_error (QOF_ERROR_ARG);
	return qof_numeric_create (ABS (a.num), a.denom);
}

/* *******************************************************************
 *  qof_numeric_convert
 ********************************************************************/

QofNumeric
qof_numeric_convert (QofNumeric in, gint64 denom, gint how)
{
	QofNumeric out;
	QofNumeric temp;
	gint64 temp_bc;
	gint64 temp_a;
	gint64 remainder;
	gint64 sign;
	gint denom_neg = 0;
	gdouble ratio, logratio;
	gdouble sigfigs;
	QofInt128 nume, newm;

	temp.num = 0;
	temp.denom = 0;

	if (qof_numeric_check (in))
		return qof_numeric_error (QOF_ERROR_ARG);

	if (denom == QOF_DENOM_AUTO)
	{
		switch (how & QOF_NUMERIC_DENOM_MASK)
		{
		default:
		case QOF_HOW_DENOM_LCD:	/* LCD is meaningless with AUTO in here */
		case QOF_HOW_DENOM_EXACT:
			return in;
			break;

		case QOF_HOW_DENOM_REDUCE:
			/* reduce the input to a relatively-prime fraction */
			return qof_numeric_reduce (in);
			break;

		case QOF_HOW_DENOM_FIXED:
			if (in.denom != denom)
				return qof_numeric_error (QOF_ERROR_DENOM_DIFF);
			else
				return in;
			break;

		case QOF_HOW_DENOM_SIGFIG:
			ratio = fabs (qof_numeric_to_double (in));
			if (ratio < 10e-20)
				logratio = 0;
			else
			{
				logratio = log10 (ratio);
				logratio = ((logratio > 0.0) ?
					(floor (logratio) + 1.0) : (ceil (logratio)));
			}
			sigfigs = QOF_HOW_GET_SIGFIGS (how);

			if (sigfigs - logratio >= 0)
				denom = (gint64) (pow (10, sigfigs - logratio));
			else
				denom = -((gint64) (pow (10, logratio - sigfigs)));

			how = how & ~QOF_HOW_DENOM_SIGFIG & ~QOF_NUMERIC_SIGFIGS_MASK;
			break;
		}
	}

	/* Make sure we need to do the work */
	if (in.denom == denom)
		return in;
	if (in.num == 0)
	{
		out.num = 0;
		out.denom = denom;
		return out;
	}

	/* If the denominator of the input value is negative, get rid of that. */
	if (in.denom < 0)
	{
		in.num = in.num * (-in.denom);	/* BUG: overflow not handled.  */
		in.denom = 1;
	}

	sign = (in.num < 0) ? -1 : 1;

	/* If the denominator is less than zero, we are to interpret it as 
	 * the reciprocal of its magnitude. */
	if (denom < 0)
	{

		/* XXX FIXME: use 128-bit math here ... */
		denom = -denom;
		denom_neg = 1;
		temp_a = (in.num < 0) ? -in.num : in.num;
		temp_bc = in.denom * denom;	/* BUG: overflow not handled.  */
		remainder = temp_a % temp_bc;
		out.num = temp_a / temp_bc;
		out.denom = -denom;
	}
	else
	{
		/* Do all the modulo and int division on positive values to make
		 * things a little clearer. Reduce the fraction denom/in.denom to
		 * help with range errors */
		temp.num = denom;
		temp.denom = in.denom;
		temp = qof_numeric_reduce (temp);

		/* Symbolically, do the following:
		 * out.num   = in.num * temp.num;
		 * remainder = out.num % temp.denom;
		 * out.num   = out.num / temp.denom;
		 * out.denom = denom;
		 */
		nume = mult128 (in.num, temp.num);
		newm = div128 (nume, temp.denom);
		remainder = rem128 (nume, temp.denom);

		if (newm.isbig)
			return qof_numeric_error (QOF_ERROR_OVERFLOW);

		out.num = newm.lo;
		out.denom = denom;
	}

	if (remainder)
	{
		switch (how & QOF_NUMERIC_RND_MASK)
		{
		case QOF_HOW_RND_FLOOR:
			if (sign < 0)
				out.num = out.num + 1;
			break;

		case QOF_HOW_RND_CEIL:
			if (sign > 0)
				out.num = out.num + 1;
			break;

		case QOF_HOW_RND_TRUNC:
			break;

		case QOF_HOW_RND_PROMOTE:
			out.num = out.num + 1;
			break;

		case QOF_HOW_RND_ROUND_HALF_DOWN:
			if (denom_neg)
			{
				if ((2 * remainder) > in.denom * denom)
					out.num = out.num + 1;
				}
			else if ((2 * remainder) > temp.denom)
				out.num = out.num + 1;
			/* check that 2*remainder didn't over-flow */
			else if (((2 * remainder) < remainder) &&
				(remainder > (temp.denom / 2)))
				out.num = out.num + 1;
			break;

		case QOF_HOW_RND_ROUND_HALF_UP:
			if (denom_neg)
			{
				if ((2 * remainder) >= in.denom * denom)
					out.num = out.num + 1;
				}
			else if ((2 * remainder) >= temp.denom)
				out.num = out.num + 1;
			/* check that 2*remainder didn't over-flow */
			else if (((2 * remainder) < remainder) &&
				(remainder >= (temp.denom / 2)))
				out.num = out.num + 1;
			break;

		case QOF_HOW_RND_ROUND:
			if (denom_neg)
			{
				if ((2 * remainder) > in.denom * denom)
					out.num = out.num + 1;
				else if ((2 * remainder) == in.denom * denom)
				{
					if (out.num % 2)
						out.num = out.num + 1;
					}
				}
			else
			{
				if ((2 * remainder) > temp.denom)
					out.num = out.num + 1;
				/* check that 2*remainder didn't over-flow */
				else if (((2 * remainder) < remainder) &&
					(remainder > (temp.denom / 2)))
				{
					out.num = out.num + 1;
				}
				else if ((2 * remainder) == temp.denom)
				{
					if (out.num % 2)
						out.num = out.num + 1;
					}
				/* check that 2*remainder didn't over-flow */
				else if (((2 * remainder) < remainder) &&
					(remainder == (temp.denom / 2)))
				{
					if (out.num % 2)
						out.num = out.num + 1;
					}
				}
			break;

		case QOF_HOW_RND_NEVER:
			return qof_numeric_error (QOF_ERROR_REMAINDER);
			break;
		}
	}

	out.num = (sign > 0) ? out.num : (-out.num);

	return out;
}

/* *************************************************************
reduce a fraction by GCF elimination.  This is NOT done as a
part of the arithmetic API unless QOF_HOW_DENOM_REDUCE is 
specified 
as the output denominator.
****************************************************************/

QofNumeric
qof_numeric_reduce (QofNumeric in)
{
	gint64 t;
	gint64 num = (in.num < 0) ? (-in.num) : in.num;
	gint64 denom = in.denom;
	QofNumeric out;

	if (qof_numeric_check (in))
		return qof_numeric_error (QOF_ERROR_ARG);

	/* The strategy is to use Euclid's algorithm */
	while (denom > 0)
	{
		t = num % denom;
		num = denom;
		denom = t;
	}
	/* num now holds the GCD (Greatest Common Divisor) */

	/* All calculations are done on positive num, since it's not 
	 * well defined what % does for negative values */
	out.num = in.num / num;
	out.denom = in.denom / num;
	return out;
}

/* ***************************************************************
 *  double_to_QofNumeric
 ****************************************************************/

QofNumeric
qof_numeric_from_double (gdouble in, gint64 denom, gint how)
{
	QofNumeric out;
	gint64 int_part = 0;
	gdouble frac_part;
	gint64 frac_int = 0;
	gdouble logval;
	gdouble sigfigs;

	if ((denom == QOF_DENOM_AUTO) && (how & QOF_HOW_DENOM_SIGFIG))
	{
		if (fabs (in) < 10e-20)
			logval = 0;
		else
		{
			logval = log10 (fabs (in));
			logval = ((logval > 0.0) ?
				(floor (logval) + 1.0) : (ceil (logval)));
		}
		sigfigs = QOF_HOW_GET_SIGFIGS (how);
		if (sigfigs - logval >= 0)
			denom = (gint64) (pow (10, sigfigs - logval));
		else
			denom = -((gint64) (pow (10, logval - sigfigs)));

		how = how & ~QOF_HOW_DENOM_SIGFIG & ~QOF_NUMERIC_SIGFIGS_MASK;
	}

	int_part = (gint64) (floor (fabs (in)));
	frac_part = in - (double) int_part;

	int_part = int_part * denom;
	frac_part = frac_part * (double) denom;

	switch (how & QOF_NUMERIC_RND_MASK)
	{
	case QOF_HOW_RND_FLOOR:
		frac_int = (gint64) floor (frac_part);
		break;

	case QOF_HOW_RND_CEIL:
		frac_int = (gint64) ceil (frac_part);
		break;

	case QOF_HOW_RND_TRUNC:
		frac_int = (gint64) frac_part;
		break;

	case QOF_HOW_RND_ROUND:
	case QOF_HOW_RND_ROUND_HALF_UP:
		frac_int = (gint64) rint (frac_part);
		break;

	case QOF_HOW_RND_NEVER:
		frac_int = (gint64) floor (frac_part);
		if (frac_part != (double) frac_int)
		{
			/* signal an error */
		}
		break;
	}

	out.num = int_part + frac_int;
	out.denom = denom;
	return out;
}

/* *******************************************************************
 *  qof_numeric_to_double
 ********************************************************************/

gdouble
qof_numeric_to_double (QofNumeric in)
{
	if (in.denom > 0)
		return (gdouble) in.num / (gdouble) in.denom;
	else
		return (gdouble) (in.num * -in.denom);
}

/* *******************************************************************
 *  qof_numeric_error
 ********************************************************************/

QofNumeric
qof_numeric_error (QofNumericErrorCode error_code)
{
	return qof_numeric_create (error_code, 0LL);
}

/* *******************************************************************
 *  qof_numeric_add_with_error
 ********************************************************************/

QofNumeric
qof_numeric_add_with_error (QofNumeric a, QofNumeric b,
	gint64 denom, gint how, QofNumeric * error)
{

	QofNumeric sum = qof_numeric_add (a, b, denom, how);
	QofNumeric exact = qof_numeric_add (a, b, QOF_DENOM_AUTO,
		QOF_HOW_DENOM_REDUCE);
	QofNumeric err = qof_numeric_sub (sum, exact, QOF_DENOM_AUTO,
		QOF_HOW_DENOM_REDUCE);

	if (error)
		*error = err;
	return sum;
}

/* *******************************************************************
 *  qof_numeric_sub_with_error
 ********************************************************************/

QofNumeric
qof_numeric_sub_with_error (QofNumeric a, QofNumeric b,
	gint64 denom, gint how, QofNumeric * error)
{
	QofNumeric diff = qof_numeric_sub (a, b, denom, how);
	QofNumeric exact = qof_numeric_sub (a, b, QOF_DENOM_AUTO,
		QOF_HOW_DENOM_REDUCE);
	QofNumeric err = qof_numeric_sub (diff, exact, QOF_DENOM_AUTO,
		QOF_HOW_DENOM_REDUCE);
	if (error)
		*error = err;
	return diff;
}

/* *******************************************************************
 *  qof_numeric_mul_with_error
 ********************************************************************/

QofNumeric
qof_numeric_mul_with_error (QofNumeric a, QofNumeric b,
	gint64 denom, gint how, QofNumeric * error)
{
	QofNumeric prod = qof_numeric_mul (a, b, denom, how);
	QofNumeric exact = qof_numeric_mul (a, b, QOF_DENOM_AUTO,
		QOF_HOW_DENOM_REDUCE);
	QofNumeric err = qof_numeric_sub (prod, exact, QOF_DENOM_AUTO,
		QOF_HOW_DENOM_REDUCE);
	if (error)
		*error = err;
	return prod;
}


/* *******************************************************************
 *  qof_numeric_div_with_error
 ********************************************************************/

QofNumeric
qof_numeric_div_with_error (QofNumeric a, QofNumeric b,
	gint64 denom, gint how, QofNumeric * error)
{
	QofNumeric quot = qof_numeric_div (a, b, denom, how);
	QofNumeric exact = qof_numeric_div (a, b, QOF_DENOM_AUTO,
		QOF_HOW_DENOM_REDUCE);
	QofNumeric err = qof_numeric_sub (quot, exact,
		QOF_DENOM_AUTO, QOF_HOW_DENOM_REDUCE);
	if (error)
		*error = err;
	return quot;
}

/* ***************************************************************
 *  QofNumeric text IO
 ****************************************************************/

gchar *
qof_numeric_to_string (QofNumeric n)
{
	gchar *result;
	gint64 tmpnum = n.num;
	gint64 tmpdenom = n.denom;

	result =
		g_strdup_printf ("%" G_GINT64_FORMAT "/%" G_GINT64_FORMAT, tmpnum,
		tmpdenom);

	return result;
}

gchar *
qof_numeric_dbg_to_string (QofNumeric n)
{
	static gchar buff[1000];
	static gchar *p = buff;
	gint64 tmpnum = n.num;
	gint64 tmpdenom = n.denom;

	p += 100;
	if (p - buff >= 1000)
		p = buff;

	sprintf (p, "%" G_GINT64_FORMAT "/%" G_GINT64_FORMAT, tmpnum,
		tmpdenom);

	return p;
}

gboolean
qof_numeric_from_string (const gchar * str, QofNumeric * n)
{
	size_t G_GNUC_UNUSED num_read;
	gint64 tmpnum;
	gint64 tmpdenom;

	if (!str)
		return FALSE;
	tmpnum = strtoll (str, NULL, 0);
	str = strchr (str, '/');
	if (!str)
		return FALSE;
	str++;
	tmpdenom = strtoll (str, NULL, 0);
	num_read = strspn (str, "0123456789");
	n->num = tmpnum;
	n->denom = tmpdenom;
	return TRUE;
}

/* ======================== END OF FILE =================== */
