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
#include "TextClass.h"


namespace lyx {


class InsetFlexParams {
public:
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	std::string name;
};


/** The Flex inset, e.g., CharStyle, Custom inset or XML short element

*/
class InsetFlex : public InsetCollapsable {
public:
	///
	InsetFlex(BufferParams const &, InsetLayout const &);
	///
	docstring name() const { return from_ascii("Flex"); }
	///
	InsetLayout const & getLayout(BufferParams const &) const
	{ return *layout_; } 
	/// Is this character style defined in the document's textclass?
	/// May be wrong after textclass change or paste from another document
	bool undefined() const;
	/// (Re-)set the character style parameters from \p il
	void setLayout(InsetLayout const & il);
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
	InsetFlexParams const & params() const { return params_; }

	/// should paragraph indendation be ommitted in any case?
	bool neverIndent(Buffer const &) const { return true; }

protected:
	InsetFlex(InsetFlex const &);

private:
	friend class InsetFlexParams;

	virtual Inset * clone() const;

	///
	InsetFlexParams params_;
};


} // namespace lyx

#endif
