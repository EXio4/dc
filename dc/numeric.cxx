/* 
 * interface dc to the bc numeric routines
 *
 * Copyright (C) 1994, 1997, 1998, 2000 Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can either send email to this
 * program's author (see below) or write to:
 *   The Free Software Foundation, Inc.
 *   59 Temple Place, Suite 330
 *   Boston, MA 02111 USA
 */

/* This should be the only module that knows the internals of type dc_num */
/* In this particular implementation we just slather out some glue and
 * make use of bc's numeric routines.
 */

#include "config.h"

#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include "number.h"
#include "dc.h"
#include "dc-proto.h"

#ifdef __GNUC__
# if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__-0 >= 7) 
#  define ATTRIB(x) __attribute__(x)
# endif
#endif
#ifndef ATTRIB
# define ATTRIB(x)
#endif

/* Forward prototype */
static void out_char (int);

/* there is no POSIX standard for dc, so we'll take the GNU definitions */
int std_only = FALSE;

/* convert an opaque dc_num into a real bc_num */
#define CastNum(x)	((bc_num)(x.d))

/* add two dc_nums, place into *result;
 * return DC_SUCCESS on success, DC_DOMAIN_ERROR on domain error
 */
int
DC::add (
	
	dc_num a ,
	dc_num b ,
	int kscale ATTRIB((unused)) ,
	dc_num &result )
{
	bc.init_num((bc_num *)&result);
	bc.add(CastNum(a), CastNum(b), (bc_num *)&result, 0);
	return DC_SUCCESS;
}

/* subtract two dc_nums, place into *result;
 * return DC_SUCCESS on success, DC_DOMAIN_ERROR on domain error
 */
int
DC::sub (
	
	dc_num a ,
	dc_num b ,
	int kscale ATTRIB((unused)) ,
	dc_num &result )
{
	bc.init_num((bc_num *)&result);
	bc.sub(CastNum(a), CastNum(b), (bc_num *)&result, 0);
	return DC_SUCCESS;
}

/* multiply two dc_nums, place into *result;
 * return DC_SUCCESS on success, DC_DOMAIN_ERROR on domain error
 */
int
DC::mul (
	
	dc_num a ,
	dc_num b ,
	int kscale ,
	dc_num &result )
{
	bc.init_num((bc_num *)&result);
	bc.multiply(CastNum(a), CastNum(b), (bc_num *)&result, kscale);
	return DC_SUCCESS;
}

/* divide two dc_nums, place into *result;
 * return DC_SUCCESS on success, DC_DOMAIN_ERROR on domain error
 */
int
DC::div (
	
	dc_num a ,
	dc_num b ,
	int kscale ,
	dc_num &result )
{
	bc.init_num((bc_num *)&result);
	if (bc.divide(CastNum(a), CastNum(b), (bc_num *)&result, kscale)){
		out << "divide by zero" << std::endl;
		return DC_DOMAIN_ERROR;
	}
	return DC_SUCCESS;
}

/* divide two dc_nums, place quotient into *quotient and remainder
 * into *remainder;
 * return DC_SUCCESS on success, DC_DOMAIN_ERROR on domain error
 */
int
DC::divrem (
	
	dc_num a ,
	dc_num b ,
	int kscale ,
	dc_num &quotient ,
	dc_num &remainder )
{
	bc.init_num((bc_num *)&quotient);
	bc.init_num((bc_num *)&remainder);
	if (bc.divmod(CastNum(a), CastNum(b),
				  (bc_num *)&quotient, (bc_num *)&remainder, kscale)){
		out << "divide by zero" << std::endl;
		return DC_DOMAIN_ERROR;
	}
	return DC_SUCCESS;
}

/* place the reminder of dividing a by b into *result;
 * return DC_SUCCESS on success, DC_DOMAIN_ERROR on domain error
 */
int
DC::rem (
	
	dc_num a ,
	dc_num b ,
	int kscale ,
	dc_num &result )
{
	bc.init_num((bc_num *)&result);
	if (bc.modulo(CastNum(a), CastNum(b), (bc_num *)&result, kscale)){
		out << "remainder by zero" << std::endl;
		return DC_DOMAIN_ERROR;
	}
	return DC_SUCCESS;
}

