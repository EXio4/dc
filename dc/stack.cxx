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

/* allocate a new dc_list item */
dc_list *
DC::alloc DC_DECLVOID()
{
	dc_list *result;
	
	result = new dc_list;
	result->array = NULL;
	result->link = NULL;
	return result;
}


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
DC::register_init DC_DECLVOID()
{
	int i;

	for (i=0; i<DC_REGCOUNT; ++i)
		registers[i] = NULL;
}

/* clear the evaluation stack */
void
DC::clear_stack ()
{
	dc_list *n;
	dc_list *t;

	for (n=stack; n; n=t){
		t = n->link;
		p_visit<void>(n->value, FreeVar(*this));
		dc_array_free(n->array);
		free(n);
	}
	stack = NULL;
}

/* push a value onto the evaluation stack */
void
DC::push (
	
	dc_data value )
{
	dc_list *n = alloc();
	n->value = value;
	n->link = stack;
	stack = n;
}

/* push a value onto the named register stack */
void
DC::register_push (
	int stackid ,
	dc_data value )
{
	dc_list *n = alloc();

	stackid = regmap(stackid);
	n->value = value;
	n->link = registers[stackid];
	registers[stackid] = n;
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
	if (!stack){
		throw DC_Exc("Empty Stack");
	}
	*result = stack->value;
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
	dc_list *r;

	regid = regmap(regid);
	r = registers[regid];
	if ( ! r ){
		throw DC_Exc("Empty Register");
	}
	*result = dup(r->value);
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
	dc_list *r;

	regid = regmap(regid);
	r = registers[regid];
	if ( ! r )
		registers[regid] = alloc();
	else {
		p_visit<void>(r->value, FreeVar(*this));

	}
	registers[regid]->value = value;
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
	dc_list *r;

	r = stack;
	if (!r){
		throw DC_Exc("Empty Stack");
	}
	*result = r->value;
	stack = r->link;
	dc_array_free(r->array);
	delete r;
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
	dc_list *r;

	stackid = regmap(stackid);
	r = registers[stackid];
	if (!r){
		throw DC_Exc("Empty Register");
	}
	*result = r->value;
	registers[stackid] = r->link;
	dc_array_free(r->array);
	free(r);
	return DC_SUCCESS;
}


/* tell how many entries are currently on the evaluation stack */
int
DC::tell_stackdepth DC_DECLVOID()
{
	dc_list *n;
	int depth=0;

	for (n=stack; n; n=n->link)
		++depth;
	return depth;
}


/* return the length of the indicated data value;
 * if discard_p is DC_TOSS, the deallocate the value when done
 *
 * The definition of a datum's length is deligated to the
 * appropriate module.
 */
int
DC::tell_length (
	
	dc_data value ,
	dc_discard discard_p )
{
	int length = 0;
	p_match<void>(value,
				  [this,&length,discard_p](dc_num n) {		
					length = dc_numlen(n);
					if (discard_p == DC_TOSS)
						free_num(&n);
				  },
   			      [this,&length,discard_p](dc_str s) {		
					  length = dc_strlen(s);
					  if (discard_p == DC_TOSS)
						  free_str(&s);
				  });
	return length;
}



/* print out all of the values on the evaluation stack */
void
DC::printall (
	
	int obase )
{
	dc_list *n;

	for (n=stack; n; n=n->link)
		print(n->value, obase, DC_WITHNL, DC_KEEP);
}




/* get the current array head for the named array */
struct dc_array *
DC::get_stacked_array (
	int array_id )
{
	dc_list *r = registers[regmap(array_id)];
	return r ? r->array : NULL;
}

/* set the current array head for the named array */
void
DC::set_stacked_array (
	int array_id ,
	struct dc_array *new_head )
{
	dc_list *r;

	array_id = regmap(array_id);
	r = registers[array_id];
	if ( ! r )
		r = registers[array_id] = alloc();
	r->array = new_head;
}
