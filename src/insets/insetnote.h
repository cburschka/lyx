// -*- C++ -*-
/**
 * \file insetnote.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
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
	Inset::Code lyxCode() const { return Inset::NOTE_CODE; }
	///
	void write(Buffer const *, std::ostream &) const;
	///
	int latex(Buffer const *, std::ostream &, bool, bool) const
		{ return 0; }
	///
	int linuxdoc(Buffer const *, std::ostream &) const
		{ return 0; }
	///
	int docbook(Buffer const *, std::ostream &, bool) const
		{ return 0; }
	///
	int ascii(Buffer const *, std::ostream &, int) const
		{ return 0; }
	///
	void validate(LaTeXFeatures &) const {}
private:
	/// used by the constructors
	void init();

};

#endif
