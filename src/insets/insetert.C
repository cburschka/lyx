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
#include "Painter.h"
#include "buffer.h"
//#include "lyx_gui_misc.h"


InsetERT::InsetERT(Buffer * bf)
	: InsetCollapsable(bf)
{
    setLabel(_("ERT"));
    LyXFont font(LyXFont::ALL_SANE);
    font.setLatex (LyXFont::ON);
    real_current_font = current_font = font;
    LyXFont labelfont(LyXFont::ALL_SANE);
    labelfont.decSize();
    labelfont.decSize();
    labelfont.setColor(LColor::ert);
    setLabelFont(labelfont);
    setAutoCollapse(false);
}


Inset * InsetERT::Clone() const
{
    InsetERT * result = new InsetERT(buffer);
    return result;
}


void InsetERT::Write(ostream & os) const
{
    os << "ERT\n";
    WriteParagraphData(os);
}

bool InsetERT::InsertInset(BufferView *, Inset *)
{
    return false;
}


void InsetERT::SetFont(BufferView *, LyXFont const &, bool)
{
    WriteAlert(_("Impossible Operation!"),
	       _("Not permitted to change font-types inside ERT-insets!"),
	       _("Sorry."));
}
