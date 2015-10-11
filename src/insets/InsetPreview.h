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

#include "Dimension.h"

#include <boost/scoped_ptr.hpp>

namespace lyx {

class RenderPreview;

namespace graphics {
	class PreviewLoader;
}

/// An inset with an instant preview
class InsetPreview : public InsetText {
	
public:
	///
	InsetPreview(Buffer *);
	///
	~InsetPreview();
	///
	InsetPreview(InsetPreview const & other);
	///
	InsetPreview & operator=(InsetPreview const & other);

	/// \name Methods inherited from Inset class
	//@{
	Inset * clone() const { return new InsetPreview(*this); }

	bool neverIndent() const { return true; }
	
	InsetCode lyxCode() const { return PREVIEW_CODE; }
	
	docstring layoutName() const { return from_ascii("Preview"); }
	
	bool descendable(BufferView const & /*bv*/) const { return true; }

	std::string contextMenuName() const
		{ return "context-preview"; }

	void metrics(MetricsInfo & mi, Dimension & dim) const;

	Inset * editXY(Cursor & cur, int x, int y);

	void draw(PainterInfo & pi, int x, int y) const;

	void addPreview(DocIterator const & inset_pos,
		graphics::PreviewLoader & ploader) const;

	bool notifyCursorLeaves(Cursor const & old, Cursor & cur);

	void write(std::ostream & os) const;

	void edit(Cursor & cur, bool front, EntryDirection entry_from);
	//@}
	
protected:
	/// Retrieves the preview state. Returns true if preview
	/// is enabled and the preview image is availabled.
	bool previewState(BufferView * bv) const;
	/// Recreates the preview if preview is enabled.
	void reloadPreview(DocIterator const & pos) const;
	/// Prepare the preview if preview is enabled.
	void preparePreview(DocIterator const & pos) const;

	///
	boost::scoped_ptr<RenderPreview> preview_;
	///
	mutable bool use_preview_;

private:
	///
	mutable Dimension dim_;
};


} // namespace lyx


#endif

