// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#ifndef UNDO_H
#define UNDO_H

#ifdef __GNUG__
#pragma interface
#endif

#include "lyxparagraph.h"

#include <list>

///
class Undo {
public:
	/// The undo kinds
	enum undo_kind {
		///
		INSERT,
		///
		DELETE,
		///
		EDIT,
		///
		FINISH
	};
	///
	undo_kind kind;
	///
	int number_of_before_par;
	///
	int number_of_behind_par;
	///
	int number_of_cursor_par;
	///
	int cursor_pos; // valid if >= 0
	///
	LyXParagraph * par;
	///
	Undo(undo_kind kind_arg,
	     int number_before_arg, int number_behind_arg,
	     int cursor_par_arg, int cursor_pos_arg,
	     LyXParagraph * par_arg);
	///
	~Undo();
};


/// A limited Stack for the undo informations.
class UndoStack{
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
