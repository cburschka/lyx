// -*- C++ -*-
/**
 * \file InsetEnvironment.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETENVIRONMENT_H
#define INSETENVIRONMENT_H

#include "InsetText.h"

namespace lyx {

class Layout;
	
class InsetEnvironment : public InsetText {
public:
	///
	InsetEnvironment(Buffer const &, docstring const & name);
	///
	docstring name() const { return name_; }
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	InsetCode lyxCode() const { return ENVIRONMENT_CODE; }
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	docstring editMessage() const;
	///
	Inset::EDITABLE editable() const { return HIGHLY_EDITABLE; }
	///
	Layout const & layout() const;
	/** returns true if, when outputing LaTeX, font changes should
	    be closed before generating this inset. This is needed for
	    insets that may contain several paragraphs */
	bool noFontChange() const { return true; }
private:
	///
	Inset * clone() const { return new InsetEnvironment(*this); }
	/// the layout
	Layout const & layout_;
	///
	docstring name_;
};


} // namespace lyx

#endif
