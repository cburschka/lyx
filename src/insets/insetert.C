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

#include "insetert.h"
#include "gettext.h"
#include "lyxfont.h"
#include "buffer.h"
#include "insets/insettext.h"
#include "support/LOstream.h"
#include "lyx_gui_misc.h"

using std::ostream;

InsetERT::InsetERT() : InsetCollapsable()
{
    setLabel(_("ERT"));
    LyXFont font(LyXFont::ALL_SANE);
    font.setLatex (LyXFont::ON);
    labelfont = LyXFont(LyXFont::ALL_SANE);
    labelfont.decSize();
    labelfont.decSize();
    labelfont.setColor(LColor::ert);
    setAutoCollapse(false);
    setInsetName("ERT");
}


void InsetERT::Write(Buffer const * buf, ostream & os) const 
{
    os << getInsetName() << "\n";
    InsetCollapsable::Write(buf, os);
}


Inset * InsetERT::Clone() const
{
    InsetERT * result = new InsetERT;
    result->inset->init(inset);

    result->collapsed = collapsed;
    return result;
}


char const * InsetERT::EditMessage() const 
{
    return _("Opened ERT Inset");
}


void InsetERT::SetFont(BufferView *, LyXFont const &, bool)
{
    WriteAlert(_("Impossible Operation!"),
	       _("Not permitted to change font-types inside ERT-insets!"),
	       _("Sorry."));
}


void InsetERT::Edit(BufferView * bv, int x, int y, unsigned int button)
{
    InsetCollapsable::Edit(bv, x, y, button);
    LyXFont font(LyXFont::ALL_SANE);
    font.setLatex (LyXFont::ON);
}
