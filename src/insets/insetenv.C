// -*- C++ -*-
/**
 * \file insetenv.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "insetenv.h"
#include "gettext.h"
#include "lyxtextclass.h"
#include "lyxlayout.h"
#include "bufferparams.h"
#include "support/LOstream.h"
#include "debug.h"


using std::ostream;
using std::endl;


InsetEnvironment::InsetEnvironment
		(BufferParams const & bp, string const & name)
	: InsetText(bp)
{
	//setLabel(name);
	setInsetName(name);
	autoBreakRows = true;
	drawFrame_ = ALWAYS;
	// needs more stuff in lyxlayout. coming in later patches.
	//LyXTextClass const & tc = bp.getLyXTextClass();
	//LyXLayout_ptr const & layout = tc.getEnv(name);
	//header_ = layout->latexheader;
	//footer_ = layout->latexfooter;
	header_ = "\\begin{" + name + "}";
	footer_ = "\\end{" + name + "}";
}


InsetEnvironment::InsetEnvironment(InsetEnvironment const & in, bool same_id)
	: InsetText(in, same_id), header_(in.header_), footer_(in.footer_)
{}


Inset * InsetEnvironment::clone(Buffer const &, bool same_id) const
{
	return new InsetEnvironment(*this, same_id);
}


void InsetEnvironment::write(Buffer const * buf, ostream & os) const
{
	os << "Environment " << getInsetName() << "\n";
	InsetText::write(buf, os);
}


void InsetEnvironment::read(Buffer const * buf, LyXLex & lex)
{
	InsetText::read(buf, lex);
}


string const InsetEnvironment::editMessage() const
{
	return _("Opened Environment Inset: ") + getInsetName();
}


int InsetEnvironment::latex(Buffer const * buf,
			 ostream & os, bool fragile, bool fp) const
{
	os << header_;
	int i = InsetText::latex(buf, os, fragile, fp);
	os << footer_;
	return i;
}
