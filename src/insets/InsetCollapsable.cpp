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
	return autoOpen_ ? Open : status_;
}


InsetCollapsable::Geometry InsetCollapsable::geometry() const
{
	switch (decoration()) {
	case Classic:
		if (status_ == Open || autoOpen_) {
			if (openinlined_)
				return LeftButton;
			else
				return TopButton;
		} else
			return ButtonOnly;

	case Minimalistic:
		return NoButton;

	case Conglomerate:
		return status_ == Open ? SubLabel : Corners;
	}

	// dummy return value to shut down a warning,
	// this is dead code.
	return NoButton;
}


InsetCollapsable::InsetCollapsable
		(BufferParams const & bp, CollapseStatus status)
	: InsetText(bp), status_(status),
	  openinlined_(false), autoOpen_(false), mouse_hover_(false)
{
	setAutoBreakRows(true);
	setDrawFrame(true);
	setFrameColor(Color::collapsableframe);
	setButtonLabel();
}


InsetCollapsable::InsetCollapsable(InsetCollapsable const & rhs)
	: InsetText(rhs),
		button_dim(rhs.button_dim),
		topx(rhs.topx),
		topbaseline(rhs.topbaseline),
		layout_(rhs.layout_),
		status_(rhs.status_),
		openinlined_(rhs.openinlined_),
		autoOpen_(rhs.autoOpen_),
		textdim_(rhs.textdim_),
		// the sole purpose of this copy constructor
		mouse_hover_(false)
{
}


