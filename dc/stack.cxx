/* 
 * implement stack functions for dc
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

/* This module is the only one that knows what stacks (both the
 * regular evaluation stack and the named register stacks)
 * look like.
 */

#include "config.h"

#include <stdio.h>
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#include "dc.h"
#include "dc-proto.h"
#include "dc-regdef.h"

#include "visitors.h"


/* check that there are two numbers on top of the stack,
 * then call op with the popped numbers.  Construct a dc_data
 * value from the dc_num returned by op and push it
 * on the stack.
 * If the op call doesn't return DC_SUCCESS, then leave the stack
 * unmodified.
 */
void
DC::binop (
	
	std::function<int(dc_num, dc_num, int, dc_num&)> op ,
	int kscale )
{
	dc_data a;
	dc_data b;
	dc_num r;

	(void)pop(&b);
	(void)pop(&a);
	
	dc_num na = dc_dget<dc_num>(a);
	dc_num nb = dc_dget<dc_num>(b);
	
	if ((op)(na, nb, kscale, r) == DC_SUCCESS){
		push(r);
		free_num(&na);
		free_num(&nb);
	}else{
		/* op failed; restore the stack */
		push(a);
		push(b);
	}
}

/* check that there are two numbers on top of the stack,
 * then call op with the popped numbers.  Construct two dc_data
 * values from the dc_num's returned by op and push them
 * on the stack.
 * If the op call doesn't return DC_SUCCESS, then leave the stack
 * unmodified.
 */
void
DC::binop2 (
	
	std::function<int(dc_num, dc_num, int, dc_num &, dc_num &)> op ,
	int kscale )
{
	dc_data a;
	dc_data b;
	dc_num r1;
	dc_num r2;

	(void)pop(&b);
	(void)pop(&a);
	
	dc_num na = dc_dget<dc_num>(a);
	dc_num nb = dc_dget<dc_num>(b);
	
	if ((op)(na, nb, kscale, r1, r2) == DC_SUCCESS){
		push(r1);
		push(r2);
		free_num(&na);
		free_num(&nb);
	}else{
		/* op failed; restore the stack */
		push(a);
		push(b);
	}
}

/* check that there are two numbers on top of the stack,
 * then call compare with the popped numbers.
 * Return negative, zero, or positive based on the ordering
 * of the two numbers.
 */
int
DC::cmpop ()
{
	int result;
	dc_data a;
	dc_data b;

	(void)pop(&b);
	(void)pop(&a);
	
	dc_num na = dc_dget<dc_num>(a);
	dc_num nb = dc_dget<dc_num>(b);
	
	result = compare(na, nb);
	free_num(&na);
	free_num(&nb);
	return result;
}

/* check that there are three numbers on top of the stack,
 * then call op with the popped numbers.  Construct a dc_data
 * value from the dc_num returned by op and push it
 * on the stack.
 * If the op call doesn't return DC_SUCCESS, then leave the stack
 * unmodified.
 */
void
DC::triop (
	
	std::function<int(dc_num, dc_num, dc_num, int, dc_num &)> op ,
	int kscale )
{
	dc_data a;
	dc_data b;
	dc_data c;
	dc_num r;

	(void)pop(&c);
	(void)pop(&b);
	(void)pop(&a);
	
	dc_num na = dc_dget<dc_num>(a);
	dc_num nb = dc_dget<dc_num>(b);
	dc_num nc = dc_dget<dc_num>(c);
	
	if ((op)(na, nb, nc, kscale, r) == DC_SUCCESS){
		push(r);
		free_num(&na);
		free_num(&nb);
		free_num(&nc);
	}else{
		/* op failed; restore the stack */
		push(a);
		push(b);
		push(c);
	}
}


/* initialize the register stacks to their initial values */
void
DC::register_init ()
{
}

/* clear the evaluation stack */
void
DC::clear_stack ()
{
	for (dc_node n : stack){
		p_visit<void>(n.value, FreeVar(*this));
    }
	stack.clear();
}

