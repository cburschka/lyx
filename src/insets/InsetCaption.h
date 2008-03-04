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

namespace lyx {

/** A caption inset
*/
class InsetCaption : public InsetText {
public:
	///
	InsetCaption(InsetCaption const &);
	InsetCaption(Buffer const &);
	///
	virtual ~InsetCaption() {}
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	virtual DisplayType display() const;
	///
	virtual bool neverIndent() const { return true; }
	///
	virtual InsetCode lyxCode() const;
	///
	docstring editMessage() const;
	///
	virtual void cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x, int & y) const;
	///
	bool descendable() const { return true; }
	///
	virtual void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	virtual void draw(PainterInfo & pi, int x, int y) const;
	///
	virtual void edit(Cursor & cur, bool front, EntryDirection entry_from);
	///
	virtual Inset * editXY(Cursor & cur, int x, int y);
	///
	bool insetAllowed(InsetCode code) const;
	///
	virtual bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const;
	// Update the counters of this inset and of its contents
	virtual void updateLabels(ParIterator const &);
	///
	int latex(odocstream & os, OutputParams const &) const;
	///
	int plaintext(odocstream & os, OutputParams const & runparams) const;
	///
	int docbook(odocstream & os, OutputParams const & runparams) const;
	/// return the mandatory argument (LaTeX format) only
	int getArgument(odocstream & os, OutputParams const &) const;
	/// return the optional argument(s) only
	int getOptArg(odocstream & os, OutputParams const &) const;
	///
	std::string const & type() const { return type_; }
	///
	void setCustomLabel(docstring const & label);
	///
	void addToToc(ParConstIterator const &) const;
	/// 
	virtual bool forceEmptyLayout() const { return true; }
	/// Captions don't accept alignment, spacing, etc.
	virtual bool allowParagraphCustomization(idx_type) const { return false; }

private:
	///
	virtual Inset * clone() const;
	///
	mutable docstring full_label_;
	///
	mutable int labelwidth_;
	///
	std::string type_;
	///
	bool in_subfloat_;
	///
	docstring custom_label_;
};


inline
Inset::DisplayType InsetCaption::display() const
{
	return AlignCenter;
}


inline
InsetCode InsetCaption::lyxCode() const
{
	return CAPTION_CODE;
}


} // namespace lyx

#endif
