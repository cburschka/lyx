/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *          Copyright 1998 The LyX Team.
 *
 * ======================================================
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetfoot.h"
#include "gettext.h"
#include "lyxfont.h"
#include "BufferView.h"
#include "Painter.h"
#include "lyxtext.h"
#include "insets/insettext.h"
#include "support/LOstream.h"

using std::ostream;
using std::endl;

InsetFoot::InsetFoot() : InsetCollapsable()
{
    setLabel(_("foot"));
    LyXFont font(LyXFont::ALL_SANE);
    font.decSize();
    font.decSize();
    font.setColor(LColor::footnote);
    setLabelFont(font);
    setAutoCollapse(false);
    setInsetName("Foot");
}


void InsetFoot::Write(Buffer const * buf, ostream & os) const 
{
    os << getInsetName() << "\n";
    InsetCollapsable::Write(buf, os);
}


Inset * InsetFoot::Clone() const
{
    InsetFoot * result = new InsetFoot;
    result->inset->init(inset);

    result->collapsed = collapsed;
    return result;
}


char const * InsetFoot::EditMessage() const
{
    return _("Opened Footnote Inset");
}


int InsetFoot::Latex(Buffer const * buf,
		     ostream & os, bool fragile, bool fp) const
{
    os << "\\footnote{%\n";
    
    int i = inset->Latex(buf, os, fragile, fp);
    os << "}%\n";
    
    return i + 2;
}


bool InsetFoot::InsertInset(BufferView * bv, Inset * in)
{
    if (!InsertInsetAllowed(in))
	return false;

    return inset->InsertInset(bv, in);
}


bool InsetFoot::InsertInsetAllowed(Inset * in) const
{
    if ((in->LyxCode() == Inset::FOOT_CODE) ||
	(in->LyxCode() == Inset::MARGIN_CODE)) {
	return false;
    }
    return true;
}

#if 0
LyXFont InsetFoot::GetDrawFont(BufferView * bv,
			       LyXParagraph * p, int pos) const
{
    LyXFont fn = getLyXText(bv)->GetFont(bv->buffer(), p, pos);
    fn.decSize().decSize();
    return fn;
}
#endif
