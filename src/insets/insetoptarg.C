/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "debug.h"

#include "insetoptarg.h"
#include "support/LOstream.h"
#include "frontends/Alert.h"
#include "support/lstrings.h" //frontStrip, strip
#include "lyxtext.h"
#include "buffer.h"
#include "gettext.h"
#include "BufferView.h"
#include "support/lstrings.h"

using std::ostream;
using std::vector;
using std::pair;

/* OptArg. Used to insert a short version of sectioning header etc.
 *  automatically, or other optional LaTeX arguments */


InsetOptArg::InsetOptArg(BufferParams const & ins)
	: InsetCollapsable(ins, true)
{
    LyXFont font(LyXFont::ALL_SANE);
	font.setColor(LColor::collapsable);
	setLabelFont(font);
	setLabel(_("opt"));
}

InsetOptArg::InsetOptArg(InsetOptArg const & in, bool same_id)
		    : InsetCollapsable(in, same_id)
{
    LyXFont font(LyXFont::ALL_SANE);
	font.setColor(LColor::collapsable);
	setLabelFont(font);
	setLabel(_("opt"));
}

Inset * InsetOptArg::clone(Buffer const &, bool same_id) const
{
		    return new InsetOptArg(*this, same_id);
}

string const InsetOptArg::editMessage() const
{
	return _("Opened Optional Argument Inset");
}

void InsetOptArg::write(Buffer const * buf, ostream & os) const
{
	os << "OptArg" << "\n";
	InsetCollapsable::write(buf, os);
}

int InsetOptArg::latex(Buffer const *, ostream &, bool, bool) const
{
	return 0;
}

int InsetOptArg::latexOptional(Buffer const * buf, ostream & os,
				bool, bool fp) const
{
	os << '[';
	int const i = inset.latex(buf, os, false, fp);
	os << ']';
	return i + 2;
}

