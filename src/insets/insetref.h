// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1997 LyX Team (this file was created this year)
 * 
 * ====================================================== */

#ifndef INSET_REF_H
#define INSET_REF_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"

struct LaTeXFeatures;

/** The reference inset  
 */
class InsetRef : public InsetCommand {
public:
	///
	InsetRef(InsetCommandParams const &);
	///
	Inset * Clone() const { return new InsetRef(params()); }
	///
	string getScreenLabel() const;
	///
	EDITABLE Editable() const { return IS_EDITABLE; }
	///
	Inset::Code LyxCode() const { return Inset::REF_CODE; }
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
	///
	void Validate(LaTeXFeatures & features) const;
private:
	/// This function escapes 8-bit characters
	string escape(string const &) const;
};
#endif
