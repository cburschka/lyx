// -*- C++ -*-
/**
 * \file insetnewline.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_NEWLINE_H
#define INSET_NEWLINE_H


#include "inset.h"

class InsetNewline : public InsetOld {
public:

	InsetNewline() {}

	InsetBase::Code lyxCode() const { return InsetBase::NEWLINE_CODE; }

	void metrics(MetricsInfo &, Dimension &) const;

	virtual void draw(PainterInfo & pi, int x, int y) const;

	virtual int latex(Buffer const &, std::ostream &,
			  OutputParams const &) const;

	virtual int plaintext(Buffer const &, std::ostream &,
			  OutputParams const &) const;

	virtual int linuxdoc(Buffer const &, std::ostream &,
			     OutputParams const &) const;

	virtual int docbook(Buffer const &, std::ostream &,
			    OutputParams const &) const;

	virtual void read(Buffer const &, LyXLex & lex);

	virtual void write(Buffer const & buf, std::ostream & os) const;
	/// We don't need \begin_inset and \end_inset
	virtual bool directWrite() const { return true; }
	/// is this equivalent to a space (which is BTW different from
	// a line separator)?
	bool isSpace() const;
private:
	virtual std::auto_ptr<InsetBase> doClone() const
	{
		return std::auto_ptr<InsetBase>(new InsetNewline);
	}
};

#endif // INSET_NEWLINE_H
