/**
 * \file InsetFlex.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Martin Vermeer
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetFlex.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "Paragraph.h"
#include "paragraph_funcs.h"
#include "sgml.h"
#include "Text.h"

#include "support/gettext.h"

#include <ostream>

using namespace std;

namespace lyx {


InsetFlex::InsetFlex(Buffer const & buf, string const & layoutName)
	: InsetCollapsable(buf), name_(layoutName)
{
	// again, because now the name is initialized
	setLayout(buf.params().documentClassPtr());
	status_= Collapsed;
}


InsetFlex::InsetFlex(InsetFlex const & in)
	: InsetCollapsable(in), name_(in.name_)
{}


docstring InsetFlex::editMessage() const
{
	return _("Opened Flex Inset");
}


void InsetFlex::write(ostream & os) const
{
	os << "Flex " <<
		(name_.empty() ? "undefined" : name_) << "\n";
	InsetCollapsable::write(os);
}


void InsetFlex::read(Lexer & lex)
{
	string token;
	while (lex.isOK()) {
		lex >> token;
		if (token == "Flex") {
			lex.next();
			name_ = lex.getString();
		} else if (token == "status") {
			// This is handled in Collapsable
			lex.pushToken(token);
			break;
		}
	}
	InsetCollapsable::read(lex);
}


int InsetFlex::plaintext(odocstream & os, OutputParams const & runparams) const
{
	return InsetText::plaintext(os, runparams);
}


int InsetFlex::docbook(odocstream & os, OutputParams const & runparams) const
{
	ParagraphList::const_iterator beg = paragraphs().begin();
	ParagraphList::const_iterator par = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();

	if (!undefined())
		sgml::openTag(os, getLayout().latexname(),
			      par->getID(buffer(), runparams) + getLayout().latexparam());

	for (; par != end; ++par) {
		par->simpleDocBookOnePar(buffer(), os, runparams,
					 outerFont(distance(beg, par),
						   paragraphs()));
	}

	if (!undefined())
		sgml::closeTag(os, getLayout().latexname());

	return 0;
}


void InsetFlex::tocString(odocstream & os) const
{
	os << text().asString(0, 1, AS_STR_LABEL | AS_STR_INSETS);
}


} // namespace lyx
