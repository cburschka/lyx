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
	InsetCaption(Buffer const &);
	///
	std::string const & type() const { return type_; }
	/// return the mandatory argument (LaTeX format) only
	int getArgument(odocstream & os, OutputParams const &) const;
	/// return the optional argument(s) only
	int getOptArg(odocstream & os, OutputParams const &) const;
	/// return the caption text
	int getCaptionText(odocstream & os, OutputParams const &) const;
private:
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	DisplayType display() const { return AlignCenter; }
	///
	bool neverIndent() const { return true; }
	///
	InsetCode lyxCode() const { return CAPTION_CODE; }
	///
	docstring editMessage() const;
	///
	void cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x, int & y) const;
	///
	bool descendable() const { return true; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void edit(Cursor & cur, bool front, EntryDirection entry_from);
	///
	Inset * editXY(Cursor & cur, int x, int y);
	///
	bool insetAllowed(InsetCode code) const;
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const;
	// Update the counters of this inset and of its contents
	void updateLabels(ParIterator const &);
	///
	int latex(odocstream & os, OutputParams const &) const;
	///
	int plaintext(odocstream & os, OutputParams const & runparams) const;
	///
	int docbook(odocstream & os, OutputParams const & runparams) const;
	///
	void setCustomLabel(docstring const & label);
	///
	void addToToc(ParConstIterator const &) const;
	/// 
	virtual bool forceEmptyLayout(idx_type = 0) const { return true; }
	/// Captions don't accept alignment, spacing, etc.
	virtual bool allowParagraphCustomization(idx_type = 0) const { return false; }
	///
	Inset * clone() const { return new InsetCaption(*this); }

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


} // namespace lyx

#endif
