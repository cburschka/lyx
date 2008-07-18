/**
 * \file InsetCollapsable.cpp
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

#include "InsetCollapsable.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "debug.h"
#include "DispatchResult.h"
#include "FloatList.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "Color.h"
#include "Lexer.h"
#include "FuncRequest.h"
#include "MetricsInfo.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"


namespace lyx {

using graphics::PreviewLoader;

using std::endl;
using std::string;
using std::max;
using std::min;
using std::ostream;


InsetCollapsable::CollapseStatus InsetCollapsable::status() const
{
	return (autoOpen_ && status_ != Inlined) ? Open : status_;
}


InsetCollapsable::InsetCollapsable
		(BufferParams const & bp, CollapseStatus status)
	: InsetText(bp), label(from_ascii("Label")), status_(status),
	  openinlined_(false), autoOpen_(false), mouse_hover_(false)
{
	setAutoBreakRows(true);
	setDrawFrame(true);
	setFrameColor(Color::collapsableframe);
	setButtonLabel();
}


InsetCollapsable::InsetCollapsable(InsetCollapsable const & rhs)
	: InsetText(rhs),
		labelfont_(rhs.labelfont_),
		button_dim(rhs.button_dim),
		topx(rhs.topx),
		topbaseline(rhs.topbaseline),
		label(rhs.label),
		status_(rhs.status_),
		openinlined_(rhs.openinlined_),
		autoOpen_(rhs.autoOpen_),
		textdim_(rhs.textdim_),
		// the sole purpose of this copy constructor
		mouse_hover_(false)
{
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


void InsetCollapsable::read(Buffer const & buf, Lexer & lex)
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
	theFontMetrics(labelfont_).buttonText(
		label, dim.wid, dim.asc, dim.des);
	return dim;
}


bool InsetCollapsable::metrics(MetricsInfo & mi, Dimension & dim) const
{
	autoOpen_ = mi.base.bv->cursor().isInside(this);

	if (status() == Inlined) {
		InsetText::metrics(mi, dim);
	} else {
		dim = dimensionCollapsed();
		if (status() == Open) {
			InsetText::metrics(mi, textdim_);
			openinlined_ = !hasFixedWidth()
				&& (textdim_.wid + dim.wid) <= mi.base.textwidth;
			if (openinlined_) {
				// Correct for button width
				dim.wid += textdim_.wid;
				dim.des = max(dim.des - textdim_.asc + dim.asc, textdim_.des);
				dim.asc = textdim_.asc;
			} else {
				dim.des += textdim_.height();
				dim.wid = max(dim.wid, textdim_.wid);
				if (hasFixedWidth())
					dim.wid = max(dim.wid, mi.base.textwidth);
			}
		}
	}
	bool const changed = dim_ != dim;
	dim_ = dim;
	return changed;
}


bool InsetCollapsable::setMouseHover(bool mouse_hover)
{
	mouse_hover_ = mouse_hover;
	return true;
}


void InsetCollapsable::draw(PainterInfo & pi, int x, int y) const
{
	if (status() == Inlined) {
		InsetText::draw(pi, x, y);
	} else {
		Dimension dimc = dimensionCollapsed();
		int const top  = y - ascent();
		button_dim.x1 = x + 0;
		button_dim.x2 = x + dimc.width();
		button_dim.y1 = top;
		button_dim.y2 = top + dimc.height();

		pi.pain.buttonText(x, top + dimc.asc, label, labelfont_, mouse_hover_);

		if (status() == Open) {
			int textx, texty;
			if (openinlined_) {
				textx = x + dimc.width();
				texty = top + textdim_.asc;
			} else {
				textx = x;
				texty = top + dimc.height() + textdim_.asc;
			}
			InsetText::draw(pi, textx, texty);
		}
	}
	setPosCache(pi, x, y);
}


void InsetCollapsable::drawSelection(PainterInfo & pi, int x, int y) const
{
	if (status() == Open) {
		if (openinlined_)
			x += dimensionCollapsed().wid;
		else
			y += dimensionCollapsed().des + textdim_.asc;
	}
	if (status() != Collapsed)
		InsetText::drawSelection(pi, x, y);
}


void InsetCollapsable::cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x, int & y) const
{
	BOOST_ASSERT(status() != Collapsed);

	InsetText::cursorPos(bv, sl, boundary, x, y);

	if (status() == Open) {
		if (openinlined_)
			x += dimensionCollapsed().wid;
		else
			y += dimensionCollapsed().height() - ascent() + textdim_.asc;
	}
}


Inset::EDITABLE InsetCollapsable::editable() const
{
	return status() != Collapsed ? HIGHLY_EDITABLE : IS_EDITABLE;
}


bool InsetCollapsable::descendable() const
{
	return status() != Collapsed;
}


bool InsetCollapsable::hitButton(FuncRequest const & cmd) const
{
	return button_dim.contains(cmd.x, cmd.y);
}


docstring const InsetCollapsable::getNewLabel(docstring const & l) const
{
	docstring label;
	pos_type const max_length = 15;
	pos_type const p_siz = paragraphs().begin()->size();
	pos_type const n = min(max_length, p_siz);
	pos_type i = 0;
	pos_type j = 0;
	for (; i < n && j < p_siz; ++j) {
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


void InsetCollapsable::edit(Cursor & cur, bool left)
{
	//lyxerr << "InsetCollapsable: edit left/right" << endl;
	cur.push(*this);
	InsetText::edit(cur, left);
}


Inset * InsetCollapsable::editXY(Cursor & cur, int x, int y)
{
	//lyxerr << "InsetCollapsable: edit xy" << endl;
	if (status() == Collapsed || (button_dim.contains(x, y) && status() != Inlined))
		return this;
	cur.push(*this);
	return InsetText::editXY(cur, x, y);
}


void InsetCollapsable::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	//lyxerr << "InsetCollapsable::doDispatch (begin): cmd: " << cmd
	//	<< " cur: " << cur << " bvcur: " << cur.bv().cursor() << endl;

	switch (cmd.action) {
	case LFUN_MOUSE_PRESS:
		if (cmd.button() == mouse_button::button1 && hitButton(cmd) && status() != Inlined) {
			// reset selection if necessary (see bug 3060)
			if (cur.selection())
				cur.bv().cursor().clearSelection();
			else
				cur.noUpdate();
			cur.dispatched();
			break;
		}
		if (status() == Inlined)
			InsetText::doDispatch(cur, cmd);
		else if (status() == Open && !hitButton(cmd))
			InsetText::doDispatch(cur, cmd);
		else
			cur.undispatched();
		break;

	case LFUN_MOUSE_MOTION:
	case LFUN_MOUSE_DOUBLE:
	case LFUN_MOUSE_TRIPLE:
		if (status_ == Inlined)
			InsetText::doDispatch(cur, cmd);
		else if (status() && !hitButton(cmd))
			InsetText::doDispatch(cur, cmd);
		else
			cur.undispatched();
		break;

	case LFUN_MOUSE_RELEASE:
		if (cmd.button() == mouse_button::button3) {
			// Open the Inset configuration dialog
			showInsetDialog(&cur.bv());
			break;
		}

		if (status() == Inlined) {
			// The mouse click has to be within the inset!
			InsetText::doDispatch(cur, cmd);
			break;
		}

		if (cmd.button() == mouse_button::button1 && hitButton(cmd)) {
			// if we are selecting, we do not want to
			// toggle the inset.
			if (cur.selection())
				break;
			// Left button is clicked, the user asks to
			// toggle the inset visual state.
			cur.dispatched();
			cur.updateFlags(Update::Force | Update::FitCursor);
			if (status() == Collapsed) {
				setStatus(cur, Open);
				edit(cur, true);
			}
			else {
				setStatus(cur, Collapsed);
			}
			cur.bv().cursor() = cur;
			break;
		}

		if (cmd.button() != mouse_button::button1 && hitButton(cmd)) {
			// Nothing to do.
			cur.noUpdate();
			break;
		}

		// The mouse click is within the opened inset.
		if (status() == Open)
			InsetText::doDispatch(cur, cmd);
		break;

	case LFUN_INSET_TOGGLE:
		if (cmd.argument() == "open")
			setStatus(cur, Open);
		else if (cmd.argument() == "close")
			setStatus(cur, Collapsed);
		else if (cmd.argument() == "toggle" || cmd.argument().empty())
			if (isOpen()) {
				setStatus(cur, Collapsed);
				cur.forwardPosNoDescend();
			}
			else
				setStatus(cur, Open);
		else // if assign or anything else
			cur.undispatched();
		cur.dispatched();
		break;

	default:
		InsetText::doDispatch(cur, cmd);
		break;
	}
}


bool InsetCollapsable::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action) {

	case LFUN_INSET_TOGGLE:
		if (cmd.argument() == "open" || cmd.argument() == "close" ||
		    cmd.argument() == "toggle")
			flag.enabled(true);
		else
			flag.enabled(false);
		return true;

	default:
		return InsetText::getStatus(cur, cmd, flag);
	}
}


void InsetCollapsable::setLabel(docstring const & l)
{
	label = l;
}


void InsetCollapsable::setStatus(Cursor & cur, CollapseStatus status)
{
	status_ = status;
	setButtonLabel();
	if (status_ == Collapsed)
		cur.leaveInset(*this);
}


void InsetCollapsable::setLabelFont(Font & font)
{
	labelfont_ = font;
}

docstring InsetCollapsable::floatName(string const & type, BufferParams const & bp) const
{
	FloatList const & floats = bp.getTextClass().floats();
	FloatList::const_iterator it = floats[type];
	// FIXME UNICODE
	return (it == floats.end()) ? from_ascii(type) : bp.B_(it->second.name());
}


} // namespace lyx
