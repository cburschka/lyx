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
#include "lyxtext.h"
#include "insets/insettext.h"
#include "support/LOstream.h"
#include "debug.h"

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
	: InsetCollapsable()
{
	setLabel(_("minipage"));
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.decSize();
	font.setColor(LColor::footnote);
	setLabelFont(font);
	setAutoCollapse(false);
	setInsetName("Minipage");
}


void InsetMinipage::Write(Buffer const * buf, ostream & os) const 
{
	os << getInsetName() << "\n";
	InsetCollapsable::Write(buf, os);
}


Inset * InsetMinipage::Clone(Buffer const &) const
{
	InsetMinipage * result = new InsetMinipage;
	result->inset->init(inset);
	
	result->collapsed = collapsed;
	return result;
}


string const InsetMinipage::EditMessage() const
{
	return _("Opened Minipage Inset");
}


int InsetMinipage::Latex(Buffer const * buf,
			 ostream & os, bool fragile, bool fp) const
{
	os << "\\begin{minipage}{\\columnwidth}%\n";
	
	int i = inset->Latex(buf, os, fragile, fp);
	os << "\\end{minipage}%\n";
	
	return i + 2;
}


bool InsetMinipage::InsertInsetAllowed(Inset * in) const
{
	if ((in->LyxCode() == Inset::FLOAT_CODE) ||
	    (in->LyxCode() == Inset::MARGIN_CODE)) {
		return false;
	}
	return true;
}
