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
#include "BufferView.h"
#include "Cursor.h"
#include "Dimension.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "InsetLayout.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "OutputParams.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/RefChanger.h"

using namespace std;


namespace lyx {

InsetCollapsable::InsetCollapsable(Buffer * buf, InsetText::UsePlain ltype)
	: InsetText(buf, ltype), status_(Open)
{
	setDrawFrame(true);
	setFrameColor(Color_collapsableframe);
}


// The sole purpose of this copy constructor is to make sure
// that the view_ map is not copied and remains empty.
InsetCollapsable::InsetCollapsable(InsetCollapsable const & rhs)
	: InsetText(rhs),
	  status_(rhs.status_),
	  labelstring_(rhs.labelstring_)
{}


InsetCollapsable::~InsetCollapsable()
{
	map<BufferView const *, View>::iterator it = view_.begin();
	map<BufferView const *, View>::iterator end = view_.end();
	for (; it != end; ++it)
		if (it->second.mouse_hover_)
			it->first->clearLastInset(this);
}


InsetCollapsable::CollapseStatus InsetCollapsable::status(BufferView const & bv) const
{
	if (decoration() == InsetLayout::CONGLOMERATE)
		return status_;
	return view_[&bv].auto_open_ ? Open : status_;
}


InsetCollapsable::Geometry InsetCollapsable::geometry(BufferView const & bv) const
{
	switch (decoration()) {
	case InsetLayout::CLASSIC:
		if (status(bv) == Open)
			return view_[&bv].openinlined_ ? LeftButton : TopButton;
		return ButtonOnly;

	case InsetLayout::MINIMALISTIC:
		return status(bv) == Open ? NoButton : ButtonOnly ;

	case InsetLayout::CONGLOMERATE:
		return status(bv) == Open ? SubLabel : Corners ;

	case InsetLayout::DEFAULT:
		break; // this shouldn't happen
	}

	// dummy return value to shut down a warning,
	// this is dead code.
	return NoButton;
}


docstring InsetCollapsable::toolTip(BufferView const & bv, int x, int y) const
{
	Dimension const dim = dimensionCollapsed(bv);
	if (geometry(bv) == NoButton)
		return translateIfPossible(getLayout().labelstring());
	if (x > xo(bv) + dim.wid || y > yo(bv) + dim.des || isOpen(bv))
		return docstring();

	return toolTipText();
}


void InsetCollapsable::write(ostream & os) const
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
	text().write(os);
}


void InsetCollapsable::read(Lexer & lex)
{
	lex.setContext("InsetCollapsable::read");
	string tmp_token;
	status_ = Collapsed;
	lex >> "status" >> tmp_token;
	if (tmp_token == "open")
		status_ = Open;

	InsetText::read(lex);
	setButtonLabel();
}


Dimension InsetCollapsable::dimensionCollapsed(BufferView const & bv) const
{
	Dimension dim;
	FontInfo labelfont(getLabelfont());
	labelfont.realize(sane_font);
	theFontMetrics(labelfont).buttonText(
		buttonLabel(bv), dim.wid, dim.asc, dim.des);
	return dim;
}


void InsetCollapsable::metrics(MetricsInfo & mi, Dimension & dim) const
{
	view_[mi.base.bv].auto_open_ = mi.base.bv->cursor().isInside(this);

	FontInfo tmpfont = mi.base.font;
	mi.base.font = getFont();
	mi.base.font.realize(tmpfont);

	BufferView const & bv = *mi.base.bv;

	switch (geometry(bv)) {
	case NoButton:
		InsetText::metrics(mi, dim);
		break;
	case Corners:
		InsetText::metrics(mi, dim);
		dim.des -= 3;
		dim.asc -= 1;
		break;
	case SubLabel: {
		InsetText::metrics(mi, dim);
		// consider width of the inset label
		FontInfo font(getLabelfont());
		font.realize(sane_font);
		font.decSize();
		font.decSize();
		int w = 0;
		int a = 0;
		int d = 0;
		theFontMetrics(font).rectText(buttonLabel(bv), w, a, d);
		dim.des += a + d;
		break;
		}
	case TopButton:
	case LeftButton:
	case ButtonOnly:
		if (hasFixedWidth()){
			int const mindim = view_[&bv].button_dim_.x2 - view_[&bv].button_dim_.x1;
			if (mi.base.textwidth < mindim)
				mi.base.textwidth = mindim;
		}
		dim = dimensionCollapsed(bv);
		if (geometry(bv) == TopButton || geometry(bv) == LeftButton) {
			Dimension textdim;
			InsetText::metrics(mi, textdim);
			view_[&bv].openinlined_ = (textdim.wid + dim.wid) < mi.base.textwidth;
			if (view_[&bv].openinlined_) {
				// Correct for button width.
				dim.wid += textdim.wid;
				dim.des = max(dim.des - textdim.asc + dim.asc, textdim.des);
				dim.asc = textdim.asc;
			} else {
				dim.des += textdim.height() + TEXT_TO_INSET_OFFSET;
				dim.wid = max(dim.wid, textdim.wid);
			}
		}
		break;
	}

	mi.base.font = tmpfont;
}


