/**
 * \file undo.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "undo.h"

Undo::Undo(undo_kind kind_arg, int inset,
	   int first, int last,
	   int cursor, int cursor_pos_arg,
	   ParagraphList const & par)
	:
		kind(kind_arg),
		inset_id(inset),
		first_par_offset(first),
		last_par_offset(last),
		cursor_par_offset(cursor),
		cursor_pos(cursor_pos_arg),
		pars(par)
{}

