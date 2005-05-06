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
#include "FuncStatus.h"
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


InsetCollapsable::InsetCollapsable
		(BufferParams const & bp, CollapseStatus status)
	: InsetText(bp), label("Label"), status_(status), openinlined_(false)
{
	setAutoBreakRows(true);
	setDrawFrame(true);
	setFrameColor(LColor::collapsableframe);
	setInsetName("Collapsable");
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
	text_.write(buf, os);
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
			lyxerr << "InsetCollapsable::read: Missing 'status'-tag!"
				   << endl;
			// take countermeasures
			lex.pushToken(token);
		}
	}
	InsetText::read(buf, lex);

	if (!token_found)
		status_ = isOpen() ? Open : Collapsed;

	setButtonLabel();
}


Dimension InsetCollapsable::dimensionCollapsed() const
{
	Dimension dim;
	font_metrics::buttonText(label, labelfont_, dim.wid, dim.asc, dim.des);
	return dim;
}


void InsetCollapsable::metrics(MetricsInfo & mi, Dimension & dim) const
{
	mi.base.textwidth -= 2 * TEXT_TO_INSET_OFFSET;
	if (status_ == Inlined) {
		InsetText::metrics(mi, dim);
	} else {
		dim = dimensionCollapsed();
		if (status_ == Open) {
			InsetText::metrics(mi, textdim_);
			openinlined_ = (textdim_.wid + dim.wid <= mi.base.textwidth);
			if (openinlined_) {
				dim.wid += textdim_.wid;
				dim.des = max(dim.des - textdim_.asc + dim.asc, textdim_.des);
				dim.asc = textdim_.asc;
			} else {
				dim.des += textdim_.height() + TEXT_TO_BOTTOM_OFFSET;
				dim.wid = max(dim.wid, textdim_.wid);
			}
		}
	}
	dim.asc += TEXT_TO_INSET_OFFSET;
	dim.des += TEXT_TO_INSET_OFFSET;
	dim.wid += 2 * TEXT_TO_INSET_OFFSET;
	mi.base.textwidth += 2 * TEXT_TO_INSET_OFFSET;
	dim_ = dim;
}


void InsetCollapsable::draw(PainterInfo & pi, int x, int y) const
{
	const int xx = x + TEXT_TO_INSET_OFFSET;
	if (status_ == Inlined) {
		InsetText::draw(pi, xx, y);
	} else {
		Dimension dimc = dimensionCollapsed();
		int const top  = y - ascent() + TEXT_TO_INSET_OFFSET;
		button_dim.x1 = xx + 0;
		button_dim.x2 = xx + dimc.width();
		button_dim.y1 = top;
		button_dim.y2 = top + dimc.height();

		pi.pain.buttonText(xx, top + dimc.asc, label, labelfont_);
		if (status_ == Open) {
			int textx, texty;
			if (openinlined_) {
				textx = xx + dimc.width();
				texty = top + textdim_.asc;
			} else {
				textx = xx;
				texty = top + dimc.height() + textdim_.asc;
			}
			InsetText::draw(pi, textx, texty);
		}
	}
	setPosCache(pi, x, y);
}


void InsetCollapsable::drawSelection(PainterInfo & pi, int x, int y) const
{
	x += TEXT_TO_INSET_OFFSET;
	if (status_ == Open) {
		if (openinlined_)
			x += dimensionCollapsed().wid;
		else
			y += dimensionCollapsed().des + textdim_.asc;
	}
	if (status_ != Collapsed)
		InsetText::drawSelection(pi, x, y);
}


void InsetCollapsable::getCursorPos
	(CursorSlice const & sl, int & x, int & y) const
{
	if (status_ == Collapsed) {
		x = xo();
		y = yo();
		return;
	}

	InsetText::getCursorPos(sl, x, y);
	if (status_ == Open) {
		if (openinlined_)
			x += dimensionCollapsed().wid;
		else
			y += dimensionCollapsed().height() - ascent() + TEXT_TO_INSET_OFFSET + textdim_.asc;
	}

	x += TEXT_TO_INSET_OFFSET;
}


InsetBase::EDITABLE InsetCollapsable::editable() const
{
	return status_ != Collapsed ? HIGHLY_EDITABLE : IS_EDITABLE;
}


bool InsetCollapsable::descendable() const
{
	return status_ != Collapsed;
}


bool InsetCollapsable::hitButton(FuncRequest const & cmd) const
{
	return button_dim.contains(cmd.x, cmd.y);
}