void  InsetCollapsable::setLayout(BufferParams const & bp)
{
	layout_ = getLayout(bp);
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

			if (tmp_token == "collapsed") {
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
	theFontMetrics(layout_.labelfont).buttonText(
		layout_.labelstring, dim.wid, dim.asc, dim.des);
	return dim;
}


bool InsetCollapsable::metrics(MetricsInfo & mi, Dimension & dim) const
{
	autoOpen_ = mi.base.bv->cursor().isInside(this);
	mi.base.textwidth -= (int) (1.5 * TEXT_TO_INSET_OFFSET);

	switch (decoration()) {
	case Minimalistic:
		InsetText::metrics(mi, dim);
		break;
	case Conglomerate:
		InsetText::metrics(mi, dim);
		if (status() == Open) {
			// consider width of the inset label
			Font font(layout_.labelfont);
			font.realize(Font(Font::ALL_SANE));
			font.decSize();
			font.decSize();
			int w = 0;
			int a = 0;
			int d = 0;
			docstring s = layout_.labelstring;
			theFontMetrics(font).rectText(s, w, a, d);
			dim.wid = max(dim.wid, w);
		}
		if (status() == Open)
			dim.des += ascent();
		else {
			dim.des -= 3;
			dim.asc -= 3;
		}
		break;
	case Classic:
		dim = dimensionCollapsed();
		if (geometry() == TopButton
		 || geometry() == LeftButton) {
			InsetText::metrics(mi, textdim_);
			// This expression should not contain mi.base.texwidth
			openinlined_ = !hasFixedWidth()
				&& textdim_.wid < 0.5 * mi.base.bv->workWidth();
			if (openinlined_) {
				// Correct for button width, and re-fit
				mi.base.textwidth -= dim.wid;
				InsetText::metrics(mi, textdim_);
				dim.wid += textdim_.wid;
				dim.des = max(dim.des - textdim_.asc + dim.asc, textdim_.des);
				dim.asc = textdim_.asc;
			} else {
				dim.des += textdim_.height() + TEXT_TO_BOTTOM_OFFSET;
				dim.wid = max(dim.wid, textdim_.wid);
				if (hasFixedWidth())
					dim.wid = max(dim.wid, mi.base.textwidth);
			}
		}
		break;
	}
	dim.asc += TEXT_TO_INSET_OFFSET;
	dim.des += TEXT_TO_INSET_OFFSET;
	dim.wid += (int) (1.5 * TEXT_TO_INSET_OFFSET);
	mi.base.textwidth += (int) (1.5 * TEXT_TO_INSET_OFFSET);
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
	const int xx = x + TEXT_TO_INSET_OFFSET;

	// Draw button first -- top, left or only
	Dimension dimc = dimensionCollapsed();
	int const top  = y - ascent() + TEXT_TO_INSET_OFFSET;
	if (decoration() == Classic) {
		button_dim.x1 = xx + 0;
		button_dim.x2 = xx + dimc.width();
		button_dim.y1 = top;
		button_dim.y2 = top + dimc.height();

		pi.pain.buttonText(xx, top + dimc.asc, layout_.labelstring, layout_.labelfont, mouse_hover_);
	}

	int textx, texty;
	switch (geometry()) {
	case LeftButton:
		textx = xx + dimc.width();
		texty = top + textdim_.asc;
		InsetText::draw(pi, textx, texty);
		break;
	case TopButton:
		textx = xx;
		texty = top + dimc.height() + textdim_.asc;
		InsetText::draw(pi, textx, texty);
		break;
	case ButtonOnly:
		break;
	case NoButton:
		textx = xx;
		texty = y + textdim_.asc;
		InsetText::draw(pi, textx, texty);
		break;
	case SubLabel:
	case Corners:
		textx = xx;
		texty = y + textdim_.asc;
		const_cast<InsetCollapsable *>(this)->setDrawFrame(false);
		InsetText::draw(pi, textx, texty);
		const_cast<InsetCollapsable *>(this)->setDrawFrame(true);

		int desc = InsetText::descent();
		if (status() == Open)
			desc -= ascent();
		else
			desc -= 3;

		pi.pain.line(x, y + desc - 4, x, y + desc, 
			layout_.labelfont.color());
		if (internalStatus() == Open)
			pi.pain.line(x, y + desc, 
				x + dim_.wid - 3, y + desc,
				layout_.labelfont.color());
		else {
			// Make status_ value visible:
			pi.pain.line(x, y + desc,
				x + 4, y + desc,
				layout_.labelfont.color());
			pi.pain.line(x + dim_.wid - 7, y + desc,
				x + dim_.wid -3, y + desc,
				layout_.labelfont.color());
		}
		pi.pain.line(x + dim_.wid - 3, y + desc, x + dim_.wid - 3, y + desc - 4,
			layout_.labelfont.color());

		// the label of the charstyle. Can be toggled.
		if (status() == Open) {
			Font font(layout_.labelfont);
			font.realize(Font(Font::ALL_SANE));
			font.decSize();
			font.decSize();
			int w = 0;
			int a = 0;
			int d = 0;
			// FIXME UNICODE
			docstring s = layout_.labelstring;
			theFontMetrics(font).rectText(s, w, a, d);
			pi.pain.rectText(x + (dim_.wid - w) / 2, y + desc + a,
				s, font, Color::none, Color::none);
		}

		// a visual cue when the cursor is inside the inset
		Cursor & cur = pi.base.bv->cursor();
		if (cur.isInside(this)) {
			y -= ascent();
			y += 3;
			pi.pain.line(x, y + 4, x, y, layout_.labelfont.color());
			pi.pain.line(x + 4, y, x, y, layout_.labelfont.color());
			pi.pain.line(x + dim_.wid - 3, y + 4, x + dim_.wid - 3, y,
				layout_.labelfont.color());
			pi.pain.line(x + dim_.wid - 7, y, x + dim_.wid - 3, y,
				layout_.labelfont.color());
		}
		break;
	}
	setPosCache(pi, x, y);
}


void InsetCollapsable::drawSelection(PainterInfo & pi, int x, int y) const
{
	x += TEXT_TO_INSET_OFFSET;
	switch (geometry()) {
	case LeftButton:
		x += dimensionCollapsed().wid;
		InsetText::drawSelection(pi, x, y);
		break;
	case TopButton:
		y += dimensionCollapsed().des + textdim_.asc;
		InsetText::drawSelection(pi, x, y);
		break;
	case ButtonOnly:
		break;
	case NoButton:
	case SubLabel:
	case Corners:
		InsetText::drawSelection(pi, x, y);
		break;
	}
}


void InsetCollapsable::cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x, int & y) const
{
	BOOST_ASSERT(geometry() != ButtonOnly);

	InsetText::cursorPos(bv, sl, boundary, x, y);

	switch (geometry()) {
	case LeftButton:
		x += dimensionCollapsed().wid;
		break;
	case TopButton:
		y += dimensionCollapsed().height() - ascent()
			+ TEXT_TO_INSET_OFFSET + textdim_.asc;
		break;
	case NoButton:
	case SubLabel:
	case Corners:
		// Do nothing
		break;
	case ButtonOnly:
		// Cannot get here
		break;
	}
	x += TEXT_TO_INSET_OFFSET;
}


Inset::EDITABLE InsetCollapsable::editable() const
{
	return geometry() != ButtonOnly? HIGHLY_EDITABLE : IS_EDITABLE;
}


