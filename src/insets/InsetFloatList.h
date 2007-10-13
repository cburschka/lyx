// -*- C++ -*-
/**
 * \file InsetFloatList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_FLOATLIST_H
#define INSET_FLOATLIST_H


#include "InsetCommand.h"


namespace lyx {

/** Used to insert table of contents
 */
class InsetFloatList : public InsetCommand {
public:
	///
	InsetFloatList();
	///
	InsetFloatList(std::string const & type);
	///
	docstring const getScreenLabel(Buffer const &) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	InsetCode lyxCode() const;
	///
	DisplayType display() const { return AlignCenter; }
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void read(Buffer const &, Lexer &);
	///
	int latex(Buffer const &, odocstream &,
		  OutputParams const &) const;
	///
	int docbook(Buffer const &, odocstream &,
		    OutputParams const &) const { return 0; }
	///
	int plaintext(Buffer const &, odocstream &,
		      OutputParams const & runparams) const;
	///
	void validate(LaTeXFeatures & features) const;
private:
	virtual Inset * clone() const
	{
		return new InsetFloatList(to_ascii(getParam("type")));
	}
};


} // namespace lyx

#endif
