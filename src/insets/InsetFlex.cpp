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
#include "BufferView.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "Cursor.h"
#include "support/gettext.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "Text.h"
#include "MetricsInfo.h"
#include "Paragraph.h"
#include "paragraph_funcs.h"
#include "sgml.h"

#include "support/convert.h"

#include <sstream>

using namespace std;

namespace lyx {


InsetFlex::InsetFlex(Buffer const & buf,
	DocumentClass * dc, string const & layoutName)
	: InsetCollapsable(buf, Collapsed, dc),
	name_(layoutName)
{
	setLayout(dc); // again, because now the name is initialized
	packages_ = getLayout().requires();
	preamble_ = getLayout().preamble();
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


void InsetFlex::textString(odocstream & os) const
{
	os << paragraphs().begin()->asString(true);
}


void InsetFlex::validate(LaTeXFeatures & features) const
{
	if (!preamble_.empty())
		features.addPreambleSnippet(preamble_);
	features.require(packages_);
}

} // namespace lyx
