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


namespace lyx {

/** A caption inset
*/
class InsetCaption : public InsetText {
public:
	///
	InsetCaption(BufferParams const &);
	///
	virtual ~InsetCaption() {}
	///
	void write(Buffer const & buf, std::ostream & os) const;
	///
	void read(Buffer const & buf, LyXLex & lex);
	///
	virtual bool display() const;
	///
	virtual bool neverIndent(Buffer const &) const { return true; }
	///
	virtual InsetBase::Code lyxCode() const;
	///
	virtual docstring const editMessage() const;
	///
	virtual void cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x, int & y) const;
	///
	bool descendable() const { return true; }
	///
	virtual bool metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	virtual void draw(PainterInfo & pi, int x, int y) const;
	///
	void drawSelection(PainterInfo & pi, int x, int y) const;
	///
	virtual void edit(LCursor & cur, bool left);
	///
	virtual InsetBase * editXY(LCursor & cur, int x, int y);
	///
	bool insetAllowed(InsetBase::Code code) const;
	///
	virtual bool getStatus(LCursor & cur, FuncRequest const & cmd, FuncStatus &) const;
	///
	virtual bool wide() const { return false; }
	///
	int latex(Buffer const & buf, odocstream & os,
	          OutputParams const &) const;
	///
	int plaintext(Buffer const & buf, odocstream & os,
	              OutputParams const & runparams) const;
	///
	int docbook(Buffer const & buf, odocstream & os,
	            OutputParams const & runparams) const;
	///
	void setCount(int c) { counter_ = c; }
	///
	void setType(std::string const & type) { type_ = type; }
	///
	void setCustomLabel(docstring const & label);
	///
	void addToToc(TocList &, Buffer const &) const;

private:
	///
	void computeFullLabel() const;
	///
	virtual std::auto_ptr<InsetBase> doClone() const;
	///
	mutable docstring full_label_;
	///
	mutable int labelwidth_;
	///
	std::string type_;
	///
	docstring custom_label_;
	///
	int counter_;
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


} // namespace lyx

#endif
