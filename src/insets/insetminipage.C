/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *          Copyright 1998 The LyX Team.
 *
 *======================================================*/

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetminipage.h"
#include "gettext.h"
#include "lyxfont.h"
#include "BufferView.h"
#include "LyXView.h"
#include "frontends/Dialogs.h"
#include "lyxtext.h"
#include "insets/insettext.h"
#include "support/LOstream.h"
#include "support/lstrings.h"
#include "debug.h"
#include "gettext.h"

using std::ostream;
using std::endl;


// Some information about Minipages in LaTeX:
// A minipage is a complete miniversion of a page and can contain
// its own footnotes, paragraphs, and array, tabular, and multicols
// environments. However it cannot contain floats or \marginpar's,
// but it can appear inside floats.
//
// The minipage environment is defined like this:
//
// \begin{minipage}[pos][height][inner-pos]{width} <text> \end{minipage}
//
// Where:
//     pos [opt] = is the vertical placement of the box with respect
//                 to the text baseline, [c], [t] and [b].
//     height [opt] = the height of the box
//     inner-pos [opt] = the position of the text within the box.
//                 It can be t, c, b or s, if unspecified the value
//                 of pos is used.
//     width = the width of the box
//
// In LyX we should try to support all these parameters, settable in a
// pop-up dialog.
// In this pop-up diallog it should also be possible to set all margin
// values that is usable in the minipage.
// With regard to different formats (like DocBook) I guess a minipage
// can be used there also. Perhaps not in the latex way, but we do not
// have to output "" for minipages.
// (Lgb)

InsetMinipage::InsetMinipage()
	: InsetCollapsable(), pos_(center),
	  inner_pos_(inner_center)
{
	setLabel(_("minipage"));
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.decSize();
	font.setColor(LColor::collapsable);
	setLabelFont(font);
	setAutoCollapse(false);
	setInsetName("Minipage");
	width_ = "100%"; // set default to 100% of column_width
}


InsetMinipage::~InsetMinipage()
{
	hideDialog();
}


void InsetMinipage::write(Buffer const * buf, ostream & os) const 
{
	os << getInsetName() << "\n"
	   << "position " << pos_ << "\n"
	   << "inner_position " << inner_pos_ << "\n"
	   << "height \"" << height_ << "\"\n"
	   << "width \"" << width_ << "\"\n";
	InsetCollapsable::write(buf, os);
}


void InsetMinipage::read(Buffer const * buf, LyXLex & lex)
{
	string token;

	if (lex.IsOK()) {
		lex.next();
		token = lex.GetString();
		if (token == "position") {
			lex.next();
			pos_ = static_cast<Position>(lex.GetInteger());
			token = string();
		} else {
			lyxerr << "InsetMinipage::Read: Missing 'position'-tag!"
				   << endl;
		}
	}
	if (lex.IsOK()) {
		if (token.empty()) {
			lex.next();
			token = lex.GetString();
		}
		if (token == "inner_position") {
			lex.next();
			inner_pos_ = static_cast<InnerPosition>(lex.GetInteger());
			token = string();
		} else {
			lyxerr << "InsetMinipage::Read: Missing 'inner_position'-tag!"
				   << endl;
		}
	}
	if (lex.IsOK()) {
		if (token.empty()) {
			lex.next();
			token = lex.GetString();
		}
		if (token == "height") {
			lex.next();
			height_ = lex.GetString();
			token = string();
		} else {
			lyxerr << "InsetMinipage::Read: Missing 'height'-tag!"
				   << endl;
		}
	}
	if (lex.IsOK()) {
		if (token.empty()) {
			lex.next();
			token = lex.GetString();
		}
		if (token == "width") {
			lex.next();
			width_ = lex.GetString();
			token = string();
		} else {
			lyxerr << "InsetMinipage::Read: Missing 'width'-tag!"
				   << endl;
		}
	}
#ifdef WITH_WARNINGS
#warning Remove me before final 1.2.0 (Jug)
#warning Can we please remove this as soon as possible? (Lgb)
#endif
	// this is only for compatibility to the intermediate format and should
	// vanish till the final 1.2.0!
	if (lex.IsOK()) {
		if (token.empty()) {
			lex.next();
			token = lex.GetString();
		}
		if (token == "widthp") {
			lex.next();
			// only do this if the width_-string was not already set!
			if (width_.empty())
				width_ = lex.GetString() + "%";
			token = string();
		}
	}
	if (!token.empty())
		lex.pushToken(token);
	InsetCollapsable::read(buf, lex);
}


