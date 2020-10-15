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

#include "support/unique_ptr.h"


namespace lyx {

class Dimension;
class RenderPreview;

namespace graphics {
	class PreviewLoader;
}

/// An IPA inset with instant preview
class InsetIPA : public InsetText {

public:
	///
	explicit InsetIPA(Buffer *);
	///
	~InsetIPA();
	///
	InsetIPA(InsetIPA const & other);
	///
	InsetIPA & operator=(InsetIPA const & other);

	/// \name Methods inherited from Inset class
	//@{
	Inset * clone() const override { return new InsetIPA(*this); }

	bool neverIndent() const override { return true; }

	bool forceLocalFontSwitch() const override { return true; }

	InsetCode lyxCode() const override { return IPA_CODE; }

	docstring layoutName() const override { return from_ascii("IPA"); }

	bool descendable(BufferView const & /*bv*/) const override { return true; }

	void metrics(MetricsInfo & mi, Dimension & dim) const override;

	Inset * editXY(Cursor & cur, int x, int y) override;

	void draw(PainterInfo & pi, int x, int y) const override;

	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const override;

	void addPreview(DocIterator const & inset_pos,
		graphics::PreviewLoader & ploader) const override;

	bool notifyCursorLeaves(Cursor const & old, Cursor & cur) override;

	void write(std::ostream & os) const override;

	void edit(Cursor & cur, bool front, EntryDirection entry_from) override;

	///
	void latex(otexstream &, OutputParams const &) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	///
	docstring xhtml(XMLStream & xs, OutputParams const &) const override;
	///
	void validate(LaTeXFeatures & features) const override;
	///
	bool allowSpellCheck() const override { return false; }
	///
	bool insetAllowed(InsetCode code) const override;
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


} // namespace lyx


#endif

