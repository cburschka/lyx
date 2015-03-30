/**
 * \file InsetPreview.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Vincent van Ravesteijn
 *
 * Full author contact details are available in file CREDITS.
 */
#include "config.h"

#include "InsetPreview.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "RenderPreview.h"

#include "frontends/Painter.h"

#include "graphics/PreviewImage.h"

#include <sstream>

using namespace std;

namespace lyx {


InsetPreview::InsetPreview(Buffer * buf) 
	: InsetText(buf),
	  preview_(new RenderPreview(this)), use_preview_(true)
{
	setDrawFrame(true);
	setFrameColor(Color_previewframe);
}


InsetPreview::~InsetPreview() 
{}


InsetPreview::InsetPreview(InsetPreview const & other)
	: InsetText(other)
{
	preview_.reset(new RenderPreview(*other.preview_, this));
}


void InsetPreview::write(ostream & os) const
{
	os << "Preview" << "\n";
	text().write(os);
}


void InsetPreview::addPreview(DocIterator const & inset_pos,
	graphics::PreviewLoader &) const
{
	preparePreview(inset_pos);
}


void InsetPreview::preparePreview(DocIterator const & pos) const  
{
	TexRow texrow;
	odocstringstream str;  
	otexstream os(str, texrow);
	OutputParams runparams(&pos.buffer()->params().encoding());
	latex(os, runparams);
	docstring const snippet = str.str();
	preview_->addPreview(snippet, *pos.buffer());  
}


bool InsetPreview::previewState(BufferView * bv) const
{
	if (!editing(bv) && RenderPreview::previewText()) {
		graphics::PreviewImage const * pimage =
			preview_->getPreviewImage(bv->buffer());
		return pimage && pimage->image();
	}
	return false;
}


void InsetPreview::reloadPreview(DocIterator const & pos) const
{
	preparePreview(pos);
	preview_->startLoading(*pos.buffer());
}


void InsetPreview::draw(PainterInfo & pi, int x, int y) const
{
	use_preview_ = previewState(pi.base.bv);

	if (use_preview_) {
		// one pixel gap in front
		preview_->draw(pi, x + 1 + TEXT_TO_INSET_OFFSET, y);
		setPosCache(pi, x, y);
		return;
	}
	InsetText::draw(pi, x, y);
}


void InsetPreview::edit(Cursor & cur, bool front, EntryDirection entry_from)
{
	cur.push(*this);
	InsetText::edit(cur, front, entry_from);
}


Inset * InsetPreview::editXY(Cursor & cur, int x, int y)
{
	if (use_preview_) {
		edit(cur, true, ENTRY_DIRECTION_IGNORE);
		return this;
	}
	cur.push(*this);
	return InsetText::editXY(cur, x, y);
}


void InsetPreview::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (previewState(mi.base.bv)) {
		preview_->metrics(mi, dim);
		mi.base.textwidth += 2 * TEXT_TO_INSET_OFFSET;
		
		dim.wid = max(dim.wid, 4);
		dim.asc = max(dim.asc, 4);
		
		dim.asc += TEXT_TO_INSET_OFFSET;
		dim.des += TEXT_TO_INSET_OFFSET;
		dim.wid += TEXT_TO_INSET_OFFSET;
		dim_ = dim;
		dim.wid += TEXT_TO_INSET_OFFSET;
		// insert a one pixel gap
		dim.wid += 1;
		// Cache the inset dimension.
		setDimCache(mi, dim);
		Dimension dim_dummy;
		MetricsInfo mi_dummy = mi;
		InsetText::metrics(mi_dummy, dim_dummy);
		return;
	}
	InsetText::metrics(mi, dim);
}


bool InsetPreview::notifyCursorLeaves(Cursor const & old, Cursor & cur)
{
	reloadPreview(old);
	cur.screenUpdateFlags(Update::Force);
	return InsetText::notifyCursorLeaves(old, cur);
}


} // namespace lyx
