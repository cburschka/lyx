// -*- C++ -*-
/**
 * \file InsetERT.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETERT_H
#define INSETERT_H

#include "InsetCollapsable.h"
#include "RenderPreview.h"
#include "graphics/PreviewLoader.h"

#include <boost/scoped_ptr.hpp>

namespace lyx {

/** A collapsable text inset for LaTeX insertions.

  To write full ert (including styles and other insets) in a given
  space.

  Note that collapsed_ encompasses both the inline and collapsed button
  versions of this inset.
*/

class Language;

class InsetERT : public InsetCollapsable {
public:
	///
	InsetERT(Buffer *, CollapseStatus status = Open);
	///
	InsetERT(InsetERT const & other);
	///
	~InsetERT();
	//
	InsetERT & operator=(InsetERT const & other);
	///
	static CollapseStatus string2params(std::string const &);
	///
	static std::string params2string(CollapseStatus);
	///
	void addPreview(DocIterator const & inset_pos,
		graphics::PreviewLoader & ploader) const;
	///
	void preparePreview(DocIterator const & pos) const;
	///
	void reloadPreview(DocIterator const & pos) const;
	///
	bool notifyCursorLeaves(Cursor const & old, Cursor & cur);
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	Inset * editXY(Cursor & cur, int x, int y);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	bool previewState(BufferView * bv) const;
private:
	///
	InsetCode lyxCode() const { return ERT_CODE; }
	///
	docstring name() const { return from_ascii("ERT"); }
	///
	void write(std::ostream & os) const;
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	///
	void validate(LaTeXFeatures &) const {}
	///
	bool showInsetDialog(BufferView *) const;
	/// should paragraph indendation be omitted in any case?
	bool neverIndent() const { return true; }
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const;
	///
	Inset * clone() const { return new InsetERT(*this); }
	///
	docstring const buttonLabel(BufferView const & bv) const;
	///
	bool allowSpellCheck() const { return false; }
	///
	boost::scoped_ptr<RenderPreview> preview_;
	///
	mutable bool use_preview_;
};


} // namespace lyx

#endif
