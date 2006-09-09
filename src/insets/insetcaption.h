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
#include "lyxtextclass.h"

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
	virtual bool neverIndent() const { return true; }
	///
	virtual InsetBase::Code lyxCode() const;
	///
	virtual lyx::docstring const editMessage() const;
	///
	virtual void cursorPos
		(CursorSlice const & sl, bool boundary, int & x, int & y) const;
	///
	bool descendable() const { return true; }
	///
	virtual void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	virtual void draw(PainterInfo & pi, int x, int y) const;
	///
	virtual void edit(LCursor & cur, bool left);
	///
	virtual InsetBase * editXY(LCursor & cur, int x, int y);
	///
	virtual int latex(Buffer const & buf, std::ostream & os,
			  OutputParams const &) const;
	///
	int plaintext(Buffer const & buf, std::ostream & os,
		  OutputParams const & runparams) const;
	///
	int docbook(Buffer const & buf, std::ostream & os,
		    OutputParams const & runparams) const;
	///
	void setCount(int c) { counter_ = c; }
private:
	///
	void setLabel(LCursor & cur) const;
	///
	virtual std::auto_ptr<InsetBase> doClone() const;
	///
	mutable std::string label;
	///
	mutable int labelwidth_;
	///
	mutable int counter_;
	///
	LyXTextClass const & textclass_;
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
