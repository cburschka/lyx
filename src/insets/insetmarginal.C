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

#include "insetmarginal.h"
#include "gettext.h"
#include "lyxfont.h"
#include "BufferView.h"
#include "Painter.h"
#include "lyxtext.h"
#include "support/LOstream.h"

using std::ostream;
using std::endl;

InsetMarginal::InsetMarginal() : InsetCollapsable()
{
	setLabel(_("margin"));
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.decSize();
	font.setColor(LColor::footnote);
	setLabelFont(font);
	setAutoCollapse(false);
	setInsetName("Marginal");
}


Inset * InsetMarginal::Clone() const
{
	InsetMarginal * result = new InsetMarginal;
	result->init(this);

	result->collapsed = collapsed;
	return result;
}


char const * InsetMarginal::EditMessage() const
{
	return _("Opened Marginal Note Inset");
}


int InsetMarginal::Latex(Buffer const * buf,
			 ostream & os, bool fragile, bool fp) const
{
    os << "\\marginpar{%" << endl;
    
    int i = InsetText::Latex(buf, os, fragile, fp);
    os << "}%" << endl;
    
    return i + 2;
}


bool InsetMarginal::InsertInset(BufferView * bv, Inset * inset)
{
    if (!InsertInsetAllowed(inset))
	return false;

    return InsetText::InsertInset(bv, inset);
}


bool InsetMarginal::InsertInsetAllowed(Inset * inset) const
{
    if ((inset->LyxCode() == Inset::FOOT_CODE) ||
	(inset->LyxCode() == Inset::MARGIN_CODE)) {
	return false;
    }
    return true;
}


LyXFont InsetMarginal::GetDrawFont(BufferView * bv,
				   LyXParagraph * p, int pos) const
{
    LyXFont fn = getLyXText(bv)->GetFont(bv->buffer(), p, pos);
    fn.decSize().decSize();
    return fn;
}
