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
using std::min;
using std::ostream;


InsetCollapsable::InsetCollapsable(BufferParams const & bp, CollapseStatus status)
	: UpdatableInset(), inset(bp), status_(status),
	  label("Label")
{
	inset.setOwner(this);
	inset.setAutoBreakRows(true);
	inset.setDrawFrame(InsetText::ALWAYS);
	inset.setFrameColor(LColor::collapsableframe);
	setInsetName("Collapsable");

	setButtonLabel();
}


InsetCollapsable::InsetCollapsable(InsetCollapsable const & in)
	: UpdatableInset(in), inset(in.inset), status_(in.status_),
	  labelfont_(in.labelfont_), label(in.label)
{
	inset.setOwner(this);
	setButtonLabel();
}


void InsetCollapsable::write(Buffer const & buf, ostream & os) const
{
	string st;

	switch (status_) {
	case Open:
		st = "open";
		break;
	case Collapsed:
		st = "collapsed";
		break;
	case Inlined:
		st = "inlined";
		break;
	}
	os << "status " << st << "\n";
	inset.text_.write(buf, os);
}


void InsetCollapsable::read(Buffer const & buf, LyXLex & lex)
{
	bool token_found = false;
	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token == "status") {
			lex.next();
			string const tmp_token = lex.getString();

			if (tmp_token == "inlined") {
				status_ = Inlined;
				token_found = true;
			} else if (tmp_token == "collapsed") {
				status_ = Collapsed;
				token_found = true;
			} else if (tmp_token == "open") {
				status_ = Open;
				token_found = true;
			} else {
				lyxerr << "InsetCollapsable::read: Missing status!"
				       << endl;
				// Take countermeasures
				lex.pushToken(token);
			}
		} else {
			lyxerr << "InsetCollapsable::Read: Missing 'status'-tag!"
				   << endl;
			// take countermeasures
			lex.pushToken(token);
		}
	}
	inset.read(buf, lex);

	if (!token_found) {
		if (isOpen())
			status_ = Open;
		else
			status_ = Collapsed;
	}

	setButtonLabel();
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
	if (status_ == Inlined) {
		inset.metrics(mi, dim);
	} else {
		dimension_collapsed(dim);
		if (status_ == Open) {
			Dimension insetdim;
			inset.metrics(mi, insetdim);
			dim.des += insetdim.height() + TEXT_TO_BOTTOM_OFFSET;
			dim.wid = max(dim.wid, insetdim.wid);
		}
	}
	dim_ = dim;
	//lyxerr << "InsetCollapsable::metrics:  dim.wid: " << dim.wid << endl;
}


void InsetCollapsable::draw_collapsed(PainterInfo & pi, int x, int y) const
{
	pi.pain.buttonText(x, y, label, labelfont_);
}


void InsetCollapsable::draw(PainterInfo & pi, int x, int y) const
{
	xo_ = x;
	yo_ = y;

	if (status_ == Inlined) {
		inset.draw(pi, x, y);
	} else {
		Dimension dimc;
		dimension_collapsed(dimc);

		int const aa  = ascent();
		button_dim.x1 = 0;
		button_dim.x2 = dimc.width();
		button_dim.y1 = -aa;
		button_dim.y2 = -aa + dimc.height();

		draw_collapsed(pi, x, y);
		if (status_ == Open) {
			if (!owner())
				x += scroll();
			inset.draw(pi, x, y - aa + dimc.height() + inset.ascent());
		}
	}
}


InsetOld::EDITABLE InsetCollapsable::editable() const
{
	return status_ != Collapsed ? HIGHLY_EDITABLE : IS_EDITABLE;
}


bool InsetCollapsable::descendable() const
{
	return status_ != Collapsed;
}


FuncRequest InsetCollapsable::adjustCommand(FuncRequest const & cmd)
{
	FuncRequest cmd1 = cmd;
	cmd1.y += ascent() - height_collapsed();
	return cmd1;
}


