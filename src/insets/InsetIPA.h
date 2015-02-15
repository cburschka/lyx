// -*- C++ -*-
/**
 * \file InsetIPA.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETIPA_H
#define INSETIPA_H

#include "InsetText.h"

#include "Dimension.h"

#include <boost/scoped_ptr.hpp>

namespace lyx {

class RenderPreview;

namespace graphics {
	class PreviewLoader;
}

/// An IPA inset with instant preview
class InsetIPA : public InsetText {
	
public:
	///
	InsetIPA(Buffer *);
	///
	~InsetIPA();
	///
	InsetIPA(InsetIPA const & other);

	/// \name Methods inherited from Inset class
	//@{
	Inset * clone() const { return new InsetIPA(*this); }

	bool neverIndent() const { return true; }
	
	InsetCode lyxCode() const { return IPA_CODE; }
	
	docstring layoutName() const { return from_ascii("IPA"); }
	
	bool descendable(BufferView const & /*bv*/) const { return true; }

	void metrics(MetricsInfo & mi, Dimension & dim) const;

	Inset * editXY(Cursor & cur, int x, int y);

	void draw(PainterInfo & pi, int x, int y) const;

	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;

	void addPreview(DocIterator const & inset_pos,
		graphics::PreviewLoader & ploader) const;

	bool notifyCursorLeaves(Cursor const & old, Cursor & cur);

	void write(std::ostream & os) const;

	void edit(Cursor & cur, bool front, EntryDirection entry_from);
	
	///
	void latex(otexstream &, OutputParams const &) const;
	///
	docstring xhtml(XHTMLStream & xs, OutputParams const &) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	bool allowSpellCheck() const { return false; }
	///
	bool insetAllowed(InsetCode code) const;
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

