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
        Inset * Clone(Buffer const &) const {
		return new InsetFloatList(*this);
	}
    	///
	string const getScreenLabel() const;
	///
	void Edit(BufferView * bv, int, int, unsigned int);
        ///
	EDITABLE Editable() const { return IS_EDITABLE; }
	///
	bool display() const { return true; }
	///
	Inset::Code LyxCode() const;
	///
	void Write(Buffer const *, ostream &) const;
	///
	void Read(Buffer const *, LyXLex &);
	///
	int Latex(Buffer const *, ostream &, bool, bool) const;
	///
	int Linuxdoc(Buffer const *, ostream &) const { return 0; }
	///
	int DocBook(Buffer const *, ostream &) const { return 0; }
	///
	int Ascii(Buffer const *, std::ostream &, int linelen) const;
private:
	string float_type;
};

#endif