/* push a value onto the evaluation stack */
void
DC::push (
	
	dc_data value )
{
    dc_node s;
    s.value = value;
    stack.push_front(s);
}

/* push a value onto the named register stack */
void
DC::register_push (
	int stackid ,
	dc_data value )
{
	dc_node s;

	stackid = regmap(stackid);
    s.value = value;
	registers[stackid].push_front(s);
}

/* set *result to the value on the top of the evaluation stack */
/* The caller is responsible for duplicating the value if it
 * is to be maintained as anything more than a transient identity.
 *
 * DC_FAIL is returned if the stack is empty (and *result unchanged),
 * DC_SUCCESS is returned otherwise
 */
int
DC::top_of_stack (
	
	dc_data *result )
{
	if (stack.empty()){
		throw DC_Exc("Empty Stack");
	}
	*result = stack.front().value;
	return DC_SUCCESS;
}

/* set *result to a dup of the value on the top of the named register stack */
/*
 * DC_FAIL is returned if the named stack is empty (and *result unchanged),
 * DC_SUCCESS is returned otherwise
 */
int
DC::register_get (
	
	int regid ,
	dc_data *result )
{
	regid = regmap(regid);
	if (registers[regid].empty()){
		throw DC_Exc("Empty Register");
	}
	*result = dup(registers[regid].front().value);
	return DC_SUCCESS;
}

/* set the top of the named register stack to the indicated value */
/* If the named stack is empty, craft a stack entry to enter the
 * value into.
 */
void
DC::register_set (
	
	int regid ,
	dc_data value )
{
    regid = regmap(regid);
    std::list<dc_node>& r = registers[regid];
	if ( r.empty() )
		r.push_front(dc_node());
	else {
		p_visit<void>(r.front().value, FreeVar(*this));

	}
	r.front().value = value;
}

/* pop from the evaluation stack
 *
 * DC_FAIL is returned if the stack is empty (and *result unchanged),
 * DC_SUCCESS is returned otherwise
 */
int
DC::pop (
	
	dc_data *result )
{
	if (stack.empty()){
		throw DC_Exc("Empty Stack");
	}
	*result = stack.front().value;
    stack.pop_front();
	return DC_SUCCESS;
}

/* pop from the named register stack
 *
 * DC_FAIL is returned if the named stack is empty (and *result unchanged),
 * DC_SUCCESS is returned otherwise
 */
int
DC::register_pop (
	
	int stackid ,
	dc_data *result )
{
	stackid = regmap(stackid);
	if (registers[stackid].empty()){
		throw DC_Exc("Empty Register");
	}
	*result = registers[stackid].front().value;

	registers[stackid].pop_front();
	return DC_SUCCESS;
}


/* tell how many entries are currently on the evaluation stack */
int
DC::tell_stackdepth ()
{
	return stack.size();
}


/* return the length of the indicated data value;
 * if discard_p is DC_TOSS, the deallocate the value when done
 *
 * The definition of a datum's length is deligated to the
 * appropriate module.
 */
int
DC::tell_length (dc_data value )
{
	int length = 0;
	p_match<void>(value,
				  [this,&length](dc_num n) {		
					length = dc_numlen(n);
				  },
   			      [this,&length](dc_str s) {		
					  length = dc_strlen(s);
				  });
	return length;
}



/* print out all of the values on the evaluation stack */
void
DC::printall (
	
	int obase )
{
	for (dc_node n : stack)
		print(n.value, obase, DC_WITHNL);
}




/* get the current array head for the named array */
std::shared_ptr<dc_array>
DC::get_stacked_array (
	int array_id )
{
	std::list<dc_node>& r = registers[regmap(array_id)];
	return r.empty() ? NULL : r.front().array;
}

/* set the current array head for the named array */
void
DC::set_stacked_array (
	int array_id ,
	std::shared_ptr<dc_array> new_head )
{
	array_id = regmap(array_id);
	std::list<dc_node>& r = registers[array_id];
	if ( r.empty() )
		r.push_front(dc_node());
	r.front().array = new_head;
}
