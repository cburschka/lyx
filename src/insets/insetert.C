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
	setLabel(_("666"));
#ifndef NO_LATEX
	LyXFont font(LyXFont::ALL_SANE);
	font.setLatex (LyXFont::ON);
#endif
	labelfont = LyXFont(LyXFont::ALL_SANE);
	labelfont.decSize();
	labelfont.decSize();
	labelfont.setColor(LColor::latex);
	setAutoCollapse(false);
	setInsetName("ERT");
}


void InsetERT::Write(Buffer const * buf, ostream & os) const 
{
	os << getInsetName() << "\n";
	InsetCollapsable::Write(buf, os);
}


Inset * InsetERT::Clone(Buffer const &) const
{
	InsetERT * result = new InsetERT;
	result->inset.init(&inset);
	
	result->collapsed = collapsed;
	return result;
}


string const InsetERT::EditMessage() const 
{
	return _("Opened ERT Inset");
}


bool InsetERT::InsertInset(BufferView *, Inset *)
{
	return false;
}


void InsetERT::SetFont(BufferView *, LyXFont const &, bool, bool selectall)
{
	// if selectall is activated then the fontchange was an outside general
	// fontchange and this messages is not needed
	if (!selectall)
		WriteAlert(_("Impossible Operation!"),
		           _("Not permitted to change font-types inside ERT-insets!"),
		           _("Sorry."));
}


void InsetERT::Edit(BufferView * bv, int x, int y, unsigned int button)
{
	InsetCollapsable::Edit(bv, x, y, button);
#ifndef NO_LATEX
	LyXFont font(LyXFont::ALL_SANE);
	font.setLatex (LyXFont::ON);
#endif
}


int InsetERT::Latex(Buffer const *, std::ostream & os, bool /*fragile*/,
		    bool /*free_spc*/) const
{
	Paragraph::size_type siz = inset.par->size();
	for (Paragraph::size_type i = 0; i != siz; ++i) {
		os << inset.par->getChar(i);
	}
	return 1;
}


int InsetERT::Ascii(Buffer const *,
		    std::ostream &, int /*linelen*/) const 
{
	return 0;
}


int InsetERT::Linuxdoc(Buffer const *, std::ostream &) const
{
	return 0;
}


int InsetERT::DocBook(Buffer const *, std::ostream &) const
{
	return 0;
}
