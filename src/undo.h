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
	int inset_id; // valid if >= 0, if < 0 then not in inset
	///
	int plist_id;
	///
	int first_par_offset;
	///
	int last_par_offset;
	///
	int cursor_par_offset;
	///
	int cursor_pos; // valid if >= 0
	///
	ParagraphList pars;

	///
	Undo(undo_kind kind, int inset_id, int plist_id,
	     int first, int last,
	     int cursor, int cursor_pos,
	     ParagraphList const & par_arg);

};


#endif
