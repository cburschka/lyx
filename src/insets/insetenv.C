/**
 * \file InsetEnv.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetEnv.h"

#include "bufferparams.h"
#include "gettext.h"
#include "paragraph.h"
#include "outputparams.h"
#include "output_latex.h"
#include "texrow.h"


namespace lyx {

using std::string;
using std::auto_ptr;
using std::ostream;


InsetEnvironment::InsetEnvironment
		(BufferParams const & bp, string const & name)
	: InsetText(bp), layout_(bp.getLyXTextClass()[name])
{
	setInsetName(from_utf8(name));
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
	os << "Environment " << to_utf8(getInsetName()) << "\n";
	InsetText::write(buf, os);
}


void InsetEnvironment::read(Buffer const & buf, LyXLex & lex)
{
	InsetText::read(buf, lex);
}


docstring const InsetEnvironment::editMessage() const
{
	return _("Opened Environment Inset: ") + getInsetName();
}


int InsetEnvironment::latex(Buffer const & buf, odocstream & os,
                            OutputParams const & runparams) const
{
	// FIXME UNICODE
	os << from_utf8(layout_->latexheader);
	TexRow texrow;
	latexParagraphs(buf, paragraphs(), os, texrow, runparams,
			layout_->latexparagraph);
	// FIXME UNICODE
	os << from_utf8(layout_->latexfooter);
	return texrow.rows();
}


int InsetEnvironment::plaintext(Buffer const & buf, odocstream & os,
                                OutputParams const & runparams) const
{
	os << '[' << to_utf8(getInsetName()) << ":\n";
	InsetText::plaintext(buf, os, runparams);
	os << "\n]";

	return PLAINTEXT_NEWLINE + 1; // one char on a separate line
}


LyXLayout_ptr const & InsetEnvironment::layout() const
{
	return layout_;
}


} // namespace lyx
