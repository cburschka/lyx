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
#include "paragraph_funcs.h"
#include "lyxlayout.h"
#include "bufferparams.h"
#include "support/LOstream.h"
#include "debug.h"


using std::ostream;
using std::endl;


InsetEnvironment::InsetEnvironment
		(BufferParams const & bp, string const & name)
	: InsetText(bp), layout_(bp.getLyXTextClass()[name])
{
	setInsetName(name);
	autoBreakRows = true;
	drawFrame_ = ALWAYS;
}


InsetEnvironment::InsetEnvironment(InsetEnvironment const & in)
	: InsetText(in), layout_(in.layout_)
{}


Inset * InsetEnvironment::clone() const
{
	return new InsetEnvironment(*this);
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


int InsetEnvironment::latex(Buffer const * buf, ostream & os,
			    LatexRunParams const & runparams) const
{
	os << layout_->latexheader;
	TexRow texrow;
	latexParagraphs(buf, paragraphs, os, texrow, runparams,
			layout_->latexparagraph);
	os << layout_->latexfooter;
	return texrow.rows();
}


LyXLayout_ptr const & InsetEnvironment::layout() const
{
	return layout_;
}