DispatchResult InsetCollapsable::lfunMouseRelease(FuncRequest const & cmd)
{
	BufferView * bv = cmd.view();

	if (cmd.button() == mouse_button::button3) {
		lyxerr << "InsetCollapsable::lfunMouseRelease 0" << endl;
		showInsetDialog(bv);
		return DispatchResult(true, true);
	}

	switch (status_) {
	case Collapsed:
		lyxerr << "InsetCollapsable::lfunMouseRelease 1" << endl;
		setStatus(Open);
		edit(bv, true);
		return DispatchResult(true, true);

	case Open:
		if (hitButton(cmd)) {
			lyxerr << "InsetCollapsable::lfunMouseRelease 2" << endl;
			setStatus(Collapsed);
			return DispatchResult(false, FINISHED_RIGHT);
		} else {
			lyxerr << "InsetCollapsable::lfunMouseRelease 3" << endl;
			return inset.dispatch(adjustCommand(cmd));
		}	

	case Inlined:
		return inset.dispatch(cmd);
	}

	return DispatchResult(true, true);
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


string const InsetCollapsable::getNewLabel(string const & l) const
{
	string la;
	pos_type const max_length = 15;
	pos_type const p_siz = inset.paragraphs().begin()->size();
	pos_type const n = min(max_length, p_siz);
	pos_type i = 0;
	pos_type j = 0;
	for( ; i < n && j < p_siz; ++j) {
		if (inset.paragraphs().begin()->isInset(j))
			continue;
		la += inset.paragraphs().begin()->getChar(j);
		++i;
	}
	if (inset.paragraphs().size() > 1 || (i > 0 && j < p_siz)) {
		la += "...";
	}
	return la.empty() ? l : la;
}


void InsetCollapsable::edit(BufferView * bv, bool left)
{
	lyxerr << "InsetCollapsable: edit left/right" << endl;
	inset.edit(bv, left);
	open();
	bv->cursor().push(this);
}


void InsetCollapsable::edit(BufferView * bv, int x, int y)
{
	lyxerr << "InsetCollapsable: edit xy" << endl;
	if (status_ == Collapsed) {
		setStatus(Open);
	} else {
		if (y <= button_dim.y2)
			y = 0;
		else
			y += inset.ascent() - height_collapsed();
	}
	inset.edit(bv, x, y);
	bv->cursor().push(this);
}


DispatchResult
InsetCollapsable::priv_dispatch(FuncRequest const & cmd, idx_type &, pos_type &)
{
	//lyxerr << "\nInsetCollapsable::priv_dispatch (begin): cmd: " << cmd
	//	<< "  button y: " << button_dim.y2 << endl;
	switch (cmd.action) {
		case LFUN_MOUSE_PRESS:
			if (status_ == Inlined)
				inset.dispatch(cmd);
			else if (status_ == Open && cmd.y > button_dim.y2)
				inset.dispatch(adjustCommand(cmd));
			return DispatchResult(true, true);

		case LFUN_MOUSE_MOTION:
			if (status_ == Inlined)
				inset.dispatch(cmd);
			else if (status_ == Open && cmd.y > button_dim.y2)
				inset.dispatch(adjustCommand(cmd));
			return DispatchResult(true, true);

		case LFUN_MOUSE_RELEASE:
			return lfunMouseRelease(cmd);

		case LFUN_INSET_TOGGLE:
			if (inset.text_.toggleInset())
				return DispatchResult(true, true);
			if (status_ == Open)
				setStatus(Inlined);
				return DispatchResult(true, true);
			setStatus(Collapsed);
			return DispatchResult(false, FINISHED_RIGHT);

		default:
			return inset.dispatch(adjustCommand(cmd));
	}
	//lyxerr << "InsetCollapsable::priv_dispatch (end)" << endl;
}


void InsetCollapsable::validate(LaTeXFeatures & features) const
{
	inset.validate(features);
}


void InsetCollapsable::getCursorPos(int & x, int & y) const
{
	inset.getCursorPos(x, y);
	if (status_ != Inlined)
		y += - ascent() + height_collapsed() + inset.ascent();
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
		sx += inset.scroll(false);

	return sx;
}


int InsetCollapsable::numParagraphs() const
{
	return inset.numParagraphs();
}


LyXText * InsetCollapsable::getText(int i) const
{
	return inset.getText(i);
}


void InsetCollapsable::open()
{
	if (status_ == Collapsed)   // ...but not inlined
		setStatus(Open);
}


void InsetCollapsable::close()
{
	setStatus(Collapsed);
}


void InsetCollapsable::setLabel(string const & l)
{
	label = l;
}


void InsetCollapsable::setStatus(CollapseStatus st)
{
	status_ = st;
	setButtonLabel();
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


void InsetCollapsable::setLabelFont(LyXFont & font)
{
	labelfont_ = font;
}


void InsetCollapsable::scroll(BufferView * bv, float sx) const
{
	UpdatableInset::scroll(bv, sx);
}


void InsetCollapsable::scroll(BufferView * bv, int offset) const
{
	UpdatableInset::scroll(bv, offset);
}


Box const & InsetCollapsable::buttonDim() const
{
	return button_dim;
}


void InsetCollapsable::setBackgroundColor(LColor_color color)
{
	InsetOld::setBackgroundColor(color);
	inset.setBackgroundColor(color);
}
