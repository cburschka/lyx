// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Word Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1996-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef INSET_FLOATLIST_H
#define INSET_FLOATLIST_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"

/** Used to insert table of contents
 */
class InsetFloatList : public InsetCommand {
public:
	///
	InsetFloatList();
	///
	InsetFloatList(string const & type);
	///
	Inset * clone(Buffer const &, bool = false) const {
		return new InsetFloatList(getCmdName());
	}
	///
	string const getScreenLabel(Buffer const *) const;
	///
	void edit(BufferView * bv, int, int, mouse_button::state);
	///
	void edit(BufferView * bv, bool front = true);
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	bool display() const { return true; }
	///
	Inset::Code lyxCode() const;
	///
	void write(Buffer const *, std::ostream &) const;
	///
	void read(Buffer const *, LyXLex &);
	///
	int latex(Buffer const *, std::ostream &, bool, bool) const;
	///
	int linuxdoc(Buffer const *, std::ostream &) const { return 0; }
	///
	int docbook(Buffer const *, std::ostream &) const { return 0; }
	///
	int ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	void validate(LaTeXFeatures & features) const;
};

#endif
