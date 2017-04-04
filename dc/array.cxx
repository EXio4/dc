/* 
 * implement arrays for dc
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

/* This module is the only one that knows what arrays look like. */

#include "config.h"

#include <stdio.h>	/* "dc-proto.h" wants this */
#ifdef HAVE_STDLIB_H
/* get size_t definition from "almost ANSI" compiling environments. */
#include <stdlib.h>
#endif
#include "dc.h"
#include "dc-proto.h"
#include "dc-regdef.h"
#include "visitors.h"

/* initialize the arrays */
void
DC::dc_array_init DC_DECLVOID()
{
}

/* store value into array_id[Index] */
void
DC::dc_array_set (
	
	int array_id ,
	int Index ,
	dc_data value )
{
	struct dc_array *cur;
	struct dc_array *prev=NULL;
	struct dc_array *newentry;

	cur = get_stacked_array(array_id);
	while (cur && cur->Index < Index){
		prev = cur;
		cur = cur->next;
	}
	if (cur && cur->Index == Index){
		p_visit<void>(cur->value, FreeVar(*this));
		cur->value = value;
	}else{
		newentry = new dc_array;
		newentry->Index = Index;
		newentry->value = value;
		newentry->next = cur;
		if (prev)
			prev->next = newentry;
		else
			set_stacked_array(array_id, newentry);
	}
}

/* retrieve a dup of a value from array_id[Index] */
/* A zero value is returned if the specified value is unintialized. */
dc_data
DC::dc_array_get (
	
	int array_id ,
	int Index )
{
	struct dc_array *cur;

	for (cur=get_stacked_array(array_id); cur; cur=cur->next)
		if (cur->Index == Index)
			return dup(cur->value);
	return int2data(0);
}

/* free an array chain */
void
DC::dc_array_free (
	
	struct dc_array *a_head )
{
	struct dc_array *cur;
	struct dc_array *next;

	for (cur=a_head; cur; cur=next) {
		next = cur->next;
		p_visit<void>(cur->value, FreeVar(*this));
		delete cur;
	}
}
