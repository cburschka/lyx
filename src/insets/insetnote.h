// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 *======================================================
 */

#ifndef INSETNOTE_H
#define INSETNOTE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcollapsable.h"

/** The PostIt note inset
  
*/
class InsetNote : public InsetCollapsable {
public:
	/// 
	InsetNote(BufferParams const &);
	///
	InsetNote(InsetNote const &, bool same_id = false);
	///
	Inset * clone(Buffer const &, bool same_id = false) const;
	/// constructor with initial contents
	InsetNote(Buffer const *, string const & contents, bool collapsed);
	///
	string const editMessage() const;
	///
	Inset::Code lyxCode() const { return Inset::IGNORE_CODE; }
	///
	void write(Buffer const *, std::ostream &) const;
	///
	int latex(Buffer const *, std::ostream &, bool, bool) const
		{ return 0; }
	///
	void validate(LaTeXFeatures &) const {}
private:
	/// used by the constructors
	void init();

};

#endif
