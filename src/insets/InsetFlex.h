// -*- C++ -*-
/**
 * \file InsetFlex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETFLEX_H
#define INSETFLEX_H

#include "InsetCollapsable.h"

namespace lyx {

/** The Flex inset, e.g., CharStyle, Custom inset or XML short element

*/
class InsetFlex : public InsetCollapsable {
public:
	///
	InsetFlex(Buffer const &, std::string const & layoutName);
	///
	docstring name() const { return from_utf8(name_); }

	///
	docstring editMessage() const;
	///
	InsetCode lyxCode() const { return FLEX_CODE; }
	///
	void write(std::ostream &) const;
	///
	void read(Lexer & lex);

	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	/// the string that is passed to the TOC
	void textString(odocstream &) const;

	/// should paragraph indendation be ommitted in any case?
	bool neverIndent() const { return true; }

protected:
	InsetFlex(InsetFlex const &);

private:
	Inset * clone() const { return new InsetFlex(*this); }
	/// should paragraphs be forced to use the empty layout?
	//FIXME: this is not always correct. We need a layout tag that indicates
	// whether layout changes are allowed or not
	virtual bool forcePlainLayout(idx_type = 0) const { return !allowMultiPar(); }
	/// should the user be allowed to customize alignment, etc.?
	//FIXME: this is not always correct. We need a layout tag that indicates
	// whether paragraph customization is allowed or not
	virtual bool allowParagraphCustomization(idx_type = 0) const { return allowMultiPar(); }

	///
	std::string name_;
};


} // namespace lyx

#endif