int
DC::modexp (
	
	dc_num base ,
	dc_num expo ,
	dc_num mod ,
	int kscale ,
	dc_num &result )
{
	bc.init_num((bc_num *)&result);
	if (bc.raisemod(out, CastNum(base), CastNum(expo), CastNum(mod),
					(bc_num *)&result, kscale)){
		if (bc.is_zero(CastNum(mod)))
			out << "remainder by zero" << std::endl;
		return DC_DOMAIN_ERROR;
	}
	return DC_SUCCESS;
}

/* place the result of exponentiationg a by b into *result;
 * return DC_SUCCESS on success, DC_DOMAIN_ERROR on domain error
 */
int
DC::exp (
	
	dc_num a ,
	dc_num b ,
	int kscale ,
	dc_num &result )
{
	bc.init_num((bc_num *)&result);
	bc.raise(out, CastNum(a), CastNum(b), (bc_num *)&result, kscale);
	return DC_SUCCESS;
}

/* take the square root of the value, place into *result;
 * return DC_SUCCESS on success, DC_DOMAIN_ERROR on domain error
 */
int
DC::sqrt (
	
	dc_num value ,
	int kscale ,
	dc_num& result )
{
	bc_num tmp;
	tmp = bc.copy_num(CastNum(value));
	if (!bc.sqrt(out, (bc_num *)&tmp, kscale)){
		out << "square root of negative number" << std::endl;
		bc.free_num(&tmp);
		return DC_DOMAIN_ERROR;
	}
	result = dc_num(tmp);
	return DC_SUCCESS;
}

/* compare dc_nums a and b;
 *  return a negative value if a < b;
 *  return a positive value if a > b;
 *  return zero value if a == b
 */
int
DC::compare (
	dc_num a ,
	dc_num b )
{
	return bc.compare(CastNum(a), CastNum(b));
}

/* attempt to convert a dc_num to its corresponding int value
 * If discard_p is DC_TOSS then deallocate the value after use.
 */
int
DC::dc_num2int (
	dc_num value ,
	dc_discard discard_p )
{
	long result;

	result = bc.num2long(CastNum(value));
	if (discard_p == DC_TOSS)
		free_num(&value);
	return (int)result;
}

/* convert a C integer value into a dc_num */
/* For convenience of the caller, package the dc_num
 * into a dc_data result.
 */
dc_data
DC::int2data (
	int value )
{
	bc_num n;
	
	bc.init_num((bc_num *)&n);
	bc.int2num((bc_num *)&n, value);
	return dc_num(n);
}
	

/* get a dc_num from some input stream;
 *  input is a function which knows how to read the desired input stream
 *  ibase is the input base (2<=ibase<=DC_IBASE_MAX)
 *  *readahead will be set to the readahead character consumed while
 *   looking for the end-of-number
 */
/* For convenience of the caller, package the dc_num
 * into a dc_data result.
 */
dc_data
DC::getnum (
	std::function<int(void)> input ,
	int ibase ,
	int *readahead )
{
	bc_num	base;
	bc_num	result;
	bc_num	build;
	bc_num	tmp;
	bc_num	divisor;
	int		negative = 0;
	int		digit;
	int		decimal;
	int		c;

	bc.init_num(&tmp);
	bc.init_num(&build);
	bc.init_num(&base);
	result = bc.copy_num(bc._zero_);
	bc.int2num(&base, ibase);
	c = (input)();
	while (isspace(c))
		c = (input)();
	if (c == '_' || c == '-'){
		negative = c;
		c = (input)();
	}else if (c == '+'){
		c = (input)();
	}
	while (isspace(c))
		c = (input)();
	for (;;){
		if (isdigit(c))
			digit = c - '0';
		else if ('A' <= c && c <= 'F')
			digit = 10 + c - 'A';
		else
			break;
		c = (input)();
		bc.int2num(&tmp, digit);
		bc.multiply(result, base, &result, 0);
		bc.add(result, tmp, &result, 0);
	}
	if (c == '.'){
		bc.free_num(&build);
		bc.free_num(&tmp);
		divisor = bc.copy_num(bc._one_);
		build = bc.copy_num(bc._zero_);
		decimal = 0;
		for (;;){
			c = (input)();
			if (isdigit(c))
				digit = c - '0';
			else if ('A' <= c && c <= 'F')
				digit = 10 + c - 'A';
			else
				break;
			bc.int2num(&tmp, digit);
			bc.multiply(build, base, &build, 0);
			bc.add(build, tmp, &build, 0);
			bc.multiply(divisor, base, &divisor, 0);
			++decimal;
		}
		bc.divide(build, divisor, &build, decimal);
		bc.add(result, build, &result, 0);
	}
	/* Final work. */
	if (negative)
		bc.sub(bc._zero_, result, &result, 0);

	bc.free_num(&tmp);
	bc.free_num(&build);
	bc.free_num(&base);
	if (readahead)
		*readahead = c;
	
	dc_data full_result = dc_num(result);
	return full_result;
}


