// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef UNDO_STACK_H
#define UNDO_STACK_H

#ifdef __GNUG__
#pragma interface
#endif

#include <list>

class Undo;

/// A limited Stack for the undo informations.
class UndoStack {
private:
	///
	typedef std::list<Undo*> Stakk;
	///
	Stakk stakk;
	/// the maximum number of undo steps stored.
	Stakk::size_type limit;
public:
	///
	UndoStack();
	///
	Undo * pop();
	///
	Undo * top();
	///
	bool empty() const;
	///
	~UndoStack();
	///
	void clear();
	///
	void SetStackLimit(Stakk::size_type l);
	///
	void push(Undo * undo_arg);
};

#endif
