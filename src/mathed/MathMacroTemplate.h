// -*- C++ -*-
/**
 * \file math_macrotemplate.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_MACROTEMPLATE_H
#define MATH_MACROTEMPLATE_H

#include "InsetMathNest.h"
#include "MacroTable.h"
#include "MathData.h"

#include "support/types.h"


namespace lyx {

/// This class contains the macro definition.
class MathMacroTemplate : public InsetMathNest {
public:
	///
	MathMacroTemplate();
	///
	MathMacroTemplate(docstring const & name, int nargs, int optional, 
		docstring const & type, 
		std::vector<MathData> const & optionalValues = std::vector<MathData>(),
		MathData const & def = MathData(),
		MathData const & display = MathData());
	///
	explicit MathMacroTemplate(const docstring & str);
	///
	EDITABLE editable() const { return HIGHLY_EDITABLE; }
	///
	void read(Buffer const &, Lexer & lex);
	///
	void write(Buffer const &, std::ostream & os) const;
	///
	void write(WriteStream & os) const;
	///
	int plaintext(Buffer const &, odocstream &,
		OutputParams const &) const;

	///
	docstring name() const;
	/// check name and possible other formal properties
	bool validMacro() const;
	///
	bool validName() const;
	///
	MacroData asMacroData() const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	/// identifies macro templates
	MathMacroTemplate * asMacroTemplate() { return this; }
	/// identifies macro templates
	MathMacroTemplate const * asMacroTemplate() const { return this; }
	///
	InsetCode lyxCode() const { return MATHMACRO_CODE; }

protected:
	///
	virtual void doDispatch(Cursor & cur, FuncRequest & cmd);
	/// do we want to handle this event?
	bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & status) const;

private:
	virtual Inset * clone() const;

	/// remove #n with from<=n<=to
	void removeArguments(Cursor & cur, int from, int to);
	/// shift every #n with from<=n, i.e. #n -> #(n-by)
	void shiftArguments(size_t from, int by);
	///
	void insertParameter(Cursor & cur, int pos, bool greedy = false);
	///
	void removeParameter(Cursor & cur, int pos, bool greedy = false );
	///
	void makeOptional(Cursor & cur);
	///
	void makeNonOptional(Cursor & cur);
	///
	idx_type defIdx() const { return optionals_ + 1; }
	/// index of default value cell of optional parameter (#1 -> n=0)
	idx_type optIdx(idx_type n) const { return n + 1; }
	///
	idx_type displayIdx() const { return optionals_ + 2; }
	/// The label with some holes to edit
	mutable MathData label_;
	///
	mutable int numargs_;
	///
	int optionals_;
	/// keeps the old optional default value when an optional argument is disabled
	std::vector<MathData> optionalValues_;
	/// newcommand or renewcommand or def
	mutable docstring type_;
};


} // namespace lyx

#endif
