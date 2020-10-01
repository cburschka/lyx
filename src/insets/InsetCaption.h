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
	docstring layoutName() const override;
	/// return the mandatory argument (LaTeX format) only
	void getArgument(otexstream & os, OutputParams const &) const;
	/// return the caption text
	int getCaptionAsPlaintext(odocstream & os, OutputParams const &) const;
	/// write the caption text as DocBook in os
	void getCaptionAsDocBook(XMLStream & os, OutputParams const &) const;
	/// return the caption text as HTML
	docstring getCaptionAsHTML(XMLStream & os, OutputParams const &) const;
	///
	std::string contextMenuName() const override;
private:
	///
	void write(std::ostream & os) const override;
	///
	RowFlags rowFlags() const override { return Display; }
	///
	bool neverIndent() const override { return true; }
	///
	bool forceLocalFontSwitch() const override { return true; }
	///
	InsetCode lyxCode() const override { return CAPTION_CODE; }
	///
	void cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x, int & y) const override;
	///
	bool descendable(BufferView const &) const override { return true; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void drawBackground(PainterInfo & pi, int x, int y) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	/// Strike out the inset when deleted.
	bool canPaintChange(BufferView const &) const override { return true; }
	///
	void edit(Cursor & cur, bool front, EntryDirection entry_from) override;
	///
	Inset * editXY(Cursor & cur, int x, int y) override;
	///
	bool insetAllowed(InsetCode code) const override;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const override;
	// Update the counters of this inset and of its contents
	void updateBuffer(ParIterator const &, UpdateType, bool const deleted) override;
	///
	void latex(otexstream & os, OutputParams const &) const override;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	///
	docstring xhtml(XMLStream & os, OutputParams const & runparams) const override;
	///
	void addToToc(DocIterator const & di, bool output_active, UpdateType utype,
	              TocBackend & backend) const override;
	///
	bool forcePlainLayout(idx_type = 0) const override { return true; }
	/// Captions don't accept alignment, spacing, etc.
	bool allowParagraphCustomization(idx_type = 0) const override { return false; }
	///
	Inset * clone() const override { return new InsetCaption(*this); }

	///
	mutable docstring full_label_;
	///
	mutable int labelwidth_;
	///
	std::string floattype_;
	///
	bool is_subfloat_;
	///
	bool is_deleted_;
	///
	std::string type_;
};


} // namespace lyx

#endif
