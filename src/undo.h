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

#ifndef UNDO_H
#define UNDO_H

#include "ParagraphList.h"

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
	int number_of_inset_id; // valid if >= 0, if < 0 then not in inset
	///
	int cursor_pos; // valid if >= 0
	///
	ParagraphList pars;

	///
	Undo(undo_kind kind, int inset_id,
	     int before_par_id, int behind_par_id,
	     int cursor_par_id, int cursor_pos,
	     ParagraphList const & par_arg);

};


#endif
