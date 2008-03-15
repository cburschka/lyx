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

#include "support/docstring.h"
#include "support/gettext.h"


namespace lyx {

class InsetNewline : public Inset
{
public:
	///
	InsetNewline() {}
	///
	InsetCode lyxCode() const { return NEWLINE_CODE; }
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	void read(Lexer & lex);
	///
	void write(std::ostream & os) const;
	/// We don't need \begin_inset and \end_inset
	bool directWrite() const { return true; }
	/// is this equivalent to a space (which is BTW different from
	/// a line separator)?
	bool isSpace() const;
	///
	docstring insetLabel() const { return docstring(); }
	///
	std::string getLyXName() const { return "\\newline"; }
	///
	std::string getCmdName() const { return "\\\\"; }
	///
	ColorCode ColorName() const { return Color_eolmarker; }

private:
	///
	Inset * clone() const { return new InsetNewline(*this); }
};


class InsetLinebreak : public InsetNewline
{
public:
	///
	InsetLinebreak() {}
	///
	docstring insetLabel() const { return _("line break"); }
	///
	std::string getLyXName() const { return "\\linebreak"; }
	///
	std::string getCmdName() const { return "\\linebreak{}"; }
	///
	ColorCode ColorName() const { return Color_pagebreak; }

private:
	///
	Inset * clone() const { return new InsetLinebreak(*this); }
};


} // namespace lyx

#endif // INSET_NEWLINE_H
