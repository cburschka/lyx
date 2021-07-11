// -*- C++ -*-
/**
 * \file InsetNewline.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_NEWLINE_H
#define INSET_NEWLINE_H

#include "Inset.h"


namespace lyx {

class InsetNewlineParams
{
public:
	/// The different kinds of spaces we support
	enum Kind {
		///
		NEWLINE,
		///
		LINEBREAK
	};
	///
	InsetNewlineParams() : kind(NEWLINE) {}
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	Kind kind;
};


class InsetNewline : public Inset
{
public:
	///
	InsetNewline();
	///
	explicit InsetNewline(InsetNewlineParams par) : Inset(0)
	{ params_.kind = par.kind; }
	///
	int rowFlags() const override { return AlwaysBreakAfter; }
	///
	static void string2params(std::string const &, InsetNewlineParams &);
	///
	static std::string params2string(InsetNewlineParams const &);
private:
	///
	InsetCode lyxCode() const override { return NEWLINE_CODE; }
	///
	void metrics(MetricsInfo &, Dimension &) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void latex(otexstream &, OutputParams const &) const override;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	///
	docstring xhtml(XMLStream &, OutputParams const &) const override;
	///
	void read(Lexer & lex) override;
	///
	void write(std::ostream & os) const override;
	/// is this equivalent to a space (which is BTW different from
	/// a line separator)?
	bool isSpace() const override { return true; }
	///
	ColorCode ColorName() const;
	///
	std::string contextMenuName() const override;
	///
	Inset * clone() const override { return new InsetNewline(*this); }
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const override;

	///
	InsetNewlineParams params_;
};


} // namespace lyx

#endif // INSET_NEWLINE_H
