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
#include "FuncStatus.h"
#include "InsetLayout.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "FuncRequest.h"
#include "MetricsInfo.h"
#include "ParagraphParameters.h"
#include "TextClass.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"

using namespace std;

namespace lyx {


InsetCollapsable::CollapseStatus InsetCollapsable::status() const
{
	if (decoration() == InsetLayout::Conglomerate)
		return status_;
	return autoOpen_ ? Open : status_;
}


InsetCollapsable::Geometry InsetCollapsable::geometry() const
{
	switch (decoration()) {
	case InsetLayout::Classic:
		if (status() == Open)
			return openinlined_ ? LeftButton : TopButton;
		return ButtonOnly;

	case InsetLayout::Minimalistic:
		return status() == Open ? NoButton : ButtonOnly ;

	case InsetLayout::Conglomerate:
		return status() == Open ? SubLabel : Corners ;

	case InsetLayout::Default:
		break; // this shouldn't happen
	}

	// dummy return value to shut down a warning,
	// this is dead code.
	return NoButton;
}


InsetCollapsable::InsetCollapsable(Buffer const & buf,
		CollapseStatus status, DocumentClass * dc)
	: InsetText(buf), status_(status),
	  openinlined_(false), autoOpen_(false), mouse_hover_(false)
{
	setLayout(dc);
	setAutoBreakRows(true);
	setDrawFrame(true);
	setFrameColor(Color_collapsableframe);
	paragraphs().back().setLayout(buf.params().documentClass().emptyLayout());
}


InsetCollapsable::InsetCollapsable(InsetCollapsable const & rhs)
	: InsetText(rhs),
		layout_(rhs.layout_),
		labelstring_(rhs.labelstring_),
		button_dim(rhs.button_dim),
		status_(rhs.status_),
		openinlined_(rhs.openinlined_),
		autoOpen_(rhs.autoOpen_),
		// the sole purpose of this copy constructor
		mouse_hover_(false)
{
}


docstring InsetCollapsable::toolTip(BufferView const & bv, int x, int y) const
{
	Dimension dim = dimensionCollapsed();
	if (geometry() == NoButton)
		return layout_->labelstring();
	if (x > xo(bv) + dim.wid || y > yo(bv) + dim.des)
		return docstring();

	switch (status_) {
	case Open:
		return _("Left-click to collapse the inset");
	case Collapsed:
		return _("Left-click to open the inset");
	}
	return docstring();
}


void InsetCollapsable::setLayout(BufferParams const & bp)
{
	setLayout(bp.documentClassPtr());
}


void InsetCollapsable::setLayout(DocumentClass const * const dc)
{
	if (dc) {
		layout_ = &(dc->insetLayout(name()));
		labelstring_ = layout_->labelstring();
	} else {
		layout_ = &DocumentClass::emptyInsetLayout();
		labelstring_ = _("UNDEFINED");
	}

	setButtonLabel();
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
	text_.write(buffer(), os);
}


void InsetCollapsable::read(Lexer & lex)
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
			LYXERR0("InsetCollapsable::read: Missing 'status'-tag!");
			// take countermeasures
			lex.pushToken(token);
		}
	}
	// this must be set before we enter InsetText::read()
	setLayout(buffer().params());

	InsetText::read(lex);

	if (!token_found)
		status_ = isOpen() ? Open : Collapsed;

	// Force default font, if so requested
	// This avoids paragraphs in buffer language that would have a
	// foreign language after a document language change, and it ensures
	// that all new text in ERT and similar gets the "latex" language,
	// since new text inherits the language from the last position of the
	// existing text.  As a side effect this makes us also robust against
	// bugs in LyX that might lead to font changes in ERT in .lyx files.
	resetParagraphsFont();
}


Dimension InsetCollapsable::dimensionCollapsed() const
{
	BOOST_ASSERT(layout_);
	Dimension dim;
	theFontMetrics(layout_->labelfont()).buttonText(
		labelstring_, dim.wid, dim.asc, dim.des);
	return dim;
}


