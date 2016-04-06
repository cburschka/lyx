// -*- C++ -*-
/**
 * \file InsetSeparator.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Enrico Forestieri
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_SEPARATOR_H
#define INSET_SEPARATOR_H

#include "Inset.h"


namespace lyx {

class InsetSeparatorParams
{
public:
	/// The different kinds of separators we support
	enum Kind {
		PLAIN,
		PARBREAK,
		LATEXPAR
	};
	///
	InsetSeparatorParams() : kind(PLAIN) {}
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	Kind kind;
};


class InsetSeparator : public Inset
{
public:
	///
	InsetSeparator();
	///
	explicit InsetSeparator(InsetSeparatorParams const & par);
	///
	static void string2params(std::string const &, InsetSeparatorParams &);
	///
	static std::string params2string(InsetSeparatorParams const &);
private:
	///
	InsetSeparatorParams params() const { return params_; }
	///
	InsetCode lyxCode() const { return SEPARATOR_CODE; }
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void latex(otexstream &, OutputParams const &) const;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	///
	void read(Lexer & lex);
	///
	void write(std::ostream & os) const;
	/// is this equivalent to a space (which is BTW different from
	/// a line separator)?
	bool isSpace() const { return true; }
	///
	ColorCode ColorName() const;
	///
	std::string contextMenuName() const;
	///
	Inset * clone() const { return new InsetSeparator(*this); }
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const;

	///
	InsetSeparatorParams params_;
};


} // namespace lyx

#endif // INSET_SEPARATOR_H
