// -*- C++ -*-
/**
 * \file insetnewline.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef INSET_NEWLINE_H
#define INSET_NEWLINE_H


#include "inset.h"

class InsetNewline : public InsetOld {
public:

	InsetNewline() {}

	virtual std::auto_ptr<InsetBase> clone() const {
		return std::auto_ptr<InsetBase>(new InsetNewline);
	}

	InsetOld::Code lyxCode() const { return InsetOld::NEWLINE_CODE; }

	void metrics(MetricsInfo &, Dimension &) const;

	virtual void draw(PainterInfo & pi, int x, int y) const;

	virtual int latex(Buffer const *, std::ostream &,
			  LatexRunParams const &) const;

	virtual int ascii(Buffer const *, std::ostream &, int linelen) const;

	virtual int linuxdoc(Buffer const *, std::ostream &) const;

	virtual int docbook(Buffer const *, std::ostream &, bool) const;

	virtual void read(Buffer const *, LyXLex & lex);

	virtual void write(Buffer const * buf, std::ostream & os) const;
	/// We don't need \begin_inset and \end_inset
	virtual bool directWrite() const { return true; }
};

#endif // INSET_NEWLINE_H
