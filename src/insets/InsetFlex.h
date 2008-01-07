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
	InsetFlex(BufferParams const &, InsetLayout const &);
	///
	docstring name() const { return from_utf8(name_); }

	/// Is this character style defined in the document's textclass?
	/// May be wrong after textclass change or paste from another document
	bool undefined() const;
	///
	virtual docstring const editMessage() const;
	///
	InsetCode lyxCode() const { return FLEX_CODE; }
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void read(Buffer const & buf, Lexer & lex);
	///
	bool forceDefaultParagraphs(idx_type) const { return true; }

	///
	int plaintext(Buffer const &, odocstream &,
		      OutputParams const &) const;
	///
	int docbook(Buffer const &, odocstream &,
		    OutputParams const &) const;
	/// the string that is passed to the TOC
	virtual void textString(Buffer const &, odocstream &) const;
	///
	void validate(LaTeXFeatures &) const;

	/// should paragraph indendation be ommitted in any case?
	bool neverIndent(Buffer const &) const { return true; }

protected:
	InsetFlex(InsetFlex const &);

private:
	virtual Inset * clone() const;

	///
	std::string name_;
	///
	std::vector<std::string> packages_;
	///
	std::string preamble_;
};


} // namespace lyx

#endif
