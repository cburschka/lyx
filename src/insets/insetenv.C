/**
 * \file insetenv.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetenv.h"

#include "bufferparams.h"
#include "gettext.h"
#include "paragraph.h"
#include "output_latex.h"
#include "texrow.h"

#include "support/std_ostream.h"


using std::string;
using std::auto_ptr;
using std::ostream;


InsetEnvironment::InsetEnvironment
		(BufferParams const & bp, string const & name)
	: InsetText(bp), layout_(bp.getLyXTextClass()[name])
{
	setInsetName(name);
	setAutoBreakRows(true);
	setDrawFrame(true);
}


InsetEnvironment::InsetEnvironment(InsetEnvironment const & in)
	: InsetText(in), layout_(in.layout_)
{}


auto_ptr<InsetBase> InsetEnvironment::doClone() const
{
	return auto_ptr<InsetBase>(new InsetEnvironment(*this));
}


void InsetEnvironment::write(Buffer const & buf, ostream & os) const
{
	os << "Environment " << getInsetName() << "\n";
	InsetText::write(buf, os);
}


void InsetEnvironment::read(Buffer const & buf, LyXLex & lex)
{
	InsetText::read(buf, lex);
}


string const InsetEnvironment::editMessage() const
{
	return _("Opened Environment Inset: ") + getInsetName();
}


int InsetEnvironment::latex(Buffer const & buf, ostream & os,
			    OutputParams const & runparams) const
{
	os << layout_->latexheader;
	TexRow texrow;
	latexParagraphs(buf, paragraphs(), os, texrow, runparams,
			layout_->latexparagraph);
	os << layout_->latexfooter;
	return texrow.rows();
}


LyXLayout_ptr const & InsetEnvironment::layout() const
{
	return layout_;
}
