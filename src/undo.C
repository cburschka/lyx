/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include "undo.h"

#ifdef __GNUG__
#pragma implementation
#endif


Undo::Undo(undo_kind kind_arg,
	   int number_before_arg, int number_behind_arg,
	   int cursor_par_arg, int cursor_pos_arg,
	   LyXParagraph * par_arg)
{
	kind = kind_arg;
	number_of_before_par = number_before_arg;
	number_of_behind_par = number_behind_arg;
	number_of_cursor_par = cursor_par_arg;
	cursor_pos = cursor_pos_arg;
	par = par_arg;
}


Undo::~Undo()
{
	LyXParagraph * tmppar;
	while (par) {
		tmppar = par;
		par = par->next();
		delete tmppar;
	}
}


UndoStack::UndoStack()
	: limit(100) {}


Undo * UndoStack::pop()
{
	if (stakk.empty()) return 0;
	Undo * result = stakk.front();
	stakk.pop_front();
	return result;
}


Undo * UndoStack::top()
{
	if (stakk.empty()) return 0;
	return stakk.front();
}


UndoStack::~UndoStack()
{
	clear();
}


void UndoStack::clear()
{
	while (!stakk.empty()) {
		Undo * tmp = stakk.front();
		stakk.pop_front();
		delete tmp;
	}
}


void UndoStack::SetStackLimit(Stakk::size_type l)
{
	limit = l;
}


void UndoStack::push(Undo * undo_arg)
{
	if (!undo_arg) return;
	
	stakk.push_front(undo_arg);
	if (stakk.size() > limit) {
		Undo * tmp = stakk.back();
		stakk.pop_back();
		delete tmp;
	}
}


bool UndoStack::empty() const {
	return stakk.empty();
}
