/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "undo.h"
#include "paragraph.h"


Undo::Undo(undo_kind kind_arg, int id_inset_arg,
	   int number_before_arg, int number_behind_arg,
	   int cursor_par_arg, int cursor_pos_arg,
	   Paragraph * par_arg)
{
	kind = kind_arg;
	number_of_inset_id = id_inset_arg;
	number_of_before_par = number_before_arg;
	number_of_behind_par = number_behind_arg;
	number_of_cursor_par = cursor_par_arg;
	cursor_pos = cursor_pos_arg;
	par = par_arg;
}


Undo::~Undo()
{
	Paragraph * tmppar;
	while (par) {
		tmppar = par;
		par = par->next();
		delete tmppar;
	}
}