bool InsetCollapsable::setMouseHover(BufferView const * bv, bool mouse_hover)
	const
{
	view_[bv].mouse_hover_ = mouse_hover;
	return true;
}


void InsetCollapsable::draw(PainterInfo & pi, int x, int y) const
{
	BufferView const & bv = *pi.base.bv;

	view_[&bv].auto_open_ = bv.cursor().isInside(this);

	Changer dummy = pi.base.font.change(getFont(), true);

	// Draw button first -- top, left or only
	Dimension dimc = dimensionCollapsed(bv);

	if (geometry(bv) == TopButton ||
	    geometry(bv) == LeftButton ||
	    geometry(bv) == ButtonOnly) {
		view_[&bv].button_dim_.x1 = x + 0;
		view_[&bv].button_dim_.x2 = x + dimc.width();
		view_[&bv].button_dim_.y1 = y - dimc.asc;
		view_[&bv].button_dim_.y2 = y + dimc.des;

		FontInfo labelfont = getLabelfont();
		labelfont.setColor(labelColor());
		pi.pain.buttonText(x, y, buttonLabel(bv), labelfont,
		                   view_[&bv].mouse_hover_);
		// Draw the change tracking cue on the label, unless RowPainter already
		// takes care of it.
		if (canPaintChange(bv))
			pi.change_.paintCue(pi, x, y, x + dimc.width(), labelfont);
	} else {
		view_[&bv].button_dim_.x1 = 0;
		view_[&bv].button_dim_.y1 = 0;
		view_[&bv].button_dim_.x2 = 0;
		view_[&bv].button_dim_.y2 = 0;
	}

	Dimension const textdim = InsetText::dimension(bv);
	int const baseline = y;
	int textx, texty;
	Geometry g = geometry(bv);
	switch (g) {
	case LeftButton:
	case TopButton: {
		if (g == LeftButton) {
			textx = x + dimc.width();
			texty = baseline;
		} else {
			textx = x;
			texty = baseline + dimc.des + textdim.asc;
		}
		// Do not draw the cue for INSERTED -- it is already in the button and
		// that's enough.
		Changer dummy = make_change(pi.change_, Change(),
		                            pi.change_.type == Change::INSERTED);
		InsetText::draw(pi, textx, texty);
		break;
	}
	case ButtonOnly:
		break;
	case NoButton:
		textx = x;
		texty = baseline;
		InsetText::draw(pi, textx, texty);
		break;
	case SubLabel:
	case Corners:
		textx = x;
		texty = baseline;
		{	// We will take care of the frame and the change tracking cue
			// ourselves, below.
			Changer dummy = make_change(pi.change_, Change());
			const_cast<InsetCollapsable *>(this)->setDrawFrame(false);
			InsetText::draw(pi, textx, texty);
			const_cast<InsetCollapsable *>(this)->setDrawFrame(true);
		}

		int desc = textdim.descent();
		if (g == Corners)
			desc -= 3;

		// Colour the frame according to the change type. (Like for tables.)
		Color colour = pi.change_.changed() ? pi.change_.color()
		                                    : Color_foreground;
		const int xx1 = x + TEXT_TO_INSET_OFFSET - 1;
		const int xx2 = x + textdim.wid - TEXT_TO_INSET_OFFSET + 1;
		pi.pain.line(xx1, y + desc - 4,
		             xx1, y + desc, colour);
		if (status_ == Open)
			pi.pain.line(xx1, y + desc,
			             xx2, y + desc, colour);
		else {
			// Make status_ value visible:
			pi.pain.line(xx1, y + desc,
			             xx1 + 4, y + desc, colour);
			pi.pain.line(xx2 - 4, y + desc,
			             xx2, y + desc, colour);
		}
		pi.pain.line(x + textdim.wid - 3, y + desc, x + textdim.wid - 3,
		             y + desc - 4, colour);

		// the label below the text. Can be toggled.
		if (g == SubLabel) {
			FontInfo font(getLabelfont());
			if (pi.change_.changed())
				font.setPaintColor(colour);
			font.realize(sane_font);
			font.decSize();
			font.decSize();
			int w = 0;
			int a = 0;
			int d = 0;
			theFontMetrics(font).rectText(buttonLabel(bv), w, a, d);
			int const ww = max(textdim.wid, w);
			pi.pain.rectText(x + (ww - w) / 2, y + desc + a,
				buttonLabel(bv), font, Color_none, Color_none);
		}

		int const y1 = y - textdim.asc + 3;
		// a visual cue when the cursor is inside the inset
		Cursor const & cur = bv.cursor();
		if (cur.isInside(this)) {
			pi.pain.line(xx1, y1 + 4, xx1, y1, colour);
			pi.pain.line(xx1 + 4, y1, xx1, y1, colour);
			pi.pain.line(xx2, y1 + 4, xx2, y1, colour);
			pi.pain.line(xx2 - 4, y1, xx2, y1, colour);
		}
		// Strike through the inset if deleted and not already handled by
		// RowPainter.
		if (pi.change_.deleted() && canPaintChange(bv))
			pi.change_.paintCue(pi, xx1, y1, xx2, y + desc);
		break;
	}
}


