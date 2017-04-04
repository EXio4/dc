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

	std::shared_ptr<dc_array> cur = get_stacked_array(array_id);
    (*cur)[Index] = value;
}

/* retrieve a dup of a value from array_id[Index] */
/* A zero value is returned if the specified value is unintialized. */
dc_data
DC::dc_array_get (
	
	int array_id ,
	int Index )
{
    std::shared_ptr<dc_array> cur = get_stacked_array(array_id);

    auto x = cur->find(Index);
    if (x != cur->end()) {
        return dup(x->second);
    } else {
        return int2data(0);
    }
}
