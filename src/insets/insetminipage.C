/**
 * \file insetminipage.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "insetminipage.h"
#include "insettext.h"

#include "BufferView.h"
#include "debug.h"
#include "funcrequest.h"
#include "gettext.h"
#include "lyxfont.h"
#include "lyxlex.h"
#include "lyxtext.h"

#include "frontends/LyXView.h"
#include "frontends/Dialogs.h"

#include "support/LOstream.h"
#include "support/lstrings.h"

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

InsetMinipage::InsetMinipage(BufferParams const & bp)
	: InsetCollapsable(bp)
{
	setLabel(_("minipage"));
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.decSize();
	font.setColor(LColor::collapsable);
	setLabelFont(font);
#if 0
	setAutoCollapse(false);
#endif

#if 0
#ifdef WITH_WARNINGS
#warning Remove this color definitions before 1.2.0 final!
#endif
	// just for experimentation :)
	setBackgroundColor(LColor::green);
#endif

	inset.setFrameColor(0, LColor::blue);
	setInsetName("Minipage");
}


InsetMinipage::InsetMinipage(InsetMinipage const & in, bool same_id)
	: InsetCollapsable(in, same_id), params_(in.params_)
{}


Inset * InsetMinipage::clone(Buffer const &, bool same_id) const
{
	return new InsetMinipage(*const_cast<InsetMinipage *>(this), same_id);
}


InsetMinipage::~InsetMinipage()
{
	InsetMinipageMailer mailer(*this);
	mailer.hideDialog();
}


dispatch_result InsetMinipage::localDispatch(FuncRequest const & cmd)
{
	Inset::RESULT result = UNDISPATCHED;

	switch (cmd.action) {
	case LFUN_INSET_MODIFY: {
		InsetMinipage::Params params;
		InsetMinipageMailer::string2params(cmd.argument, params);

		params_.pos   = params.pos;
		params_.width = params.width;

		cmd.view()->updateInset(this);
		result = DISPATCHED;
	}
	break;

	case LFUN_INSET_DIALOG_UPDATE: {
		InsetMinipageMailer mailer(*this);
		mailer.updateDialog(cmd.view());
	}
	break;

	default:
		result = InsetCollapsable::localDispatch(cmd);
	}

	return result;
}


void InsetMinipage::Params::write(ostream & os) const
{
	os << "Minipage" << '\n'
	   << "position " << pos << '\n'
	   << "inner_position " << inner_pos << '\n'
	   << "height \"" << height.asString() << "\"\n"
	   << "width \"" << width.asString() << "\"\n";
}


void InsetMinipage::Params::read(LyXLex & lex)
{
	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token == "position") {
			lex.next();
			pos = static_cast<Position>(lex.getInteger());
		} else {
			lyxerr << "InsetMinipage::Read: Missing 'position'-tag!"
				   << endl;
			// take countermeasures
			lex.pushToken(token);
		}
	}
	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token == "inner_position") {
			lex.next();
			inner_pos = static_cast<InnerPosition>(lex.getInteger());
		} else {
			lyxerr << "InsetMinipage::Read: Missing 'inner_position'-tag!"
				   << endl;
			// take countermeasures
			lex.pushToken(token);
		}
	}
	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token == "height") {
			lex.next();
			height = LyXLength(lex.getString());
		} else {
			lyxerr << "InsetMinipage::Read: Missing 'height'-tag!"
				   << endl;
			// take countermeasures
			lex.pushToken(token);
		}
	}
	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token == "width") {
			lex.next();
			width = LyXLength(lex.getString());
		} else {
			lyxerr << "InsetMinipage::Read: Missing 'width'-tag!"
				   << endl;
			// take countermeasures
			lex.pushToken(token);
		}
	}
}


void InsetMinipage::write(Buffer const * buf, ostream & os) const
{
	params_.write(os);
	InsetCollapsable::write(buf, os);
}


void InsetMinipage::read(Buffer const * buf, LyXLex & lex)
{
	params_.read(lex);
	InsetCollapsable::read(buf, lex);
}


int InsetMinipage::ascent(BufferView * bv, LyXFont const & font) const
{
	if (collapsed_)
		return ascent_collapsed();
	else {
		// Take placement into account.
		int i = 0;
		switch (params_.pos) {
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
		return descent_collapsed();
	else {
		// Take placement into account.
		int i = 0;
		switch (params_.pos) {
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
	switch (params_.pos) {
	case top:
		s_pos += 't';
		break;
	case center:
		s_pos += 'c';
		break;
	case bottom:
		s_pos += 'b';
		break;
	}
	os << "\\begin{minipage}[" << s_pos << "]{"
	   << params_.width.asLatexString() << "}%\n";

	int i = inset.latex(buf, os, fragile, fp);

	os << "\\end{minipage}%\n";
	return i + 2;
}


bool InsetMinipage::insetAllowed(Inset::Code code) const
{
	if ((code == Inset::FLOAT_CODE) || (code == Inset::MARGIN_CODE))
		return false;

	return InsetCollapsable::insetAllowed(code);
}


bool InsetMinipage::showInsetDialog(BufferView * bv) const
{
	if (!inset.showInsetDialog(bv)) {
		InsetMinipage * tmp = const_cast<InsetMinipage *>(this);
		InsetMinipageMailer mailer(*tmp);
		mailer.showDialog(bv);
	}

	return true;
}


int InsetMinipage::getMaxWidth(BufferView * bv, UpdatableInset const * inset)
	const
{
	if (owner() &&
	    static_cast<UpdatableInset*>(owner())->getMaxWidth(bv, inset) < 0) {
		return -1;
	}
	if (!params_.width.zero()) {
		int ww1 = latexTextWidth(bv);
		int ww2 = InsetCollapsable::getMaxWidth(bv, inset);
		if (ww2 > 0 && ww2 < ww1) {
			return ww2;
		}
		return ww1;
	}
	// this should not happen!
	return InsetCollapsable::getMaxWidth(bv, inset);
}


int InsetMinipage::latexTextWidth(BufferView * bv) const
{
	return params_.width.inPixels(InsetCollapsable::latexTextWidth(bv));
}


InsetMinipage::Params::Params()
	: pos(center),
	  inner_pos(inner_center),
	  width(100, LyXLength::PCW)
{}


string const InsetMinipageMailer:: name_("minipage");

InsetMinipageMailer::InsetMinipageMailer(InsetMinipage & inset)
	: inset_(inset)
{}


string const InsetMinipageMailer::inset2string() const
{
	return params2string(inset_.params());
}


void InsetMinipageMailer::string2params(string const & in,
					InsetMinipage::Params & params)
{
	params = InsetMinipage::Params();

	istringstream data(in);
	LyXLex lex(0,0);
	lex.setStream(data);

	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token != "minipage")
			return;
	}

	// This is part of the inset proper that is usually swallowed
	// by Buffer::readInset
	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token != "Minipage")
			return;
	}

	if (lex.isOK()) {
		params.read(lex);
	}
}


string const
InsetMinipageMailer::params2string(InsetMinipage::Params const & params)
{
	ostringstream data;
	data << name_ << ' ';
	params.write(data);

	return data.str();
}
