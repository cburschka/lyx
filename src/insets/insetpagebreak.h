// -*- C++ -*-
/**
 * \file insetpagebreak.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_PAGEBREAK_H
#define INSET_PAGEBREAK_H


#include "inset.h"

class InsetPagebreak : public InsetOld {
public:

	InsetPagebreak() {}

	std::auto_ptr<InsetBase> clone() const {
		return std::auto_ptr<InsetBase>(new InsetPagebreak);
	}

	InsetOld::Code lyxCode() const { return InsetOld::LINE_CODE; }

	void metrics(MetricsInfo &, Dimension &) const;

	void draw(PainterInfo & pi, int x, int y) const;

	int latex(Buffer const &, std::ostream &,
		  OutputParams const &) const;

	int plaintext(Buffer const &, std::ostream &,
		  OutputParams const &) const;

	int linuxdoc(Buffer const &, std::ostream &,
		     OutputParams const &) const;

	int docbook(Buffer const &, std::ostream &,
		    OutputParams const &) const;

	void read(Buffer const &, LyXLex & lex);

	void write(Buffer const & buf, std::ostream & os) const;
	/// We don't need \begin_inset and \end_inset
	bool directWrite() const { return true; }

	bool display() const { return true; }
};

#endif // INSET_NEWLINE_H
