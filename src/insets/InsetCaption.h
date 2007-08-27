// -*- C++ -*-
/**
 * \file InsetCaption.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETCAPTION_H
#define INSETCAPTION_H

#include "InsetText.h"
#include "TextClass.h"


namespace lyx {

/** A caption inset
*/
class InsetCaption : public InsetText {
public:
	///
	InsetCaption(InsetCaption const &);
	InsetCaption(BufferParams const &);
	///
	virtual ~InsetCaption() {}
	///
	void write(Buffer const & buf, std::ostream & os) const;
	///
	void read(Buffer const & buf, Lexer & lex);
	///
	virtual DisplayType display() const;
	///
	virtual bool neverIndent(Buffer const &) const { return true; }
	///
	virtual Inset::Code lyxCode() const;
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
	virtual void edit(Cursor & cur, bool left);
	///
	virtual Inset * editXY(Cursor & cur, int x, int y);
	///
	bool insetAllowed(Inset::Code code) const;
	///
	virtual bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const;
	// Update the counters of this inset and of its contents
	virtual void updateLabels(Buffer const &, ParIterator const &);
	///
	int latex(Buffer const & buf, odocstream & os,
		  OutputParams const &) const;
	///
	int plaintext(Buffer const & buf, odocstream & os,
		      OutputParams const & runparams) const;
	///
	int docbook(Buffer const & buf, odocstream & os,
		    OutputParams const & runparams) const;
	/// return the mandatory argument (LaTeX format) only
	int getArgument(Buffer const & buf, odocstream & os,
		  OutputParams const &) const;
	/// return the optional argument(s) only
	int getOptArg(Buffer const & buf, odocstream & os,
		  OutputParams const &) const;
	///
	std::string const & type() const { return type_; }
	///
	void setType(std::string const & type) { type_ = type; }
	///
	void setCustomLabel(docstring const & label);
	///
	void addToToc(TocList &, Buffer const &, ParConstIterator const &) const;
	/// Captions don't accept alignment, spacing, etc.
	bool forceDefaultParagraphs(idx_type) const { return true; }

private:
	///
	virtual std::auto_ptr<Inset> doClone() const;
	///
	mutable docstring full_label_;
	///
	mutable int labelwidth_;
	///
	std::string type_;
	///
	docstring custom_label_;
	///
	TextClass const & textclass_;
};


inline
Inset::DisplayType InsetCaption::display() const
{
	return AlignCenter;
}


inline
Inset::Code InsetCaption::lyxCode() const
{
	return CAPTION_CODE;
}


} // namespace lyx

#endif
