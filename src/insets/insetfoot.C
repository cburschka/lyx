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

#include "insetfoot.h"
#include "gettext.h"
#include "lyxfont.h"
#include "BufferView.h"
#include "lyxscreen.h"
#include "Painter.h"
#include "support/LOstream.h"

using std::ostream;

InsetFoot::InsetFoot(Buffer * bf)
		: InsetCollapsable(bf)
{
    setLabel(_("foot"));
    LyXFont font(LyXFont::ALL_SANE);
    font.decSize();
    font.decSize();
    font.setColor(LColor::footnote);
    setLabelFont(font);
    setAutoCollapse(false);
}


Inset * InsetFoot::Clone() const
{
    InsetFoot * result = new InsetFoot(buffer);
    result->init(buffer, this);

    result->collapsed = collapsed;
    return result;
}


char const * InsetFoot::EditMessage() const
{
    return _("Opened Footnote Inset");
}


int InsetFoot::Latex(ostream & os, signed char fragile, bool fp) const
{
	if (fragile) 
		os << "\\footnote{"; // was footnotemark but that won't work
	else 
		os << "\\footnote{";
	
	int i = InsetText::Latex(os, fragile, fp);
	os << "}";
	
	return i;
}


void InsetFoot::Write(ostream & os) const
{
	os << "Foot\n"
	   << "\ncollapsed ";
	if (display())
		os << "false\n";
	else
		os << "true\n";
	WriteParagraphData(os);
}


void InsetFoot::Read(LyXLex & lex)
{
    if (lex.IsOK()) {
	lex.next();
        string token = lex.GetString();
	if (token == "collapsed") {
	    lex.next();
	    collapsed = lex.GetBool();
	}
    }
    InsetText::Read(lex);
}


bool InsetFoot::InsertInset(BufferView * bv, Inset * inset)
{
    if (!InsertInsetAllowed(inset))
	return false;

    return InsetText::InsertInset(bv, inset);
}

bool InsetFoot::InsertInsetAllowed(Inset * inset) const
{
    if ((inset->LyxCode() == Inset::FOOT_CODE) ||
	(inset->LyxCode() == Inset::MARGIN_CODE)) {
	return false;
    }
    return true;
}

LyXFont InsetFoot::GetDrawFont(LyXParagraph * par, int pos) const
{
    LyXFont fn = InsetCollapsable::GetDrawFont(par, pos);
    fn.decSize();
    fn.decSize();
    return fn;
}
