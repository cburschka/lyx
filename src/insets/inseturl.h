// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1997-2000 The LyX Team.
 * 
 * ====================================================== */

#ifndef INSET_URL_H
#define INSET_URL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"

struct LaTeXFeatures;

/** The url inset  
 */
class InsetUrl : public InsetCommand {
public:
	///
	explicit
	InsetUrl(InsetCommandParams const &);
        ///
	Inset * Clone() const { return new InsetUrl(params()); }
	///
	Inset::Code LyxCode() const { return Inset::URL_CODE; }
	///
	void Validate(LaTeXFeatures &) const;
	///
	string const getScreenLabel() const;
	///
	EDITABLE Editable() const { return IS_EDITABLE; }
	///
	void Edit(BufferView *, int, int, unsigned int);
        ///
	bool display() const { return false; }
	///
	int Latex(Buffer const *, std::ostream &,
		  bool fragile, bool free_spc) const;
	///
	int Ascii(Buffer const *, std::ostream &) const;
	///
	int Linuxdoc(Buffer const *, std::ostream &) const;
	///
	int DocBook(Buffer const *, std::ostream &) const;
};

#endif
