/* 
 * implement string functions for dc
 *
 * Copyright (C) 1994, 1997, 1998 Free Software Foundation, Inc.
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
 *
 *    The Free Software Foundation, Inc.
 *    59 Temple Place, Suite 330
 *    Boston, MA 02111 USA
 */

/* This should be the only module that knows the internals of type dc_string */

#include "config.h"

#include <stdio.h>
#ifdef HAVE_STDDEF_H
# include <stddef.h>	/* ptrdiff_t */
#else
# define ptrdiff_t	size_t
#endif
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
# include <string.h>	/* memcpy */
#else
# ifdef HAVE_MEMORY_H
#  include <memory.h>	/* memcpy, maybe */
# else
#  ifdef HAVE_STRINGS_H
#   include <strings.h>	/* memcpy, maybe */
#  endif
# endif
#endif
#include "dc.h"
#include "dc-proto.h"


/* return a duplicate of the string in the passed value */
/* The mismatched data types forces the caller to deal with
 * bad type'd dc_data values, and makes it more convenient
 * for the caller to not have to do the grunge work of setting
 * up a type result.
 */
dc_data
DC::dup_str (
	dc_str value )
{
	dc_data result;

	++value.s->s_refs;
	result = value;
	return result;
}

/* free an instance of a dc_str value */
void
DC::free_str (
	dc_str *value )
{
	struct dc_string *string = value->s;

	if (--string->s_refs < 1){
		free(string->s_ptr);
		free(string);
	}
}

/* Output a dc_str value.
 * Add a trailing newline if "newline" is set.
 * Free the value after use if discard_flag is set.
 */
void
DC::out_str (
	dc_str value ,
	dc_newline newline ,
	dc_discard discard_flag )
{
	out << std::string(value.s->s_ptr, value.s->s_len);
	if (newline == DC_WITHNL) {
		out << std::endl;
	}
	if (discard_flag == DC_TOSS)
		free_str(&value);
}

/* make a copy of a string (base s, length len)
 * into a dc_str value; return a dc_data result
 * with this value
 */
dc_data
DC::makestring (
	const char *s ,
	size_t len )
{
	dc_data result;
	struct dc_string *string;

	string = (dc_string*)malloc(sizeof *string);
	string->s_ptr = (char*)malloc(len+1);
	memcpy(string->s_ptr, s, len);
	string->s_ptr[len] = '\0';	/* nul terminated for those who need it */
	string->s_len = len;
	string->s_refs = 1;
	result = dc_str(string);
	return result;
}

/* read a dc_str value from FILE *fp;
 * if ldelim == rdelim, then read until a ldelim char or EOF is reached;
 * if ldelim != rdelim, then read until a matching rdelim for the
 * (already eaten) first ldelim is read.
 * Return a dc_data result with the dc_str value as its contents.
 */
dc_data
DC::readstring (
	FILE *fp ,
	int ldelim ,
	int rdelim )
{
	static char *line_buf = NULL;	/* a buffer to build the string in */ 
	static size_t buflen = 0;		/* the current size of line_buf */
	int depth=1;
	int c;
	char *p;
	const char *end;

	if (!line_buf){
		/* initial buflen should be large enough to handle most cases */
		buflen = 2016;
		line_buf = (char*)malloc(buflen);
	}
	p = line_buf;
	end = line_buf + buflen;
	for (;;){
		c = getc(fp);
		if (c == EOF)
			break;
		else if (c == rdelim && --depth < 1)
			break;
		else if (c == ldelim)
			++depth;
		if (p >= end){
			ptrdiff_t offset = p - line_buf;
			/* buflen increment should be big enough
			 * to avoid execessive reallocs:
			 */
			buflen += 2048;
			line_buf = (char*)realloc(line_buf, buflen);
			if (!line_buf)
				memfail();
			p = line_buf + offset;
			end = line_buf + buflen;
		}
		*p++ = c;
	}
	return makestring(line_buf, (size_t)(p-line_buf));
}

/* return the base pointer of the dc_str value;
 * This function is needed because no one else knows what dc_str
 * looks like.
 */
const char *
DC::dc_str2charp (
	dc_str value )
{
	return value.s->s_ptr;
}

/* return the length of the dc_str value;
 * This function is needed because no one else knows what dc_str
 * looks like, and strlen(dc_str2charp(value)) won't work
 * if there's an embedded '\0'.
 */
size_t
DC::dc_strlen (
	dc_str value )
{
	return value.s->s_len;
}


/* initialize the strings subsystem */
void
DC::dc_string_init DC_DECLVOID()
{
	/* nothing to do for this implementation */
}
