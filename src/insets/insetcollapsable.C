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


InsetCollapsable::InsetCollapsable(BufferParams const & bp,
	CollapseStatus status)
	: inset(bp), label("Label"), status_(status), openinlined_(false)
{
	inset.setOwner(this);
	inset.setAutoBreakRows(true);
	inset.setDrawFrame(InsetText::ALWAYS);
	inset.setFrameColor(LColor::collapsableframe);
	setInsetName("Collapsable");
	setButtonLabel();
}


InsetCollapsable::InsetCollapsable(InsetCollapsable const & in)
	: UpdatableInset(in), inset(in.inset),
	  labelfont_(in.labelfont_), label(in.label), status_(in.status_)
{
	inset.setOwner(this);
	setButtonLabel();
}


void InsetCollapsable::write(Buffer const & buf, ostream & os) const
{
	os << "status ";
	switch (status_) {
	case Open:
		os << "open";
		break;
	case Collapsed:
		os << "collapsed";
		break;
	case Inlined:
		os << "inlined";
		break;
	}
	os << "\n";
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

	if (!token_found)
		status_ = isOpen() ? Open : Collapsed;

	setButtonLabel();
}


void InsetCollapsable::dimension_collapsed(Dimension & dim) const
{
	font_metrics::buttonText(label, labelfont_, dim.wid, dim.asc, dim.des);
}


void InsetCollapsable::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (status_ == Inlined) {
		inset.metrics(mi, dim);
	} else {
		dimension_collapsed(dim);
		if (status_ == Open) {
			Dimension insetdim;
			inset.metrics(mi, insetdim);
			openinlined_ = (insetdim.wid + dim.wid <= mi.base.textwidth);
			if (openinlined_) {
				dim.wid += insetdim.wid;
				dim.des = max(dim.des, insetdim.des);
				dim.asc = max(dim.asc, insetdim.asc);
			} else {
				dim.des += insetdim.height()
					+ TEXT_TO_BOTTOM_OFFSET;
				dim.wid = max(dim.wid, insetdim.wid);
			}
		}
	}
	dim_ = dim;
}


void InsetCollapsable::draw_collapsed(PainterInfo & pi, int x, int y) const
{
	pi.pain.buttonText(x, y, label, labelfont_);
}


void InsetCollapsable::draw(PainterInfo & pi, int x, int y) const
{
	setPosCache(pi, x, y);

	if (status_ == Inlined) {
		inset.draw(pi, x, y);
	} else {
		Dimension dimc;
		dimension_collapsed(dimc);
		int const aa  = ascent();
		button_dim.x1 = x + 0;
		button_dim.x2 = x + dimc.width();
		button_dim.y1 = y - aa + pi.base.bv->top_y();
		button_dim.y2 = y - aa + pi.base.bv->top_y() + dimc.height();

		draw_collapsed(pi, x, y);
		if (status_ == Open) {
			if (!owner())
				x += scroll();
			
			if (openinlined_)
				inset.draw(pi, x + dimc.width(), y - aa + inset.ascent());
			else 
				inset.draw(pi, x, y - aa + dimc.height() + inset.ascent());
		}
	}
}


void InsetCollapsable::drawSelection(PainterInfo & pi, int x, int y) const
{
	inset.drawSelection(pi, x, y);
}


InsetOld::EDITABLE InsetCollapsable::editable() const
{
	return status_ != Collapsed ? HIGHLY_EDITABLE : IS_EDITABLE;
}


bool InsetCollapsable::descendable() const
{
	return status_ != Collapsed;
}


