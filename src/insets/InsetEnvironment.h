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
#include "lyxlayout_ptr_fwd.h"


namespace lyx {

class InsetEnvironment : public InsetText {
public:
	///
	InsetEnvironment(BufferParams const &, docstring const & name);
	///
	docstring name() const { return name_; }
	///
	void write(Buffer const & buf, std::ostream & os) const;
	///
	void read(Buffer const & buf, Lexer & lex);
	///
	Inset::Code lyxCode() const { return Inset::ENVIRONMENT_CODE; }
	///
	int latex(Buffer const &, odocstream &,
		  OutputParams const &) const;
	///
	int plaintext(Buffer const &, odocstream &,
		      OutputParams const &) const;
	///
	virtual docstring const editMessage() const;
	///
	Inset::EDITABLE editable() const { return HIGHLY_EDITABLE; }
	///
	LayoutPtr const & layout() const;
	/** returns true if, when outputing LaTeX, font changes should
	    be closed before generating this inset. This is needed for
	    insets that may contain several paragraphs */
	bool noFontChange() const { return true; }
protected:
	InsetEnvironment(InsetEnvironment const &);
private:
	virtual Inset * clone() const;
	/// the layout
	LayoutPtr layout_;
	///
	docstring name_;
};


} // namespace lyx

#endif
