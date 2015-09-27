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
	InsetCaption(Buffer *, std::string const &);
	///
	std::string const & floattype() const { return floattype_; }
	///
	docstring layoutName() const;
	/// return the mandatory argument (LaTeX format) only
	void getArgument(otexstream & os, OutputParams const &) const;
	/// return the caption text
	int getCaptionAsPlaintext(odocstream & os, OutputParams const &) const;
	/// return the caption text as HTML
	docstring getCaptionAsHTML(XHTMLStream & os, OutputParams const &) const;
	///
	std::string contextMenuName() const;
private:
	///
	void write(std::ostream & os) const;
	///
	DisplayType display() const { return AlignCenter; }
	///
	bool neverIndent() const { return true; }
	///
	bool forceLocalFontSwitch() const { return true; }
	///
	InsetCode lyxCode() const { return CAPTION_CODE; }
	///
	void cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x, int & y) const;
	///
	bool descendable(BufferView const &) const { return true; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///	
	void drawBackground(PainterInfo & pi, int x, int y) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void edit(Cursor & cur, bool front, EntryDirection entry_from);
	///
	Inset * editXY(Cursor & cur, int x, int y);
	///
	bool insetAllowed(InsetCode code) const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const;
	// Update the counters of this inset and of its contents
	void updateBuffer(ParIterator const &, UpdateType);
	///
	void latex(otexstream & os, OutputParams const &) const;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const;
	///
	int docbook(odocstream & os, OutputParams const & runparams) const;
	/// 
	docstring xhtml(XHTMLStream & os, OutputParams const & runparams) const;
	///
	void setCustomLabel(docstring const & label);
	///
	void addToToc(DocIterator const & di, bool output_active, UpdateType utype) const;
	/// 
	virtual bool forcePlainLayout(idx_type = 0) const { return true; }
	/// Captions don't accept alignment, spacing, etc.
	virtual bool allowParagraphCustomization(idx_type = 0) const { return false; }
	///
	Inset * clone() const { return new InsetCaption(*this); }

	///
	mutable docstring full_label_;
	///
	mutable int labelwidth_;
	///
	std::string floattype_;
	///
	bool is_subfloat_;
	///
	std::string type_;
	///
	docstring custom_label_;
};


} // namespace lyx

#endif
