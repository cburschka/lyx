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

#ifndef INSET_TOC_H
#define INSET_TOC_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"

/** Used to insert table of contents
 */
class InsetTOC : public InsetCommand {
public:
	///
	InsetTOC(InsetCommandParams const & p) : InsetCommand(p) {}
	///
        Inset * Clone(Buffer const &) const { return new InsetTOC(params()); }
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
	int Ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int Linuxdoc(Buffer const *, std::ostream &) const;
	///
	int DocBook(Buffer const *, std::ostream &) const;
};

#endif
