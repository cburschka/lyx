/**
 * \file insetnote.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "insetnote.h"
#include "gettext.h"
#include "lyxfont.h"
#include "language.h"
#include "buffer.h"
#include "BufferView.h"
#include "lyxtext.h"
#include "insets/insettext.h"
#include "support/LOstream.h"
#include "support/lstrings.h"
#include "debug.h"

using std::ostream;


void InsetNote::init()
{
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.decSize();
	font.setColor(LColor::note);
	setLabelFont(font);
	setBackgroundColor(LColor::notebg);
	setLabel(_("note"));
	setInsetName("Note");
}


InsetNote::InsetNote(BufferParams const & bp)
	: InsetCollapsable(bp)
{
	init();
}


InsetNote::InsetNote(InsetNote const & in)
	: InsetCollapsable(in)
{
	init();
}


Inset * InsetNote::clone() const
{
	return new InsetNote(*this);
}


string const InsetNote::editMessage() const
{
	return _("Opened Note Inset");
}


void InsetNote::write(Buffer const * buf, ostream & os) const
{
	os << getInsetName() << "\n";
	InsetCollapsable::write(buf, os);
}
