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
        virtual Inset * clone(Buffer const &) const {
		return new InsetTOC(params());
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
	int ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int linuxdoc(Buffer const *, std::ostream &) const;
	///
	int DocBook(Buffer const *, std::ostream &) const;
};

#endif
