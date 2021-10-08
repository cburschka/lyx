// -*- C++ -*-
/**
 * \file InsetPreview.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Vincent van Ravesteijn
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETPREVIEW_H
#define INSETPREVIEW_H

#include "InsetText.h"

#include "support/unique_ptr.h"


namespace lyx {

class Dimension;
class MacroNameSet;
class RenderPreview;

namespace graphics {
	class PreviewLoader;
}

/// An inset with an instant preview
class InsetPreview : public InsetText {

public:
	///
	explicit InsetPreview(Buffer *);
	///
	~InsetPreview();
	///
	InsetPreview(InsetPreview const & other);
	///
	InsetPreview & operator=(InsetPreview const & other);

	/// \name Methods inherited from Inset class
	//@{
	Inset * clone() const override { return new InsetPreview(*this); }

	bool neverIndent() const override { return true; }

	InsetCode lyxCode() const override { return PREVIEW_CODE; }

	docstring layoutName() const override { return from_ascii("Preview"); }

	bool descendable(BufferView const & /*bv*/) const override { return true; }

	std::string contextMenuName() const override
		{ return "context-preview"; }

	void metrics(MetricsInfo & mi, Dimension & dim) const override;

	Inset * editXY(Cursor & cur, int x, int y) override;

	void draw(PainterInfo & pi, int x, int y) const override;

	void addPreview(DocIterator const & inset_pos,
		graphics::PreviewLoader & ploader) const override;

	bool notifyCursorLeaves(Cursor const & old, Cursor & cur) override;

	void write(std::ostream & os) const override;

	void edit(Cursor & cur, bool front, EntryDirection entry_from) override;

	bool canPaintChange(BufferView const &) const override { return true; }
	//@}

protected:
	/// Retrieves the preview state. Returns true if preview
	/// is enabled and the preview image is available.
	bool previewState(BufferView * bv) const;
	/// Recreates the preview if preview is enabled.
	void reloadPreview(DocIterator const & pos) const;
	/// Prepare the preview if preview is enabled.
	void preparePreview(DocIterator const & pos) const;

	///
	unique_ptr<RenderPreview> preview_;

};


/// gathers the list of macro definitions used in the given inset
MacroNameSet gatherMacroDefinitions(const Buffer* buffer, const Inset * inset);


} // namespace lyx


#endif

