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

#include "insetnote.h"
#include "gettext.h"
#include "lyxfont.h"
#include "BufferView.h"
#include "lyxtext.h"
#include "insets/insettext.h"
#include "support/LOstream.h"
#include "debug.h"

using std::ostream;

InsetNote::InsetNote()
	: InsetCollapsable()
{
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.decSize();
	font.setColor(LColor::note);
	setLabelFont(font);
	setAutoCollapse(true);

	setLabel(_("note"));
	setInsetName("Note");
}


Inset * InsetNote::clone(Buffer const &, bool same_id) const
{
	InsetNote * result = new InsetNote;
	result->inset.init(&inset, same_id);

	result->collapsed_ = collapsed_;
	if (same_id)
		result->id_ = id_;
	return result;
}


string const InsetNote::editMessage() const 
{
	return _("Opened Note Inset");
}


void InsetNote::write(Buffer const *buf, ostream & os) const
{
	//os << getInsetName() << "\n";
	os << "Info\n";
	InsetCollapsable::write(buf, os);
}

