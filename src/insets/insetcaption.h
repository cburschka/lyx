// -*- C++ -*-
/**
 * \file insetcaption.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef INSETCAPTION_H
#define INSETCAPTION_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insettext.h"

/** A caption inset
*/
class InsetCaption : public InsetText {
public:
	///
	InsetCaption(BufferParams const &);
	///
	void write(Buffer const * buf, std::ostream & os) const;
	///
	void read(Buffer const * buf, LyXLex & lex);
	///
	virtual
	bool display() const;
	///
	virtual
	bool needFullRow() const;
	///
	virtual
	Inset::Code lyxCode() const;
	///
	virtual
	string const editMessage() const;
	///
	virtual
	void draw(BufferView * bv, LyXFont const & f,
			  int baseline, float & x, bool cleared) const;
	///
	virtual
	int latex(Buffer const * buf, std::ostream & os,
			  bool fragile, bool free_spc) const;
	///
	virtual
	int ascii(Buffer const * buf, std::ostream & os, int linelen) const;
	///
	virtual
	int docbook(Buffer const * buf, std::ostream & os, bool mixcont) const;
protected:
private:
};


inline
bool InsetCaption::display() const
{
	return true;
}


inline
bool InsetCaption::needFullRow() const
{
	return true;
}


inline
Inset::Code InsetCaption::lyxCode() const
{
	return CAPTION_CODE;
}
#endif