Inset * InsetMinipage::clone(Buffer const &, bool same_id) const
{
	InsetMinipage * result = new InsetMinipage;
	result->inset.init(&inset, same_id);
	
	result->collapsed_ = collapsed_;
	result->pos_ = pos_;
	result->inner_pos_ = inner_pos_;
	result->height_ = height_;
	result->width_ = width_;
	if (same_id)
		result->id_ = id_;
	return result;
}


int InsetMinipage::ascent(BufferView * bv, LyXFont const & font) const
{
	if (collapsed_)
		return ascent_collapsed(bv->painter(), font);
	else {
		// Take placement into account.
		int i = 0;
		switch (pos_) {
		case top:
			i = InsetCollapsable::ascent(bv, font);
			break;
		case center:
			i = (InsetCollapsable::ascent(bv, font)
			     + InsetCollapsable::descent(bv, font)) / 2;
			break;
		case bottom:
			i = InsetCollapsable::descent(bv, font);
			break;
		}
		return i;
	}
}


int InsetMinipage::descent(BufferView * bv, LyXFont const & font) const
{
	if (collapsed_)
		return descent_collapsed(bv->painter(), font);
	else {
		// Take placement into account.
		int i = 0;
		switch (pos_) {
		case top:
			i = InsetCollapsable::descent(bv, font);
			break;
		case center:
			i = (InsetCollapsable::ascent(bv, font)
			     + InsetCollapsable::descent(bv, font)) / 2;
			break;
		case bottom:
			i = InsetCollapsable::ascent(bv, font);
			break;
		}
		return i;
	}
}


string const InsetMinipage::editMessage() const
{
	return _("Opened Minipage Inset");
}


int InsetMinipage::latex(Buffer const * buf,
			 ostream & os, bool fragile, bool fp) const
{
	string s_pos;
	switch (pos_) {
	case top:
		s_pos += "t";
		break;
	case center:
		s_pos += "c";
		break;
	case bottom:
		s_pos += "b";
		break;
	}
	os << "\\begin{minipage}[" << s_pos << "]{"
	   << LyXLength(width_).asLatexString() << "}%\n";
	
	int i = inset.latex(buf, os, fragile, fp);

	os << "\\end{minipage}%\n";
	return i + 2;
}


bool InsetMinipage::insertInsetAllowed(Inset * in) const
{
	return insertInsetAllowed(in->lyxCode());
}

bool InsetMinipage::insertInsetAllowed(Inset::Code code) const
{
	if ((code == Inset::FLOAT_CODE) || (code == Inset::MARGIN_CODE))
		return false;

	return InsetCollapsable::insertInsetAllowed(code);
}


InsetMinipage::Position InsetMinipage::pos() const 
{
	return pos_;
}


void InsetMinipage::pos(InsetMinipage::Position p)
{
	pos_ = p;
}


InsetMinipage::InnerPosition InsetMinipage::innerPos() const
{
	return inner_pos_;
}


void InsetMinipage::innerPos(InsetMinipage::InnerPosition ip)
{
	inner_pos_ = ip;
}


string const & InsetMinipage::height() const
{
	return height_;
}


void InsetMinipage::height(string const & ll)
{
	height_ = ll;
}


string const & InsetMinipage::width() const
{
	return width_;
}


void InsetMinipage::width(string const & ll)
{
	width_ = ll;
}


bool InsetMinipage::showInsetDialog(BufferView * bv) const
{
	if (!inset.showInsetDialog(bv))
	bv->owner()->getDialogs()->showMinipage(const_cast<InsetMinipage *>(this));
	return true;
}


void InsetMinipage::insetButtonRelease(BufferView * bv, int x, int y,
				       int button)
{
	if (button == 3) {
	showInsetDialog(bv);
	return;
	}
	InsetCollapsable::insetButtonRelease(bv, x, y, button);
}


int InsetMinipage::getMaxWidth(BufferView * bv, UpdatableInset const * inset)
	const
{
	if (!width_.empty())
	return VSpace(width_).inPixels(bv);
	// this should not happen!
	return InsetCollapsable::getMaxWidth(bv, inset);
}
