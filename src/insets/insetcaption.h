// -*- C++ -*-
/**
 * \file insetcaption.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETCAPTION_H
#define INSETCAPTION_H


#include "insettext.h"

/** A caption inset
*/
class InsetCaption : public InsetText {
public:
	///
	InsetCaption(BufferParams const &);
	///
	void write(Buffer const & buf, std::ostream & os) const;
	///
	void read(Buffer const & buf, LyXLex & lex);
	///
	virtual bool display() const;
	///
	virtual InsetBase::Code lyxCode() const;
	///
	virtual std::string const editMessage() const;
	///
	virtual void draw(PainterInfo & pi, int x, int y) const;
	///
	virtual int latex(Buffer const & buf, std::ostream & os,
			  OutputParams const &) const;
	///
	int plaintext(Buffer const & buf, std::ostream & os,
		  OutputParams const & runparams) const;
	///
	int docbook(Buffer const & buf, std::ostream & os,
		    OutputParams const & runparams) const;
};


inline
bool InsetCaption::display() const
{
	return true;
}


inline
InsetBase::Code InsetCaption::lyxCode() const
{
	return CAPTION_CODE;
}

#endif