void InsetCollapsable::metrics(MetricsInfo & mi, Dimension & dim) const
{
	BOOST_ASSERT(layout_);

	autoOpen_ = mi.base.bv->cursor().isInside(this);

	FontInfo tmpfont = mi.base.font;
	mi.base.font = layout_->font();
	mi.base.font.realize(tmpfont);

	switch (geometry()) {
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
		FontInfo font(layout_->labelfont());
		font.realize(sane_font);
		font.decSize();
		font.decSize();
		int w = 0;
		int a = 0;
		int d = 0;
		theFontMetrics(font).rectText(labelstring_, w, a, d);
		dim.des += a + d;
		break;
		}
	case TopButton:
	case LeftButton:
	case ButtonOnly:
		dim = dimensionCollapsed();
		if (geometry() == TopButton || geometry() == LeftButton) {
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
	BOOST_ASSERT(layout_);

	autoOpen_ = pi.base.bv->cursor().isInside(this);
	ColorCode const old_color = pi.background_color;
	pi.background_color = backgroundColor();

	FontInfo tmpfont = pi.base.font;
	pi.base.font = layout_->font();
	pi.base.font.realize(tmpfont);

	// Draw button first -- top, left or only
	Dimension dimc = dimensionCollapsed();

	if (geometry() == TopButton ||
	    geometry() == LeftButton ||
	    geometry() == ButtonOnly) {
		button_dim.x1 = x + 0;
		button_dim.x2 = x + dimc.width();
		button_dim.y1 = y - dimc.asc;
		button_dim.y2 = y + dimc.des;

		pi.pain.buttonText(x, y, labelstring_, layout_->labelfont(),
			mouse_hover_);
	} else {
		button_dim.x1 = 0;
		button_dim.y1 = 0;
		button_dim.x2 = 0;
		button_dim.y2 = 0;
	}

	Dimension const textdim = InsetText::dimension(*pi.base.bv);
	int const baseline = y;
	int textx, texty;
	switch (geometry()) {
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
		if (geometry() == Corners)
			desc -= 3;

		const int xx1 = x + TEXT_TO_INSET_OFFSET - 1;
		const int xx2 = x + textdim.wid - TEXT_TO_INSET_OFFSET + 1;
		pi.pain.line(xx1, y + desc - 4, 
			     xx1, y + desc, 
			layout_->labelfont().color());
		if (status_ == Open)
			pi.pain.line(xx1, y + desc, 
				xx2, y + desc,
				layout_->labelfont().color());
		else {
			// Make status_ value visible:
			pi.pain.line(xx1, y + desc,
				xx1 + 4, y + desc,
				layout_->labelfont().color());
			pi.pain.line(xx2 - 4, y + desc,
				xx2, y + desc,
				layout_->labelfont().color());
		}
		pi.pain.line(x + textdim.wid - 3, y + desc, x + textdim.wid - 3, 
			y + desc - 4, layout_->labelfont().color());

		// the label below the text. Can be toggled.
		if (geometry() == SubLabel) {
			FontInfo font(layout_->labelfont());
			font.realize(sane_font);
			font.decSize();
			font.decSize();
			int w = 0;
			int a = 0;
			int d = 0;
			theFontMetrics(font).rectText(labelstring_, w, a, d);
			int const ww = max(textdim.wid, w);
			pi.pain.rectText(x + (ww - w) / 2, y + desc + a,
				labelstring_, font, Color_none, Color_none);
			desc += d;
		}

		// a visual cue when the cursor is inside the inset
		Cursor & cur = pi.base.bv->cursor();
		if (cur.isInside(this)) {
			y -= textdim.asc;
			y += 3;
			pi.pain.line(xx1, y + 4, xx1, y, layout_->labelfont().color());
			pi.pain.line(xx1 + 4, y, xx1, y, layout_->labelfont().color());
			pi.pain.line(xx2, y + 4, xx2, y,
				layout_->labelfont().color());
			pi.pain.line(xx2 - 4, y, xx2, y,
				layout_->labelfont().color());
		}
		break;
	}
	pi.background_color = old_color;

	pi.base.font = tmpfont;
}


void InsetCollapsable::cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x, int & y) const
{
	if (geometry() == ButtonOnly)
		status_ = Open;
	BOOST_ASSERT(geometry() != ButtonOnly);

	InsetText::cursorPos(bv, sl, boundary, x, y);
	Dimension const textdim = InsetText::dimension(bv);

	switch (geometry()) {
	case LeftButton:
		x += dimensionCollapsed().wid;
		break;
	case TopButton: {
		y += dimensionCollapsed().des + textdim.asc;
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


void InsetCollapsable::edit(
		Cursor & cur, bool front, EntryDirection entry_from)
{
	//lyxerr << "InsetCollapsable: edit left/right" << endl;
	cur.push(*this);
	InsetText::edit(cur, front, entry_from);
}


Inset * InsetCollapsable::editXY(Cursor & cur, int x, int y)
{
	//lyxerr << "InsetCollapsable: edit xy" << endl;
	if (geometry() == ButtonOnly
	 || (button_dim.contains(x, y) 
	  && geometry() != NoButton))
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
		if (hitButton(cmd) && geometry() != NoButton) {
			switch (cmd.button()) {
			case mouse_button::button1:
				// reset selection if necessary (see bug 3060)
				if (cur.selection())
					cur.bv().cursor().clearSelection();
				else
					cur.noUpdate();
				cur.dispatched();
				return;
			case mouse_button::button2:
			case mouse_button::button3:
				// Nothing to do.
				cur.undispatched();
				return;
			}
		}
		if (geometry() == NoButton 
			|| (geometry() != ButtonOnly && !hitButton(cmd)))
			InsetText::doDispatch(cur, cmd);
		else
			cur.undispatched();
		break;

	case LFUN_MOUSE_MOTION:
	case LFUN_MOUSE_DOUBLE:
	case LFUN_MOUSE_TRIPLE:
		if (geometry() == NoButton)
			InsetText::doDispatch(cur, cmd);
		else if (geometry() != ButtonOnly
		     && !hitButton(cmd))
			InsetText::doDispatch(cur, cmd);
		else
			cur.undispatched();
		break;

	case LFUN_MOUSE_RELEASE:
		if (geometry() == NoButton) {
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
			if (status_ == Open) {
				setStatus(cur, Collapsed);
				if (geometry() == ButtonOnly)
					cur.top().forwardPos();
			} else
				setStatus(cur, Open);
		else // if assign or anything else
			cur.undispatched();
		cur.dispatched();
		break;

	case LFUN_PASTE:
	case LFUN_CLIPBOARD_PASTE:
	case LFUN_PRIMARY_SELECTION_PASTE: {
		InsetText::doDispatch(cur, cmd);
		// Since we can only store plain text, we must reset all
		// attributes.
		// FIXME: Change only the pasted paragraphs

		resetParagraphsFont();
		break;
	}

	default:
		if (layout_ && layout_->isForceLtr()) {
			// Force any new text to latex_language
			// FIXME: This should only be necessary in constructor, but
			// new paragraphs that are created by pressing enter at the
			// start of an existing paragraph get the buffer language
			// and not latex_language, so we take this brute force
			// approach.
			cur.current_font.setLanguage(latex_language);
			cur.real_current_font.setLanguage(latex_language);
		}
		InsetText::doDispatch(cur, cmd);
		break;
	}
}


bool InsetCollapsable::allowMultiPar() const
{
	return layout_->isMultiPar();
}


void InsetCollapsable::resetParagraphsFont()
{
	Font font;
	font.fontInfo() = inherit_font;
	if (layout_->isForceLtr())
		font.setLanguage(latex_language);
	if (layout_->isPassThru()) {
		ParagraphList::iterator par = paragraphs().begin();
		ParagraphList::iterator const end = paragraphs().end();
		while (par != end) {
			par->resetFonts(font);
			par->params().clear();
			++par;
		}
	}
}


bool InsetCollapsable::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action) {
	// suppress these
	case LFUN_ACCENT_ACUTE:
	case LFUN_ACCENT_BREVE:
	case LFUN_ACCENT_CARON:
	case LFUN_ACCENT_CEDILLA:
	case LFUN_ACCENT_CIRCLE:
	case LFUN_ACCENT_CIRCUMFLEX:
	case LFUN_ACCENT_DOT:
	case LFUN_ACCENT_GRAVE:
	case LFUN_ACCENT_HUNGARIAN_UMLAUT:
	case LFUN_ACCENT_MACRON:
	case LFUN_ACCENT_OGONEK:
	case LFUN_ACCENT_SPECIAL_CARON:
	case LFUN_ACCENT_TIE:
	case LFUN_ACCENT_TILDE:
	case LFUN_ACCENT_UMLAUT:
	case LFUN_ACCENT_UNDERBAR:
	case LFUN_ACCENT_UNDERDOT:
	case LFUN_APPENDIX:
	case LFUN_BIBITEM_INSERT:
	case LFUN_BOX_INSERT:
	case LFUN_BRANCH_INSERT:
	case LFUN_NEW_LINE:
	case LFUN_CAPTION_INSERT:
	case LFUN_DEPTH_DECREMENT:
	case LFUN_DEPTH_INCREMENT:
	case LFUN_ENVIRONMENT_INSERT:
	case LFUN_ERT_INSERT:
	case LFUN_FILE_INSERT:
	case LFUN_FLEX_INSERT:
	case LFUN_FLOAT_INSERT:
	case LFUN_FLOAT_LIST:
	case LFUN_FLOAT_WIDE_INSERT:
	case LFUN_FONT_BOLD:
	case LFUN_FONT_TYPEWRITER:
	case LFUN_FONT_DEFAULT:
	case LFUN_FONT_EMPH:
	case LFUN_FONT_FREE_APPLY:
	case LFUN_FONT_FREE_UPDATE:
	case LFUN_FONT_NOUN:
	case LFUN_FONT_ROMAN:
	case LFUN_FONT_SANS:
	case LFUN_FONT_FRAK:
	case LFUN_FONT_ITAL:
	case LFUN_FONT_SIZE:
	case LFUN_FONT_STATE:
	case LFUN_FONT_UNDERLINE:
	case LFUN_FOOTNOTE_INSERT:
	case LFUN_HYPERLINK_INSERT:
	case LFUN_INDEX_INSERT:
	case LFUN_INDEX_PRINT:
	case LFUN_INSET_INSERT:
	case LFUN_LABEL_GOTO:
	case LFUN_LABEL_INSERT:
	case LFUN_LINE_INSERT:
	case LFUN_NEWPAGE_INSERT:
	case LFUN_LAYOUT:
	case LFUN_LAYOUT_PARAGRAPH:
	case LFUN_LAYOUT_TABULAR:
	case LFUN_MARGINALNOTE_INSERT:
	case LFUN_MATH_DISPLAY:
	case LFUN_MATH_INSERT:
	case LFUN_MATH_MATRIX:
	case LFUN_MATH_MODE:
	case LFUN_MENU_OPEN:
	case LFUN_NOACTION:
	case LFUN_NOMENCL_INSERT:
	case LFUN_NOMENCL_PRINT:
	case LFUN_NOTE_INSERT:
	case LFUN_NOTE_NEXT:
	case LFUN_OPTIONAL_INSERT:
	case LFUN_PARAGRAPH_PARAMS:
	case LFUN_PARAGRAPH_PARAMS_APPLY:
	case LFUN_PARAGRAPH_SPACING:
	case LFUN_PARAGRAPH_UPDATE:
	case LFUN_REFERENCE_NEXT:
	case LFUN_SERVER_GOTO_FILE_ROW:
	case LFUN_SERVER_NOTIFY:
	case LFUN_SERVER_SET_XY:
	case LFUN_SPACE_INSERT:
	case LFUN_SPECIALCHAR_INSERT:
	case LFUN_TABULAR_INSERT:
	case LFUN_TOC_INSERT:
	case LFUN_WRAP_INSERT:
		if (layout_->isPassThru()) {
			flag.enabled(false);
			return true;
		} else
			return InsetText::getStatus(cur, cmd, flag);

	case LFUN_INSET_TOGGLE:
		if ((cmd.argument() == "open" && status_ != Open)
		      || (cmd.argument() == "close" && status_ == Open)
		      || cmd.argument() == "toggle" || cmd.argument().empty())
				flag.enabled(true);
		else
			flag.enabled(false);
		return true;

	case LFUN_LANGUAGE:
		flag.enabled(!layout_->isForceLtr());
		return InsetText::getStatus(cur, cmd, flag);

	case LFUN_BREAK_PARAGRAPH:
	case LFUN_BREAK_PARAGRAPH_SKIP:
		flag.enabled(layout_->isMultiPar());
		return true;

	default:
		return InsetText::getStatus(cur, cmd, flag);
	}
}


void InsetCollapsable::setLabel(docstring const & l)
{
	labelstring_ = l;
}


void InsetCollapsable::setStatus(Cursor & cur, CollapseStatus status)
{
	status_ = status;
	setButtonLabel();
	if (status_ == Collapsed)
		cur.leaveInset(*this);
}


docstring InsetCollapsable::floatName(
		string const & type, BufferParams const & bp) const
{
	FloatList const & floats = bp.documentClass().floats();
	FloatList::const_iterator it = floats[type];
	// FIXME UNICODE
	return (it == floats.end()) ? from_ascii(type) : bp.B_(it->second.name());
}


InsetLayout::InsetDecoration InsetCollapsable::decoration() const
{
	if (!layout_)
		return InsetLayout::Classic;
	InsetLayout::InsetDecoration const dec = layout_->decoration();
	switch (dec) {
	case InsetLayout::Classic:
	case InsetLayout::Minimalistic:
	case InsetLayout::Conglomerate:
		return dec;
	case InsetLayout::Default:
		break;
	}
	if (lyxCode() == FLEX_CODE)
		return InsetLayout::Conglomerate;
	return InsetLayout::Classic;
}


int InsetCollapsable::latex(odocstream & os,
			  OutputParams const & runparams) const
{
	// FIXME: What should we do layout_ is 0?
	// 1) assert
	// 2) through an error
	if (!layout_)
		return 0;

	// This implements the standard way of handling the LaTeX output of
	// a collapsable inset, either a command or an environment. Standard 
	// collapsable insets should not redefine this, non-standard ones may
	// call this.
	if (!layout_->latexname().empty()) {
		if (layout_->latextype() == "command") {
			// FIXME UNICODE
			if (runparams.moving_arg)
				os << "\\protect";
			os << '\\' << from_utf8(layout_->latexname());
			if (!layout_->latexparam().empty())
				os << from_utf8(layout_->latexparam());
			os << '{';
		} else if (layout_->latextype() == "environment") {
			os << "%\n\\begin{" << from_utf8(layout_->latexname()) << "}\n";
			if (!layout_->latexparam().empty())
				os << from_utf8(layout_->latexparam());
		}
	}
	OutputParams rp = runparams;
	if (layout_->isPassThru())
		rp.verbatim = true;
	if (layout_->isNeedProtect())
		rp.moving_arg = true;
	int i = InsetText::latex(os, rp);
	if (!layout_->latexname().empty()) {
		if (layout_->latextype() == "command") {
			os << "}";
		} else if (layout_->latextype() == "environment") {
			os << "\n\\end{" << from_utf8(layout_->latexname()) << "}\n";
			i += 4;
		}
	}
	return i;
}


void InsetCollapsable::validate(LaTeXFeatures & features) const
{
	if (!layout_)
		return;

	// Force inclusion of preamble snippet in layout file
	features.require(to_utf8(layout_->name()));
	InsetText::validate(features);
}


bool InsetCollapsable::undefined() const
{
	docstring const & n = getLayout().name();
	return n.empty() || n == DocumentClass::emptyInsetLayout().name();
}


docstring InsetCollapsable::contextMenu(BufferView const & bv, int x,
	int y) const
{
	if (decoration() == InsetLayout::Conglomerate)
		return from_ascii("context-conglomerate");

	if (geometry() == NoButton)
		return from_ascii("context-collapsable");

	Dimension dim = dimensionCollapsed();
	if (x < xo(bv) + dim.wid && y < yo(bv) + dim.des)
		return from_ascii("context-collapsable");

	return InsetText::contextMenu(bv, x, y);
}

} // namespace lyx
