/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *          Copyright (C) 1998 The LyX Team.
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


InsetFoot::InsetFoot(Buffer * bf): InsetCollapsable(bf)
{
    setLabel(_("foot"));
    LyXFont font(LyXFont::ALL_SANE);
    font.decSize();
    font.decSize();
    font.setColor(LColor::footnote);
    setLabelFont(font);
    setAutoCollapse(false);
}


InsetFoot * InsetFoot::Clone() const
{
    InsetFoot * result = new InsetFoot(buffer);
    return result;
}


const char * InsetFoot::EditMessage() const
{
    return _("Opened Footnote Inset");
}

#ifndef USE_OSTREAM_ONLY
int InsetFoot::Latex(string & l, signed char fragile) const
{
    int i;
    
    if (fragile) 
	l += "\\footnotetext{";
    else 
	l += "\\footnote{";

    i = InsetText::Latex(l, fragile);
    l += "}";

    return i;
}
#endif

int InsetFoot::Latex(ostream & os, signed char fragile, bool fp) const
{
    int i;
    
    if (fragile) 
	os << "\\footnotetext{";
    else 
	os << "\\footnote{";

    i = InsetText::Latex(os, fragile, fp);
    os << "}";

    return i;
}

void InsetFoot::Write(ostream & os) const
{
    os << "Foot\n";
    os << "\ncollapsed ";
    if (display())
	os << "false";
    else
	os << "true";
    os << "\n";
    WriteParagraphData(os);
}

void InsetFoot::Read(LyXLex & lex)
{
    if (lex.IsOK()) {
	string token, tmptok;
        
	lex.next();
        token = lex.GetString();
	if (token == "collapsed") {
	    lex.next();
	    collapsed = lex.GetBool();
	}
    }
    InsetText::Read(lex);
}

bool InsetFoot::InsertInset(BufferView *bv, Inset * inset)
{
    if ((inset->LyxCode() == Inset::FOOT_CODE) ||
	(inset->LyxCode() == Inset::MARGIN_CODE)) {
	return false;
    }
    return InsetText::InsertInset(bv, inset);
}
