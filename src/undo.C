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

#include "undo.h"

Undo::Undo(undo_kind kind_arg, int inset, int plist,
	   int first, int last,
	   int cursor, int cursor_pos_arg,
	   ParagraphList const & par)
	:
		kind(kind_arg),
		inset_id(inset),
		plist_id(plist),
		first_par_offset(first),
		last_par_offset(last),
		cursor_par_offset(cursor),
		cursor_pos(cursor_pos_arg),
		pars(par)
{}

