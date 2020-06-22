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
	RowFlags rowFlags() const { return BreakAfter | RowAfter; }
	///
	static void string2params(std::string const &, InsetNewlineParams &);
	///
	static std::string params2string(InsetNewlineParams const &);
private:
	///
	InsetCode lyxCode() const { return NEWLINE_CODE; }
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
	docstring xhtml(XMLStream &, OutputParams const &) const;
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
	Inset * clone() const { return new InsetNewline(*this); }
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const;

	///
	InsetNewlineParams params_;
};


} // namespace lyx

#endif // INSET_NEWLINE_H
