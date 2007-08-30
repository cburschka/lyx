// -*- C++ -*-
/**
 * \file InsetLine.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_LINE_H
#define INSET_LINE_H


#include "Inset.h"


namespace lyx {

class InsetLine : public Inset {
public:

	InsetLine() {}

	Inset::Code lyxCode() const { return Inset::LINE_CODE; }

	bool metrics(MetricsInfo &, Dimension &) const;

	void draw(PainterInfo & pi, int x, int y) const;

	int latex(Buffer const &, odocstream &,
		  OutputParams const &) const;

	int plaintext(Buffer const &, odocstream &,
		      OutputParams const &) const;

	int docbook(Buffer const &, odocstream &,
		    OutputParams const &) const;

	void read(Buffer const &, Lexer & lex);

	void write(Buffer const & buf, std::ostream & os) const;
	/// We don't need \begin_inset and \end_inset
	bool directWrite() const { return true; }

	DisplayType display() const { return AlignCenter; }
	///
	void validate(LaTeXFeatures & features) const;
private:
	virtual Inset * clone() const
	{
		return new InsetLine;
	}
};


} // namespace lyx

#endif // INSET_NEWLINE_H
