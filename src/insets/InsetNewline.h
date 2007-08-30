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

class InsetNewline : public Inset {
public:

	InsetNewline() {}

	Inset::Code lyxCode() const { return Inset::NEWLINE_CODE; }

	bool metrics(MetricsInfo &, Dimension &) const;

	virtual void draw(PainterInfo & pi, int x, int y) const;

	int latex(Buffer const &, odocstream &, OutputParams const &) const;

	int plaintext(Buffer const &, odocstream &, OutputParams const &) const;

	int docbook(Buffer const &, odocstream &, OutputParams const &) const;

	virtual void read(Buffer const &, Lexer & lex);

	virtual void write(Buffer const & buf, std::ostream & os) const;
	/// We don't need \begin_inset and \end_inset
	virtual bool directWrite() const { return true; }
	/// is this equivalent to a space (which is BTW different from
	// a line separator)?
	bool isSpace() const;
private:
	virtual Inset * clone() const
	{
		return new InsetNewline;
	}
};


} // namespace lyx

#endif // INSET_NEWLINE_H
