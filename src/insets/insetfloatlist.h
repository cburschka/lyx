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

#include "insetbutton.h"

/** Used to insert table of contents
 */
class InsetFloatList : public InsetButton {
public:
	///
	InsetFloatList() {}
	///
	InsetFloatList(string const & type) 
		: float_type(type) {}
	///
	Inset * clone(Buffer const &, bool same_id = false) const {
		return new InsetFloatList(*this);
	}
    	///
	string const getScreenLabel() const;
	///
	void edit(BufferView * bv, int, int, unsigned int);
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
	int docBook(Buffer const *, std::ostream &) const { return 0; }
	///
	int ascii(Buffer const *, std::ostream &, int linelen) const;
private:
	string float_type;
};

#endif
