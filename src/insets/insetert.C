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

void InsetERT::init()
{
	setLabel(_("666"), true);
	labelfont = LyXFont(LyXFont::ALL_SANE);
	labelfont.decSize();
	labelfont.decSize();
#ifndef NO_LATEX
	labelfont.setLatex(LyXFont::ON);
#else
	labelfont.setColor(LColor::latex);
#endif
	setAutoCollapse(false);
	setInsetName("ERT");
}

InsetERT::InsetERT() : InsetCollapsable()
{
	init();
}


InsetERT::InsetERT(string const & contents, bool collapsed)
	: InsetCollapsable(collapsed)
{
	init();

	LyXFont const font(LyXFont::ALL_INHERIT);
	string::const_iterator cit = contents.begin();
	string::const_iterator end = contents.end();
	Paragraph::size_type pos = 0;
	for (; cit != end; ++cit) {
		inset.paragraph()->insertChar(pos++, *cit, font);
	}
}


void InsetERT::write(Buffer const * buf, ostream & os) const 
{
	os << getInsetName() << "\n";
	InsetCollapsable::write(buf, os);
}


Inset * InsetERT::clone(Buffer const &, bool same_id) const
{
	InsetERT * result = new InsetERT;
	result->inset.init(&inset, same_id);
	
	result->collapsed_ = collapsed_;
	if (same_id)
		result->id_ = id_;
	return result;
}


string const InsetERT::editMessage() const 
{
	return _("Opened ERT Inset");
}


bool InsetERT::insertInset(BufferView *, Inset *)
{
	return false;
}


void InsetERT::setFont(BufferView *, LyXFont const &, bool, bool selectall)
{
	// if selectall is activated then the fontchange was an outside general
	// fontchange and this messages is not needed
	if (!selectall)
		WriteAlert(_("Impossible Operation!"),
		           _("Not permitted to change font-types inside ERT-insets!"),
		           _("Sorry."));
}


void InsetERT::edit(BufferView * bv, int x, int y, unsigned int button)
{
	InsetCollapsable::edit(bv, x, y, button);
#ifndef NO_LATEX
	LyXFont font(LyXFont::ALL_SANE);
	font.setLatex (LyXFont::ON);
#endif
}


void InsetERT::edit(BufferView * bv, bool)
{
	edit(bv, 0, 0, 0);
}


int InsetERT::latex(Buffer const *, std::ostream & os, bool /*fragile*/,
		    bool /*free_spc*/) const
{
	Paragraph * par = inset.paragraph();
	while (par) {
		Paragraph::size_type siz = inset.paragraph()->size();
		for (Paragraph::size_type i = 0; i != siz; ++i) {
			char c = inset.paragraph()->getChar(i);
			switch (c) {
			case Paragraph::META_NEWLINE:
				os << '\n';
				break;
			default:
				os << c;
				break;
			}
		}
		par = par->next();
	}
	
	return 1;
}


int InsetERT::ascii(Buffer const *,
		    std::ostream &, int /*linelen*/) const 
{
	return 0;
}


int InsetERT::linuxdoc(Buffer const *, std::ostream &) const
{
	return 0;
}


int InsetERT::docBook(Buffer const *, std::ostream &) const
{
	return 0;
}
