/**
 * \file InsetEnvironment.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetEnvironment.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "Layout.h"
#include "OutputParams.h"
#include "output_latex.h"
#include "TexRow.h"
#include "TextClass.h"

#include "support/gettext.h"

using namespace std;

namespace lyx {


InsetEnvironment::InsetEnvironment(Buffer const & buf, docstring const & name)
	: InsetText(buf), layout_(buf.params().documentClass()[name]), name_(name)
{
	setAutoBreakRows(true);
	setDrawFrame(true);
}


void InsetEnvironment::write(ostream & os) const
{
	os << "Environment " << to_utf8(name()) << "\n";
	InsetText::write(os);
}


void InsetEnvironment::read(Lexer & lex)
{
	InsetText::read(lex);
}


docstring InsetEnvironment::editMessage() const
{
	return _("Opened Environment Inset: ") + name();
}


int InsetEnvironment::latex(odocstream & os,
			    OutputParams const & runparams) const
{
	// FIXME UNICODE
	os << from_utf8(layout_.latexheader);
	TexRow texrow;
	latexParagraphs(buffer(), text_, os, texrow, runparams,
			layout_.latexparagraph);
	// FIXME UNICODE
	os << from_utf8(layout_.latexfooter);
	return texrow.rows();
}


int InsetEnvironment::plaintext(odocstream & os,
				OutputParams const & runparams) const
{
	os << '[' << to_utf8(name()) << ":\n";
	InsetText::plaintext(os, runparams);
	os << "\n]";

	return PLAINTEXT_NEWLINE + 1; // one char on a separate line
}


Layout const & InsetEnvironment::layout() const
{
	return layout_;
}


} // namespace lyx
