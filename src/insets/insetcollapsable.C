/**
 * \file insetcollapsable.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetcollapsable.h"

#include "buffer.h"
#include "BufferView.h"
#include "cursor.h"
#include "debug.h"
#include "dispatchresult.h"
#include "LColor.h"
#include "lyxlex.h"
#include "funcrequest.h"
#include "metricsinfo.h"
#include "paragraph.h"

#include "frontends/font_metrics.h"
#include "frontends/Painter.h"
#include "frontends/LyXView.h"


using lyx::graphics::PreviewLoader;

using std::endl;
using std::string;
using std::max;
using std::ostream;


InsetCollapsable::InsetCollapsable(BufferParams const & bp, bool collapsed)
	: UpdatableInset(), inset(bp), collapsed_(collapsed), label("Label")
#if 0
	,autocollapse(false)
#endif
{
	inset.setOwner(this);
	inset.setAutoBreakRows(true);
	inset.setDrawFrame(InsetText::ALWAYS);
	inset.setFrameColor(LColor::collapsableframe);
	setInsetName("Collapsable");
}


InsetCollapsable::InsetCollapsable(InsetCollapsable const & in)
	: UpdatableInset(in), inset(in.inset), collapsed_(in.collapsed_),
	  labelfont_(in.labelfont_), label(in.label)
#if 0
	  ,autocollapse(in.autocollapse)
#endif
{
	inset.setOwner(this);
}


bool InsetCollapsable::insertInset(BufferView * bv, InsetOld * in)
{
	if (!insetAllowed(in->lyxCode())) {
		lyxerr << "InsetCollapsable::InsertInset: "
			"Unable to insert inset." << endl;
		return false;
	}
	return inset.insertInset(bv, in);
}


void InsetCollapsable::write(Buffer const & buf, ostream & os) const
{
	os << "collapsed " << (collapsed_ ? "true" : "false") << "\n";
	inset.writeParagraphData(buf, os);
}


void InsetCollapsable::read(Buffer const & buf, LyXLex & lex)
{
	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token == "collapsed") {
			lex.next();
			collapsed_ = lex.getBool();
		} else {
			lyxerr << "InsetCollapsable::Read: Missing collapsed!"
			       << endl;
			// Take countermeasures
			lex.pushToken(token);
		}
	}
	inset.read(buf, lex);
}


void InsetCollapsable::dimension_collapsed(Dimension & dim) const
{
	font_metrics::buttonText(label, labelfont_, dim.wid, dim.asc, dim.des);
}


int InsetCollapsable::height_collapsed() const
{
	Dimension dim;
	font_metrics::buttonText(label, labelfont_, dim.wid, dim.asc, dim.des);
	return dim.asc + dim.des;
}


void InsetCollapsable::metrics(MetricsInfo & mi, Dimension & dim) const
{
	//lyxerr << "InsetCollapsable::metrics:  width: " << mi.base.textwidth << endl;
	dimension_collapsed(dim);
	if (!collapsed_) {
		Dimension insetdim;
		inset.metrics(mi, insetdim);
		dim.des += insetdim.height() + TEXT_TO_BOTTOM_OFFSET;
		dim.wid = max(dim.wid, insetdim.wid);
	}
	dim_ = dim;
	//lyxerr << "InsetCollapsable::metrics:  dim.wid: " << dim.wid << endl;
}


void InsetCollapsable::draw_collapsed(PainterInfo & pi, int x, int y) const
{
	pi.pain.buttonText(x, y, label, labelfont_);
}


void InsetCollapsable::draw(PainterInfo & pi, int x, int y, bool inlined) const
{
	Dimension dim_collapsed;
	dimension_collapsed(dim_collapsed);

	int const aa  = ascent();
	button_dim.x1 = 0;
	button_dim.x2 = dim_collapsed.width();
	button_dim.y1 = -aa;
	button_dim.y2 = -aa + dim_collapsed.height();

	top_x = x;
	top_baseline = y;

	if (!isOpen()) {
		draw_collapsed(pi, x, y);
		return;
	}

	int old_x = x;

	if (!owner())
		x += scroll();

	if (inlined) {
		inset.draw(pi, x, y);
	} else {
		int const bl = y - aa + dim_collapsed.ascent();
		draw_collapsed(pi, old_x, bl);
		inset.draw(pi, x, bl + dim_collapsed.descent() + inset.ascent());
	}
}


void InsetCollapsable::draw(PainterInfo & pi, int x, int y) const
{
	// by default, we are not inlined-drawing
	draw(pi, x, y, false);
}


InsetOld::EDITABLE InsetCollapsable::editable() const
{
	return collapsed_ ? IS_EDITABLE : HIGHLY_EDITABLE;
}


FuncRequest InsetCollapsable::adjustCommand(FuncRequest const & cmd)
{
	FuncRequest cmd1 = cmd;
	cmd1.y = ascent() + cmd.y - height_collapsed() - inset.ascent();
	return cmd1;
}


DispatchResult InsetCollapsable::lfunMouseRelease(FuncRequest const & cmd)
{
	DispatchResult result(true, true);
	BufferView * bv = cmd.view();

	if (cmd.button() == mouse_button::button3) {
		lyxerr << "InsetCollapsable::lfunMouseRelease 0" << endl;
		if (hitButton(cmd))
			showInsetDialog(bv);
	} else {
		if (collapsed_) {
			lyxerr << "InsetCollapsable::lfunMouseRelease 1" << endl;
			collapsed_ = false;
			edit(bv, true);
			bv->buffer()->markDirty();
			bv->updateInset(this);
			bv->update();
			return result;
		}

		if (hitButton(cmd)) {
			if (collapsed_) {
				lyxerr << "InsetCollapsable::lfunMouseRelease 2" << endl;
				collapsed_ = false;
			} else {
				collapsed_ = true;
				result.update(true);
				result.val(FINISHED_RIGHT);
				return result;
			}
			result.update(true);
			bv->updateInset(this);
			bv->buffer()->markDirty();
		} else if (!collapsed_ && cmd.y > button_dim.y2) {
			lyxerr << "InsetCollapsable::lfunMouseRelease 3" << endl;
			result = inset.dispatch(adjustCommand(cmd));
		}
	}
	lyxerr << "InsetCollapsable::lfunMouseRelease 4" << endl;
	return result;
}


int InsetCollapsable::latex(Buffer const & buf, ostream & os,
			    OutputParams const & runparams) const
{
	return inset.latex(buf, os, runparams);
}


int InsetCollapsable::plaintext(Buffer const & buf, ostream & os,
			    OutputParams const & runparams) const
{
	return inset.plaintext(buf, os, runparams);
}


int InsetCollapsable::linuxdoc(Buffer const & buf, ostream & os,
			       OutputParams const & runparams) const
{
	return inset.linuxdoc(buf, os, runparams);
}


int InsetCollapsable::docbook(Buffer const & buf, ostream & os,
			      OutputParams const & runparams) const
{
	return inset.docbook(buf, os, runparams);
}


bool InsetCollapsable::hitButton(FuncRequest const & cmd) const
{
	return button_dim.contains(cmd.x, cmd.y);
}


void InsetCollapsable::edit(BufferView * bv, bool left)
{
	lyxerr << "InsetCollapsable: edit left/right" << endl;
	bv->cursor().push(this);
	inset.edit(bv, left);
	open(bv);
}


void InsetCollapsable::edit(BufferView * bv, int x, int y)
{
	lyxerr << "InsetCollapsable: edit xy" << endl;
	if (collapsed_) {
		collapsed_ = false;
		// set this only here as it should be recollapsed only if
		// it was already collapsed!
		bv->updateInset(this);
		bv->buffer()->markDirty();
		inset.edit(bv, x, y);
	} else {
		if (y <= button_dim.y2)
			inset.edit(bv, x, 0);
		else
			inset.edit(bv, x,
				ascent() + y - height_collapsed() + inset.ascent());
	}
	bv->cursor().push(this);
}


DispatchResult
InsetCollapsable::priv_dispatch(FuncRequest const & cmd, idx_type &, pos_type &)
{
	lyxerr << "\nInsetCollapsable::priv_dispatch (begin): cmd: " << cmd
		<< "  button y: " << button_dim.y2 << endl;
	switch (cmd.action) {
		case LFUN_MOUSE_PRESS:
			if (!collapsed_ && cmd.y > button_dim.y2)
				inset.dispatch(adjustCommand(cmd));
			return DispatchResult(true, true);

		case LFUN_MOUSE_MOTION:
			if (!collapsed_ && cmd.y > button_dim.y2)
				inset.dispatch(adjustCommand(cmd));
			return DispatchResult(true, true);

		case LFUN_MOUSE_RELEASE:
			if (!collapsed_ && cmd.y > button_dim.y2)
				inset.dispatch(adjustCommand(cmd));
			else
				return lfunMouseRelease(cmd);
			return DispatchResult(true, true);

		default:
			return inset.dispatch(cmd);
	}
	lyxerr << "InsetCollapsable::priv_dispatch (end)" << endl;
}


int InsetCollapsable::insetInInsetY() const
{
	return inset.y() - top_baseline + inset.insetInInsetY();
}


void InsetCollapsable::validate(LaTeXFeatures & features) const
{
	inset.validate(features);
}


void InsetCollapsable::getCursorPos(BufferView * bv, int & x, int & y) const
{
	inset.getCursorPos(bv, x , y);
	y += - ascent() + height_collapsed() + inset.ascent();
}


void InsetCollapsable::setFont(BufferView * bv, LyXFont const & font,
			       bool toggleall, bool selectall)
{
	inset.setFont(bv, font, toggleall, selectall);
}


void InsetCollapsable::deleteLyXText(BufferView * bv, bool recursive) const
{
	inset.deleteLyXText(bv, recursive);
}


void InsetCollapsable::getLabelList(Buffer const & buffer,
				    std::vector<string> & list) const
{
	inset.getLabelList(buffer, list);
}


int InsetCollapsable::scroll(bool recursive) const
{
	int sx = UpdatableInset::scroll(false);

	if (recursive)
		sx += inset.scroll(recursive);

	return sx;
}


ParagraphList * InsetCollapsable::getParagraphs(int i) const
{
	return inset.getParagraphs(i);
}


int InsetCollapsable::numParagraphs() const
{
	return inset.numParagraphs();
}


LyXText * InsetCollapsable::getText(int i) const
{
	return inset.getText(i);
}


void InsetCollapsable::open(BufferView * bv)
{
	if (!collapsed_)
		return;

	collapsed_ = false;
	bv->updateInset(this);
}


void InsetCollapsable::close(BufferView * bv) const
{
	if (collapsed_)
		return;

	collapsed_ = true;
	bv->updateInset(this);
}


void InsetCollapsable::setLabel(string const & l) const
{
	label = l;
}


void InsetCollapsable::setCollapsed(bool c) const
{
	collapsed_ = c;
}


void InsetCollapsable::markErased()
{
	inset.markErased();
}


void InsetCollapsable::addPreview(PreviewLoader & loader) const
{
	inset.addPreview(loader);
}


bool InsetCollapsable::insetAllowed(InsetOld::Code code) const
{
	return inset.insetAllowed(code);
}


void InsetCollapsable::fitInsetCursor(BufferView * bv) const
{
	inset.fitInsetCursor(bv);
}


void InsetCollapsable::setLabelFont(LyXFont & f)
{
	labelfont_ = f;
}

#if 0
void InsetCollapsable::setAutoCollapse(bool f)
{
	autocollapse = f;
}
#endif

void InsetCollapsable::scroll(BufferView *bv, float sx) const
{
	UpdatableInset::scroll(bv, sx);
}


void InsetCollapsable::scroll(BufferView *bv, int offset) const
{
	UpdatableInset::scroll(bv, offset);
}


bool InsetCollapsable::isOpen() const
{
	return !collapsed_;
}


Box const & InsetCollapsable::buttonDim() const
{
	return button_dim;
}

