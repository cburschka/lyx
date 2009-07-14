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
#include "Dimension.h"
#include "DispatchResult.h"
#include "FloatList.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "InsetLayout.h"
#include "InsetList.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "output_xhtml.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "sgml.h"
#include "TextClass.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"

using namespace std;


namespace lyx {

InsetCollapsable::CollapseStatus InsetCollapsable::status(BufferView const & bv) const
{
	if (decoration() == InsetLayout::CONGLOMERATE)
		return status_;
	return auto_open_[&bv] ? Open : status_;
}


InsetCollapsable::Geometry InsetCollapsable::geometry(BufferView const & bv) const
{
	switch (decoration()) {
	case InsetLayout::CLASSIC:
		if (status(bv) == Open)
			return openinlined_ ? LeftButton : TopButton;
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


InsetCollapsable::Geometry InsetCollapsable::geometry() const
{
	switch (decoration()) {
	case InsetLayout::CLASSIC:
		if (status_ == Open)
			return openinlined_ ? LeftButton : TopButton;
		return ButtonOnly;

	case InsetLayout::MINIMALISTIC:
		return status_ == Open ? NoButton : ButtonOnly ;

	case InsetLayout::CONGLOMERATE:
		return status_ == Open ? SubLabel : Corners ;

	case InsetLayout::DEFAULT:
		break; // this shouldn't happen
	}

	// dummy return value to shut down a warning,
	// this is dead code.
	return NoButton;
}


InsetCollapsable::InsetCollapsable(Buffer const & buf, InsetText::UsePlain ltype)
	: InsetText(buf, ltype), status_(Inset::Open),
	  openinlined_(false), mouse_hover_(false)
{
	setAutoBreakRows(true);
	setDrawFrame(true);
	setFrameColor(Color_collapsableframe);
}


InsetCollapsable::InsetCollapsable(InsetCollapsable const & rhs)
	: InsetText(rhs),
	  status_(rhs.status_),
	  labelstring_(rhs.labelstring_),
	  button_dim(rhs.button_dim),
	  openinlined_(rhs.openinlined_),
	  auto_open_(rhs.auto_open_),
	  // the sole purpose of this copy constructor
	  mouse_hover_(false)
{
}


docstring InsetCollapsable::toolTip(BufferView const & bv, int x, int y) const
{
	Dimension dim = dimensionCollapsed(bv);
	if (geometry(bv) == NoButton)
		return translateIfPossible(getLayout().labelstring());
	if (x > xo(bv) + dim.wid || y > yo(bv) + dim.des || isOpen(bv))
		return docstring();

	OutputParams rp(&buffer().params().encoding());
	odocstringstream ods;
	InsetText::plaintext(ods, rp);
	docstring const content_tip = ods.str();
	return support::wrapParas(content_tip, 4);
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
	text().write(buffer(), os);
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
	theFontMetrics(getLayout().labelfont()).buttonText(
		buttonLabel(bv), dim.wid, dim.asc, dim.des);
	return dim;
}


void InsetCollapsable::metrics(MetricsInfo & mi, Dimension & dim) const
{
	auto_open_[mi.base.bv] =  mi.base.bv->cursor().isInside(this);

	FontInfo tmpfont = mi.base.font;
	mi.base.font = getLayout().font();
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
		FontInfo font(getLayout().labelfont());
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
		dim = dimensionCollapsed(bv);
		if (geometry(bv) == TopButton 
			  || geometry(bv) == LeftButton) {
			Dimension textdim;
			InsetText::metrics(mi, textdim);
			openinlined_ = (textdim.wid + dim.wid) < mi.base.textwidth;
			if (openinlined_) {
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


bool InsetCollapsable::setMouseHover(bool mouse_hover)
{
	mouse_hover_ = mouse_hover;
	return true;
}


void InsetCollapsable::draw(PainterInfo & pi, int x, int y) const
{
	BufferView const & bv = *pi.base.bv;

	auto_open_[&bv] =  bv.cursor().isInside(this);

	FontInfo tmpfont = pi.base.font;
	pi.base.font = getLayout().font();
	pi.base.font.realize(tmpfont);

	// Draw button first -- top, left or only
	Dimension dimc = dimensionCollapsed(bv);

	if (geometry(*pi.base.bv) == TopButton ||
	    geometry(*pi.base.bv) == LeftButton ||
	    geometry(*pi.base.bv) == ButtonOnly) {
		button_dim.x1 = x + 0;
		button_dim.x2 = x + dimc.width();
		button_dim.y1 = y - dimc.asc;
		button_dim.y2 = y + dimc.des;

		FontInfo labelfont = getLayout().labelfont();
		labelfont.setColor(labelColor());
		pi.pain.buttonText(x, y, buttonLabel(bv), labelfont,
			mouse_hover_);
	} else {
		button_dim.x1 = 0;
		button_dim.y1 = 0;
		button_dim.x2 = 0;
		button_dim.y2 = 0;
	}

	Dimension const textdim = InsetText::dimension(bv);
	int const baseline = y;
	int textx, texty;
	switch (geometry(bv)) {
	case LeftButton:
		textx = x + dimc.width();
		texty = baseline;
		InsetText::draw(pi, textx, texty);
		break;
	case TopButton:
		textx = x;
		texty = baseline + dimc.des + textdim.asc;
		InsetText::draw(pi, textx, texty);
		break;
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
		const_cast<InsetCollapsable *>(this)->setDrawFrame(false);
		InsetText::draw(pi, textx, texty);
		const_cast<InsetCollapsable *>(this)->setDrawFrame(true);

		int desc = textdim.descent();
		if (geometry(bv) == Corners)
			desc -= 3;

		const int xx1 = x + TEXT_TO_INSET_OFFSET - 1;
		const int xx2 = x + textdim.wid - TEXT_TO_INSET_OFFSET + 1;
		pi.pain.line(xx1, y + desc - 4, 
			     xx1, y + desc, 
			labelColor());
		if (status_ == Open)
			pi.pain.line(xx1, y + desc, 
				xx2, y + desc,
				labelColor());
		else {
			// Make status_ value visible:
			pi.pain.line(xx1, y + desc,
				xx1 + 4, y + desc,
				labelColor());
			pi.pain.line(xx2 - 4, y + desc,
				xx2, y + desc,
				labelColor());
		}
		pi.pain.line(x + textdim.wid - 3, y + desc, x + textdim.wid - 3, 
			y + desc - 4, labelColor());

		// the label below the text. Can be toggled.
		if (geometry(bv) == SubLabel) {
			FontInfo font(getLayout().labelfont());
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
			desc += d;
		}

		// a visual cue when the cursor is inside the inset
		Cursor const & cur = bv.cursor();
		if (cur.isInside(this)) {
			y -= textdim.asc;
			y += 3;
			pi.pain.line(xx1, y + 4, xx1, y, labelColor());
			pi.pain.line(xx1 + 4, y, xx1, y, labelColor());
			pi.pain.line(xx2, y + 4, xx2, y,
				labelColor());
			pi.pain.line(xx2 - 4, y, xx2, y,
				labelColor());
		}
		break;
	}

	pi.base.font = tmpfont;
}


void InsetCollapsable::cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x, int & y) const
{
	if (geometry(bv) == ButtonOnly)
		status_ = Open;
	LASSERT(geometry(bv) != ButtonOnly, /**/);

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
	return geometry() != ButtonOnly;
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
	 || (button_dim.contains(x, y) 
	  && geometry(cur.bv()) != NoButton))
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
		if (hitButton(cmd)) {
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
				cur.noUpdate();
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
		if (hitButton(cmd)) 
			cur.noUpdate();
		else if (geometry(cur.bv()) != ButtonOnly)
			InsetText::doDispatch(cur, cmd);
		else
			cur.undispatched();
		break;

	case LFUN_MOUSE_RELEASE:
		if (!hitButton(cmd)) {
			// The mouse click has to be within the inset!
			if (geometry(cur.bv()) != ButtonOnly)
				InsetText::doDispatch(cur, cmd);
			else
				cur.undispatched();			
			break;
		}
		if (cmd.button() != mouse_button::button1) {
			// Nothing to do.
			cur.noUpdate();
			break;
		}
		// if we are selecting, we do not want to
		// toggle the inset.
		if (cur.selection())
			break;
		// Left button is clicked, the user asks to
		// toggle the inset visual state.
		cur.dispatched();
		cur.updateFlags(Update::Force | Update::FitCursor);
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
			if (status_ == Open) {
				setStatus(cur, Collapsed);
				if (geometry(cur.bv()) == ButtonOnly)
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


docstring const InsetCollapsable::buttonLabel(BufferView const &) const
{
	return labelstring_.empty() ? getLayout().labelstring() : labelstring_;
}


void InsetCollapsable::setStatus(Cursor & cur, CollapseStatus status)
{
	status_ = status;
	setButtonLabel();
	if (status_ == Collapsed) {
		cur.leaveInset(*this);
		mouse_hover_ = false;
	}
}


docstring InsetCollapsable::floatName(string const & type) const
{
	BufferParams const & bp = buffer().params();
	FloatList const & floats = bp.documentClass().floats();
	FloatList::const_iterator it = floats[type];
	// FIXME UNICODE
	return (it == floats.end()) ? from_ascii(type) : bp.B_(it->second.name());
}


InsetLayout::InsetDecoration InsetCollapsable::decoration() const
{
	InsetLayout::InsetDecoration const dec = getLayout().decoration();
	return dec == InsetLayout::DEFAULT ? InsetLayout::CLASSIC : dec;
}


int InsetCollapsable::latex(odocstream & os,
			  OutputParams const & runparams) const
{
	// This implements the standard way of handling the LaTeX output of
	// a collapsable inset, either a command or an environment. Standard 
	// collapsable insets should not redefine this, non-standard ones may
	// call this.
	InsetLayout const & il = getLayout();
	if (!il.latexname().empty()) {
		if (il.latextype() == InsetLayout::COMMAND) {
			// FIXME UNICODE
			if (runparams.moving_arg)
				os << "\\protect";
			os << '\\' << from_utf8(il.latexname());
			if (!il.latexparam().empty())
				os << from_utf8(il.latexparam());
			os << '{';
		} else if (il.latextype() == InsetLayout::ENVIRONMENT) {
			os << "%\n\\begin{" << from_utf8(il.latexname()) << "}\n";
			if (!il.latexparam().empty())
				os << from_utf8(il.latexparam());
		}
	}
	OutputParams rp = runparams;
	if (il.isPassThru())
		rp.verbatim = true;
	if (il.isNeedProtect())
		rp.moving_arg = true;
	int i = InsetText::latex(os, rp);
	if (!il.latexname().empty()) {
		if (il.latextype() == InsetLayout::COMMAND) {
			os << "}";
		} else if (il.latextype() == InsetLayout::ENVIRONMENT) {
			os << "\n\\end{" << from_utf8(il.latexname()) << "}\n";
			i += 4;
		}
	}
	return i;
}


// FIXME It seems as if it ought to be possible to do this more simply,
// maybe by calling InsetText::docbook() in the middle there.
int InsetCollapsable::docbook(odocstream & os, OutputParams const & runparams) const
{
	ParagraphList::const_iterator const beg = paragraphs().begin();
	ParagraphList::const_iterator par = paragraphs().begin();
	ParagraphList::const_iterator const end = paragraphs().end();

	if (!undefined())
		sgml::openTag(os, getLayout().latexname(),
			      par->getID(buffer(), runparams) + getLayout().latexparam());

	for (; par != end; ++par) {
		par->simpleDocBookOnePar(buffer(), os, runparams,
					 outerFont(distance(beg, par),
						   paragraphs()));
	}

	if (!undefined())
		sgml::closeTag(os, getLayout().latexname());

	return 0;
}


docstring InsetCollapsable::xhtml(odocstream & os, OutputParams const & runparams) const
{
	InsetLayout const & il = getLayout();
	if (undefined())
		return InsetText::xhtml(os, runparams);

	bool const opened = html::openTag(os, il.htmltag(), il.htmlattr());
	if (!il.counter().empty()) {
		BufferParams const & bp = buffer().masterBuffer()->params();
		Counters & cntrs = bp.documentClass().counters();
		cntrs.step(il.counter());
		// FIXME: translate to paragraph language
		if (!il.htmllabel().empty())
			os << cntrs.counterLabel(from_utf8(il.htmllabel()), bp.language->code());
	}
	bool innertag_opened = false;
	if (!il.htmlinnertag().empty())
		innertag_opened = html::openTag(os, il.htmlinnertag(), il.htmlinnerattr());
	docstring deferred = InsetText::xhtml(os, runparams);
	if (innertag_opened)
		html::closeTag(os, il.htmlinnertag());
	if (opened)
		html::closeTag(os, il.htmltag());
	return deferred;
}


void InsetCollapsable::validate(LaTeXFeatures & features) const
{
	features.useInsetLayout(getLayout());
	InsetText::validate(features);
}


bool InsetCollapsable::undefined() const
{
	docstring const & n = getLayout().name();
	return n.empty() || n == DocumentClass::plainInsetLayout().name();
}


docstring InsetCollapsable::contextMenu(BufferView const & bv, int x,
	int y) const
{
	if (decoration() == InsetLayout::CONGLOMERATE)
		return from_ascii("context-conglomerate");

	if (geometry(bv) == NoButton)
		return from_ascii("context-collapsable");

	Dimension dim = dimensionCollapsed(bv);
	if (x < xo(bv) + dim.wid && y < yo(bv) + dim.des)
		return from_ascii("context-collapsable");

	return InsetText::contextMenu(bv, x, y);
}

void InsetCollapsable::tocString(odocstream & os) const
{
	if (!getLayout().isInToc())
		return;
	os << text().asString(0, 1, AS_STR_LABEL | AS_STR_INSETS);
}


} // namespace lyx