bool InsetCollapsable::descendable() const
{
	return geometry() != ButtonOnly;
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
	if (geometry() == ButtonOnly
	 || (button_dim.contains(x, y) 
	  && decoration() != Minimalistic))
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
		if (cmd.button() == mouse_button::button1 
		 && hitButton(cmd) 
		 && decoration() != Minimalistic) {
			// reset selection if necessary (see bug 3060)
			if (cur.selection())
				cur.bv().cursor().clearSelection();
			else
				cur.noUpdate();
			cur.dispatched();
			break;
		}
		if (decoration() == Minimalistic)
			InsetText::doDispatch(cur, cmd);
		else if (geometry() != ButtonOnly 
		     && !hitButton(cmd))
			InsetText::doDispatch(cur, cmd);
		else
			cur.undispatched();
		break;

	case LFUN_MOUSE_MOTION:
	case LFUN_MOUSE_DOUBLE:
	case LFUN_MOUSE_TRIPLE:
		if (decoration() == Minimalistic)
			InsetText::doDispatch(cur, cmd);
		else if (geometry() != ButtonOnly
		     && !hitButton(cmd))
			InsetText::doDispatch(cur, cmd);
		else
			cur.undispatched();
		break;

	case LFUN_MOUSE_RELEASE:
		if (cmd.button() == mouse_button::button3) {
			if (decoration() == Conglomerate) {

				if (internalStatus() == Open)
					setStatus(cur, Collapsed);
				else
					setStatus(cur, Open);
				break;
			} else {
				// Open the Inset 
				// configuration dialog
				showInsetDialog(&cur.bv());
				break;
			}
		}

		if (decoration() == Minimalistic) {
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
			if (geometry() == ButtonOnly) {
				setStatus(cur, Open);
				edit(cur, true);
			}
			else {
				setStatus(cur, Collapsed);
			}
			cur.bv().cursor() = cur;
			break;
		}

		// The mouse click is within the opened inset.
		if (geometry() == TopButton
		 || geometry() == LeftButton)
			InsetText::doDispatch(cur, cmd);
		break;

	case LFUN_INSET_TOGGLE:
		if (cmd.argument() == "open")
			setStatus(cur, Open);
		else if (cmd.argument() == "close")
			setStatus(cur, Collapsed);
		else if (cmd.argument() == "toggle" || cmd.argument().empty())
			if (internalStatus() == Open) {
				setStatus(cur, Collapsed);
				if (geometry() == ButtonOnly)
					cur.top().forwardPos();
			} else
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
	layout_.labelstring = l;
}


void InsetCollapsable::setStatus(Cursor & cur, CollapseStatus status)
{
	status_ = status;
	setButtonLabel();
	if (status_ == Collapsed)
		cur.leaveInset(*this);
	// Because the collapse status is part of the inset and thus an
	// integral part of the Buffer contents a changed status must be
	// signaled to all views of current buffer.
	cur.bv().buffer()->changed();
}


void InsetCollapsable::setLabelFont(Font const & font)
{
	layout_.labelfont = font;
}

docstring InsetCollapsable::floatName(string const & type, BufferParams const & bp) const
{
	FloatList const & floats = bp.getTextClass().floats();
	FloatList::const_iterator it = floats[type];
	// FIXME UNICODE
	return (it == floats.end()) ? from_ascii(type) : bp.B_(it->second.name());
}



int InsetCollapsable::latex(Buffer const & buf, odocstream & os,
			  OutputParams const & runparams) const
{
	// This implements the standard way of handling the LaTeX output of
	// a collapsable inset, either a command or an environment. Standard 
	// collapsable insets should not redefine this, non-standard ones may
	// call this.
	if (!layout_.latexname.empty()) {
		if (layout_.latextype == "command") {
			// FIXME UNICODE
			os << '\\' << from_utf8(layout_.latexname);
			if (!layout_.latexparam.empty())
				os << from_utf8(layout_.latexparam);
			os << '{';
		} else if (layout_.latextype == "environment") {
			os << "%\n\\begin{" << from_utf8(layout_.latexname) << "}\n";
			if (!layout_.latexparam.empty())
				os << from_utf8(layout_.latexparam);
		}
	}
	int i = InsetText::latex(buf, os, runparams);
	if (!layout_.latexname.empty())
		if (layout_.latextype == "command") {
			os << "}";
		} else if (layout_.latextype == "environment") {
			os << "\n\\end{" << from_utf8(layout_.latexname) << "}\n";
			i += 4;
		}
	return i;
}


} // namespace lyx