string const InsetCollapsable::getNewLabel(string const & l) const
{
	string label;
	pos_type const max_length = 15;
	pos_type const p_siz = paragraphs().begin()->size();
	pos_type const n = min(max_length, p_siz);
	pos_type i = 0;
	pos_type j = 0;
	for( ; i < n && j < p_siz; ++j) {
		if (paragraphs().begin()->isInset(j))
			continue;
		label += paragraphs().begin()->getChar(j);
		++i;
	}
	if (paragraphs().size() > 1 || (i > 0 && j < p_siz)) {
		label += "...";
	}
	return label.empty() ? l : label;
}


void InsetCollapsable::edit(LCursor & cur, bool left)
{
	//lyxerr << "InsetCollapsable: edit left/right" << endl;
	cur.push(*this);
	InsetText::edit(cur, left);
	open();
}


InsetBase * InsetCollapsable::editXY(LCursor & cur, int x, int y) const
{
	//lyxerr << "InsetCollapsable: edit xy" << endl;
	if (status_ == Collapsed) {
		return const_cast<InsetCollapsable*>(this);
	}
	cur.push(const_cast<InsetCollapsable&>(*this));
	return InsetText::editXY(cur, x, y);
}


void InsetCollapsable::doDispatch(LCursor & cur, FuncRequest & cmd)
{
	lyxerr << "InsetCollapsable::doDispatch (begin): cmd: " << cmd
		<< " cur: " << cur << " bvcur: " << cur.bv().cursor() << endl;

	switch (cmd.action) {
	case LFUN_MOUSE_PRESS:
		if (status_ == Inlined)
			InsetText::doDispatch(cur, cmd);
		else if (status_ == Open && !hitButton(cmd))
			InsetText::doDispatch(cur, cmd);
		else
			cur.noUpdate();
		break;

	case LFUN_MOUSE_MOTION:
	case LFUN_MOUSE_DOUBLE:
	case LFUN_MOUSE_TRIPLE:
		if (status_ == Inlined)
			InsetText::doDispatch(cur, cmd);
		else if (status_ == Open && !hitButton(cmd))
			InsetText::doDispatch(cur, cmd);
		else
			cur.undispatched();
		break;

	case LFUN_MOUSE_RELEASE:
		if (cmd.button() == mouse_button::button3) {
			showInsetDialog(&cur.bv());
			break;
		}

		switch (status_) {

		case Collapsed:
			lyxerr << "InsetCollapsable::lfunMouseRelease 1" << endl;
			edit(cur, true);
			cur.bv().cursor() = cur;
			break;

		case Open: {
			if (hitButton(cmd)) {
				lyxerr << "InsetCollapsable::lfunMouseRelease 2" << endl;
				setStatus(cur, Collapsed);
				cur.bv().cursor() = cur;
			} else {
				lyxerr << "InsetCollapsable::lfunMouseRelease 3" << endl;
				InsetText::doDispatch(cur, cmd);
			}
			break;
		}

		case Inlined:
			lyxerr << "InsetCollapsable::lfunMouseRelease 4" << endl;
			InsetText::doDispatch(cur, cmd);
			break;
		}
		break;

	case LFUN_INSET_TOGGLE:
		if (cmd.argument == "open")
			setStatus(cur, Open);
		else if (cmd.argument == "close")
			setStatus(cur, Collapsed);
		else if (cmd.argument == "toggle" || cmd.argument.empty())
			setStatus(cur, isOpen() ? Collapsed : Open);
		else // if assign or anything else
			cur.undispatched();
		cur.dispatched();
		break;

	default:
		InsetText::doDispatch(cur, cmd);
		break;
	}
}


bool InsetCollapsable::getStatus(LCursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action) {

	case LFUN_INSET_TOGGLE:
		if (cmd.argument == "open" || cmd.argument == "close" ||
		    cmd.argument == "toggle")
			flag.enabled(true);
		else
			flag.enabled(false);
		return true;

	default:
		return InsetText::getStatus(cur, cmd, flag);
	}
}


int InsetCollapsable::scroll(bool recursive) const
{
	int sx = UpdatableInset::scroll(false);

	if (recursive)
		sx += InsetText::scroll(false);

	return sx;
}


void InsetCollapsable::setLabel(string const & l)
{
	label = l;
}


void InsetCollapsable::setStatus(LCursor & cur, CollapseStatus status)
{
	status_ = status;
	setButtonLabel();
	if (status_ == Collapsed)
		cur.leaveInset(*this);
}


void InsetCollapsable::setLabelFont(LyXFont & font)
{
	labelfont_ = font;
}


void InsetCollapsable::scroll(BufferView & bv, double sx) const
{
	UpdatableInset::scroll(bv, sx);
}


void InsetCollapsable::scroll(BufferView & bv, int offset) const
{
	UpdatableInset::scroll(bv, offset);
}
