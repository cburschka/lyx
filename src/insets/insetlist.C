/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *          Copyright 1998 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetlist.h"
#include "gettext.h"
#include "lyxfont.h"
#include "BufferView.h"
#include "lyxtext.h"
#include "insets/insettext.h"
#include "support/LOstream.h"
#include "debug.h"

using std::ostream;
using std::endl;

// This class is _far_ from finished. I hope that we can have a inset to
// handle the different lists that we have. It should also be possible
// to create new lists on the fly.
// Currently LyX only supports: itemize, enumerate, description and
// lyxlist. All support for these should be moved to this class and other
// helper classes.
// It is also possible that we will need a baseclass and subclasses for
// different types of lists. (and should they be collapsable?)
//
// Lgb

InsetList::InsetList()
	: InsetCollapsable()
{
	setLabel(_("list"));
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.decSize();
	font.setColor(LColor::collapsable);
	setLabelFont(font);
	setAutoCollapse(false);
	setInsetName("List");
}


void InsetList::Write(Buffer const * buf, ostream & os) const
{
	os << getInsetName() << "\n";
	InsetCollapsable::Write(buf, os);
}


Inset * InsetList::Clone(Buffer const &) const
{
	InsetList * result = new InsetList;
	result->inset.init(&inset);
	
	result->collapsed = collapsed;
	return result;
}


string const InsetList::EditMessage() const
{
	return _("Opened List Inset");
}


int InsetList::Latex(Buffer const * buf,
		     ostream & os, bool fragile, bool fp) const
{
	os << "\\footnote{%\n";
	
	int i = inset.Latex(buf, os, fragile, fp);
	os << "}%\n";
	
	return i + 2;
}


bool InsetList::InsertInsetAllowed(Inset * in) const
{
	if ((in->LyxCode() == Inset::FOOT_CODE) ||
	    (in->LyxCode() == Inset::MARGIN_CODE)) {
		return false;
	}
	return true;
}