void InsetCollapsable::lfunMouseRelease(LCursor & cur, FuncRequest & cmd)
{
	if (cmd.button() == mouse_button::button3) {
		showInsetDialog(&cur.bv());
		return;
	}

	switch (status_) {

	case Collapsed:
		lyxerr << "InsetCollapsable::lfunMouseRelease 1" << endl;
		setStatus(Open);
		edit(cur, true);
		break;

	case Open: {
		FuncRequest cmd1 = cmd;
		if (hitButton(cmd1)) {
			lyxerr << "InsetCollapsable::lfunMouseRelease 2" << endl;
			setStatus(Collapsed);
			cmd = FuncRequest(LFUN_FINISHED_RIGHT);
			break;
		}
		lyxerr << "InsetCollapsable::lfunMouseRelease 3" << endl;
		inset.dispatch(cur, cmd);
		break;
	}

	case Inlined:
		inset.dispatch(cur, cmd);
		break;
	}
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


bool InsetCollapsable::hitButton(FuncRequest & cmd) const
{
	return button_dim.contains(cmd.x, cmd.y);
}


string const InsetCollapsable::getNewLabel(string const & l) const
{
	string label;
	pos_type const max_length = 15;
	pos_type const p_siz = inset.paragraphs().begin()->size();
	pos_type const n = min(max_length, p_siz);
	pos_type i = 0;
	pos_type j = 0;
	for( ; i < n && j < p_siz; ++j) {
		if (inset.paragraphs().begin()->isInset(j))
			continue;
		label += inset.paragraphs().begin()->getChar(j);
		++i;
	}
	if (inset.paragraphs().size() > 1 || (i > 0 && j < p_siz)) {
		label += "...";
	}
	return label.empty() ? l : label;
}


void InsetCollapsable::edit(LCursor & cur, bool left)
{
	//lyxerr << "InsetCollapsable: edit left/right" << endl;
	cur.push(*this);
	inset.edit(cur, left);
	open();
}


InsetBase * InsetCollapsable::editXY(LCursor & cur, int x, int y)
{
	cur.push(*this);
	//lyxerr << "InsetCollapsable: edit xy" << endl;
	if (status_ == Collapsed) {
		setStatus(Open);
		inset.edit(cur, true);
#warning look here
//we are not calling edit(x,y) because there are no coordinates in the
//inset yet. I personally think it's ok. (ab)
		return this;
	}
	return inset.editXY(cur, x, y);
}


void InsetCollapsable::priv_dispatch(LCursor & cur, FuncRequest & cmd)
{
	//lyxerr << "\nInsetCollapsable::priv_dispatch (begin): cmd: " << cmd
	//	<< "  button y: " << button_dim.y2 << endl;
	switch (cmd.action) {
		case LFUN_MOUSE_PRESS:
			if (status_ == Inlined)
				inset.dispatch(cur, cmd);
			else if (status_ == Open && !hitButton(cmd))
				inset.dispatch(cur, cmd);
			break;

		case LFUN_MOUSE_MOTION:
			if (status_ == Inlined)
				inset.dispatch(cur, cmd);
			else if (status_ == Open && !hitButton(cmd))
				inset.dispatch(cur, cmd);
			break;

		case LFUN_MOUSE_RELEASE:
			lfunMouseRelease(cur, cmd);
			break;

		case LFUN_INSET_TOGGLE:
			if (inset.text_.toggleInset(cur))
				break;
			if (status_ == Open) {
				setStatus(Inlined);
				break;
			}
			setStatus(Collapsed);
			cmd = FuncRequest(LFUN_FINISHED_RIGHT);
			break;

		default:
			inset.dispatch(cur, cmd);
			break;
	}
}


void InsetCollapsable::validate(LaTeXFeatures & features) const
{
	inset.validate(features);
}


void InsetCollapsable::getCursorPos(CursorSlice const & cur,
	int & x, int & y) const
{
	inset.getCursorPos(cur, x, y);
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


size_t InsetCollapsable::nargs() const
{
	return inset.nargs();
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


void InsetCollapsable::scroll(BufferView & bv, float sx) const
{
	UpdatableInset::scroll(bv, sx);
}


void InsetCollapsable::scroll(BufferView & bv, int offset) const
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
