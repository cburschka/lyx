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
	InsetFlex(BufferParams const &, InsetLayout);
	///
	docstring name() const { return from_ascii("Flex"); }
	/// Is this character style defined in the document's textclass?
	/// May be wrong after textclass change or paste from another document
	bool undefined() const;
	/// (Re-)set the character style parameters from \p il
	void setLayout(InsetLayout il);
	///
	virtual docstring const editMessage() const;
	///
	Inset::Code lyxCode() const { return Inset::FLEX_CODE; }
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void read(Buffer const & buf, Lexer & lex);
	///
	bool metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo &, int, int) const;
	///
	void getDrawFont(Font &) const;
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
	virtual void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const;

private:
	friend class InsetFlexParams;

	virtual Inset * clone() const;

	///
	InsetFlexParams params_;
};


} // namespace lyx

#endif
