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

#include "undostack.h"
#include "undo.h"
#include "paragraph.h"


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
