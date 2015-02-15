/**
 * \file InsetIPA.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */
#include "config.h"

#include "InsetIPA.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "RenderPreview.h"

#include "frontends/Painter.h"

#include "graphics/PreviewImage.h"

#include <sstream>

using namespace std;

namespace lyx {


InsetIPA::InsetIPA(Buffer * buf) 
	: InsetText(buf),
	  preview_(new RenderPreview(this)), use_preview_(true)
{
	setAutoBreakRows(true);
	setDrawFrame(true);
	setFrameColor(Color_insetframe);
}


InsetIPA::~InsetIPA() 
{}


InsetIPA::InsetIPA(InsetIPA const & other)
	: InsetText(other)
{
	preview_.reset(new RenderPreview(*other.preview_, this));
}


void InsetIPA::write(ostream & os) const
{
	os << "IPA" << "\n";
	text().write(os);
}


void InsetIPA::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {
	case LFUN_QUOTE_INSERT: {
		FuncRequest fr(LFUN_SELF_INSERT, "\"");
		InsetText::doDispatch(cur, fr);
		break;
	}
	default:
		InsetText::doDispatch(cur, cmd);
		break;
	}

}


bool InsetIPA::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action()) {
	case LFUN_SCRIPT_INSERT: {
		if (cmd.argument() == "subscript") {
			flag.setEnabled(false);
			return true;
		}
		break;
	}
	case LFUN_IN_IPA:
		flag.setEnabled(true);
		return true;
		break;
	default:
		break;
	}
	return InsetText::getStatus(cur, cmd, flag);
}


void InsetIPA::addPreview(DocIterator const & inset_pos,
	graphics::PreviewLoader &) const
{
	preparePreview(inset_pos);
}


void InsetIPA::preparePreview(DocIterator const & pos) const  
{
	TexRow texrow;
	odocstringstream str;  
	otexstream os(str, texrow);
	OutputParams runparams(&pos.buffer()->params().encoding());
	latex(os, runparams);
	docstring const snippet = str.str();
	preview_->addPreview(snippet, *pos.buffer());  
}


bool InsetIPA::previewState(BufferView * bv) const
{
	if (!editing(bv) && RenderPreview::previewText()) {
		graphics::PreviewImage const * pimage =
			preview_->getPreviewImage(bv->buffer());
		return pimage && pimage->image();
	}
	return false;
}


void InsetIPA::reloadPreview(DocIterator const & pos) const
{
	preparePreview(pos);
	preview_->startLoading(*pos.buffer());
}


void InsetIPA::draw(PainterInfo & pi, int x, int y) const
{
	use_preview_ = previewState(pi.base.bv);

	if (use_preview_) {
		preview_->draw(pi, x + TEXT_TO_INSET_OFFSET, y);
		setPosCache(pi, x, y);
		return;
	}
	InsetText::draw(pi, x, y);
}


void InsetIPA::edit(Cursor & cur, bool front, EntryDirection entry_from)
{
	cur.push(*this);
	InsetText::edit(cur, front, entry_from);
}


Inset * InsetIPA::editXY(Cursor & cur, int x, int y)
{
	if (use_preview_) {
		edit(cur, true, ENTRY_DIRECTION_IGNORE);
		return this;
	}
	cur.push(*this);
	return InsetText::editXY(cur, x, y);
}


void InsetIPA::metrics(MetricsInfo & mi, Dimension & dim) const
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


bool InsetIPA::notifyCursorLeaves(Cursor const & old, Cursor & cur)
{
	reloadPreview(old);
	cur.screenUpdateFlags(Update::Force);
	return InsetText::notifyCursorLeaves(old, cur);
}


void InsetIPA::validate(LaTeXFeatures & features) const
{
	features.require("tipa");
	features.require("tipx");

	InsetText::validate(features);
}


void InsetIPA::latex(otexstream & os, OutputParams const & runparams_in) const
{
	OutputParams runparams(runparams_in);
	runparams.inIPA = true;
	bool const multipar = (text().paragraphs().size() > 1);
	// fontspec knows \textipa, but not the IPA environment
	bool const nontexfonts = buffer_->params().useNonTeXFonts;
	if (multipar && !nontexfonts)
		os << "\\begin{IPA}\n";
	else
		os << "\\textipa{";
	InsetText::latex(os, runparams);
	if (multipar && !nontexfonts)
		os << "\n\\end{IPA}";
	else
		os << "}";
}


docstring InsetIPA::xhtml(XHTMLStream & xs, OutputParams const & runparams_in) const
{
	OutputParams runparams(runparams_in);
	runparams.inIPA = true;
	return InsetText::xhtml(xs, runparams);
}


bool InsetIPA::insetAllowed(InsetCode code) const
{
	switch (code) {
	// code that is allowed
	case ERT_CODE:
	case IPACHAR_CODE:
	case IPADECO_CODE:
	case SCRIPT_CODE:
		return true;
	default:
		return false;
	}
}


} // namespace lyx