/* return the "length" of the number */
int
DC::dc_numlen (
	dc_num value )
{
	bc_num num = CastNum(value);

	/* is this right??? */
	return num->n_len + num->n_scale - (*num->n_value == '\0');
}

/* return the scale factor of the passed dc_num
 * If discard_p is DC_TOSS then deallocate the value after use.
 */
int
DC::tell_scale (
	dc_num value ,
	dc_discard discard_p )
{
	int kscale;

	kscale = CastNum(value)->n_scale;
	if (discard_p == DC_TOSS)
		free_num(&value);
	return kscale;
}

/* print out a dc_num in output base obase to stdout;
 * if newline_p is DC_WITHNL, terminate output with a '\n';
 * if discard_p is DC_TOSS then deallocate the value after use
 */
void
DC::out_num (
	
	dc_num value ,
	int obase ,
	dc_newline newline_p ,
	dc_discard discard_p )
{
	std::string output;
	bc.out_num(CastNum(value), obase, output, 0);
	out << output;
	if (newline_p == DC_WITHNL) { out << std::endl; }
	if (discard_p == DC_TOSS)
		free_num(&value);
}

/* dump out the absolute value of the integer part of a
 * dc_num as a byte stream, without any line wrapping;
 * if discard_p is DC_TOSS then deallocate the value after use
 */
void
DC::dump_num (
	
	dc_num dcvalue ,
	dc_discard discard_p )
{
	std::string output;
	struct digit_stack { int digit; struct digit_stack *link;};
	struct digit_stack *top_of_stack = NULL;
	struct digit_stack *cur;
	struct digit_stack *next;
	bc_num value;
	bc_num obase;
	bc_num digit;

	bc.init_num(&value);
	bc.init_num(&obase);
	bc.init_num(&digit);

	/* we only handle the integer portion: */
	bc.divide(CastNum(dcvalue), bc._one_, &value, 0);
	/* we only handle the absolute value: */
	value->n_sign = PLUS;
	/* we're done with the dcvalue parameter: */
	if (discard_p == DC_TOSS)
		free_num(&dcvalue);

	bc.int2num(&obase, 1+UCHAR_MAX);
	do {
		(void) bc.divmod(value, obase, &value, &digit, 0);
		cur = (digit_stack*)malloc(sizeof *cur);
		cur->digit = (int)bc.num2long(digit);
		cur->link = top_of_stack;
		top_of_stack = cur;
	} while (!bc.is_zero(value));

	for (cur=top_of_stack; cur; cur=next) {
		output.push_back(cur->digit);
		next = cur->link;
		free(cur);
	}
	out << output;
	bc.free_num(&digit);
	bc.free_num(&obase);
	bc.free_num(&value);
}

/* deallocate an instance of a dc_num */
void
DC::free_num (
	dc_num *value )
{
	bc.free_num((bc_num *)value);
}

/* return a duplicate of the number in the passed value */
/* The mismatched data types forces the caller to deal with
 * bad type'd dc_data values, and makes it more convenient
 * for the caller to not have to do the grunge work of setting
 * up a type result.
 */
dc_data
DC::dup_num (
	dc_num value )
{
	dc_data result;

	++CastNum(value)->n_refs;
	result = value;
	return result;
}



/*---------------------------------------------------------------------------\
| The rest of this file consists of stubs for bc routines called by numeric.c|
| so as to minimize the amount of bc code needed to build dc.                |
| The bulk of the code was just lifted straight out of the bc source.        |
\---------------------------------------------------------------------------*/

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

#ifdef HAVE_STDARG_H
# include <stdarg.h>
#else
# include <varargs.h>
#endif


int out_col = 0;

/* Output routines: Write a character CH to the standard output.
   It keeps track of the number of characters output and may
   break the output with a "\<cr>". */

static void out_char (int ch)
{

  if (ch == '\0')
    {
      out_col = 0;
    }
  else
    {
      out_col++;
      if (out_col == 70)
	{
	  putchar ('\\');
	  putchar ('\n');
	  out_col = 1;
	}
      putchar (ch);
    }
}

/* Malloc could not get enough memory. */

void
out_of_memory()
{
  memfail();
}