void InsetCollapsable::cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x, int & y) const
{
	if (geometry(bv) == ButtonOnly)
		status_ = Open;

	InsetText::cursorPos(bv, sl, boundary, x, y);
	Dimension const textdim = InsetText::dimension(bv);

	switch (geometry(bv)) {
	case LeftButton:
		x += dimensionCollapsed(bv).wid;
		break;
	case TopButton: {
		y += dimensionCollapsed(bv).des + textdim.asc;
		break;
	}
	case NoButton:
	case SubLabel:
	case Corners:
		// Do nothing
		break;
	case ButtonOnly:
		// Cannot get here
		break;
	}
}


bool InsetCollapsable::editable() const
{
	switch (decoration()) {
	case InsetLayout::CLASSIC:
	case InsetLayout::MINIMALISTIC:
		return status_ == Open;
	default:
		return true;
	}
}


bool InsetCollapsable::descendable(BufferView const & bv) const
{
	return geometry(bv) != ButtonOnly;
}


bool InsetCollapsable::clickable(BufferView const & bv, int x, int y) const
{
	return view_[&bv].button_dim_.contains(x, y);
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


void InsetCollapsable::edit(Cursor & cur, bool front, EntryDirection entry_from)
{
	//lyxerr << "InsetCollapsable: edit left/right" << endl;
	cur.push(*this);
	InsetText::edit(cur, front, entry_from);
}


Inset * InsetCollapsable::editXY(Cursor & cur, int x, int y)
{
	//lyxerr << "InsetCollapsable: edit xy" << endl;
	if (geometry(cur.bv()) == ButtonOnly
	    || (view_[&cur.bv()].button_dim_.contains(x, y)
	        && geometry(cur.bv()) != NoButton))
		return this;
	cur.push(*this);
	return InsetText::editXY(cur, x, y);
}


void InsetCollapsable::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	//lyxerr << "InsetCollapsable::doDispatch (begin): cmd: " << cmd
	//	<< " cur: " << cur << " bvcur: " << cur.bv().cursor() << endl;

	bool const hitButton = clickable(cur.bv(), cmd.x(), cmd.y());

	switch (cmd.action()) {
	case LFUN_MOUSE_PRESS:
		if (hitButton) {
			switch (cmd.button()) {
			case mouse_button::button1:
			case mouse_button::button3:
				// Pass the command to the enclosing InsetText,
				// so that the cursor gets set.
				cur.undispatched();
				break;
			case mouse_button::none:
			case mouse_button::button2:
			case mouse_button::button4:
			case mouse_button::button5:
				// Nothing to do.
				cur.noScreenUpdate();
				break;
			}
		} else if (geometry(cur.bv()) != ButtonOnly)
			InsetText::doDispatch(cur, cmd);
		else
			cur.undispatched();
		break;

	case LFUN_MOUSE_MOTION:
	case LFUN_MOUSE_DOUBLE:
	case LFUN_MOUSE_TRIPLE:
		if (hitButton)
			cur.noScreenUpdate();
		else if (geometry(cur.bv()) != ButtonOnly)
			InsetText::doDispatch(cur, cmd);
		else
			cur.undispatched();
		break;

	case LFUN_MOUSE_RELEASE:
		if (!hitButton) {
			// The mouse click has to be within the inset!
			if (geometry(cur.bv()) != ButtonOnly)
				InsetText::doDispatch(cur, cmd);
			else
				cur.undispatched();
			break;
		}
		if (cmd.button() != mouse_button::button1) {
			// Nothing to do.
			cur.noScreenUpdate();
			break;
		}
		// if we are selecting, we do not want to
		// toggle the inset.
		if (cur.selection())
			break;
		// Left button is clicked, the user asks to
		// toggle the inset visual state.
		cur.dispatched();
		cur.screenUpdateFlags(Update::Force | Update::FitCursor);
		if (geometry(cur.bv()) == ButtonOnly) {
			setStatus(cur, Open);
			edit(cur, true);
		}
		else
			setStatus(cur, Collapsed);
		cur.bv().cursor() = cur;
		break;

	case LFUN_INSET_TOGGLE:
		if (cmd.argument() == "open")
			setStatus(cur, Open);
		else if (cmd.argument() == "close")
			setStatus(cur, Collapsed);
		else if (cmd.argument() == "toggle" || cmd.argument().empty())
			if (status_ == Open)
				setStatus(cur, Collapsed);
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
	switch (cmd.action()) {
	case LFUN_INSET_TOGGLE:
		if (cmd.argument() == "open")
			flag.setEnabled(status_ != Open);
		else if (cmd.argument() == "close")
			flag.setEnabled(status_ == Open);
		else if (cmd.argument() == "toggle" || cmd.argument().empty()) {
			flag.setEnabled(true);
			flag.setOnOff(status_ == Open);
		} else
			flag.setEnabled(false);
		return true;

	default:
		return InsetText::getStatus(cur, cmd, flag);
	}
}


void InsetCollapsable::setLabel(docstring const & l)
{
	labelstring_ = l;
}


docstring const InsetCollapsable::buttonLabel(BufferView const & bv) const
{
	InsetLayout const & il = getLayout();
	docstring const label = labelstring_.empty() ?
		translateIfPossible(il.labelstring()) : labelstring_;
	if (!il.contentaslabel() || geometry(bv) != ButtonOnly)
		return label;
	return getNewLabel(label);
}


void InsetCollapsable::setStatus(Cursor & cur, CollapseStatus status)
{
	status_ = status;
	setButtonLabel();
	if (status_ == Collapsed)
		cur.leaveInset(*this);
}


InsetLayout::InsetDecoration InsetCollapsable::decoration() const
{
	InsetLayout::InsetDecoration const dec = getLayout().decoration();
	return dec == InsetLayout::DEFAULT ? InsetLayout::CLASSIC : dec;
}


string InsetCollapsable::contextMenu(BufferView const & bv, int x,
	int y) const
{
	string context_menu = contextMenuName();
	string const it_context_menu = InsetText::contextMenuName();
	if (decoration() == InsetLayout::CONGLOMERATE)
		return context_menu + ";" + it_context_menu;

	string const ic_context_menu = InsetCollapsable::contextMenuName();
	if (ic_context_menu != context_menu)
		context_menu += ";" + ic_context_menu;

	if (geometry(bv) == NoButton)
		return context_menu + ";" + it_context_menu;

	Dimension dim = dimensionCollapsed(bv);
	if (x < xo(bv) + dim.wid && y < yo(bv) + dim.des)
		return context_menu;

	return it_context_menu;
}


string InsetCollapsable::contextMenuName() const
{
	if (decoration() == InsetLayout::CONGLOMERATE)
		return "context-conglomerate";
	else
		return "context-collapsable";
}


bool InsetCollapsable::canPaintChange(BufferView const & bv) const
{
	// return false to let RowPainter draw the change tracking cue consistently
	// with the surrounding text, when the inset is inline: for buttons, for
	// non-allowMultiPar insets.
	switch (geometry(bv)) {
	case Corners:
	case SubLabel:
		return allowMultiPar();
	case ButtonOnly:
		return false;
	default:
		break;
	}
	return true;
}


} // namespace lyx
