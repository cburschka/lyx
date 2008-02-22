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


InsetFlex::InsetFlex(BufferParams const & bp,
	TextClassPtr tc, string const & layoutName)
	: InsetCollapsable(bp, Collapsed, tc),
	name_(layoutName)
{
	setLayout(tc); // again, because now the name is initialized
	packages_ = getLayout().requires();
	preamble_ = getLayout().preamble();
}


InsetFlex::InsetFlex(InsetFlex const & in)
	: InsetCollapsable(in), name_(in.name_)
{}


Inset * InsetFlex::clone() const
{
	return new InsetFlex(*this);
}


docstring const InsetFlex::editMessage() const
{
	return _("Opened Flex Inset");
}


void InsetFlex::write(Buffer const & buf, ostream & os) const
{
	os << "Flex " <<
		(name_.empty() ? "undefined" : name_) << "\n";
	InsetCollapsable::write(buf, os);
}


void InsetFlex::read(Buffer const & buf, Lexer & lex)
{
	while (lex.isOK()) {
		lex.next();
		string token = lex.getString();

		if (token == "Flex") {
			lex.next();
			name_ = lex.getString();
		}

		// This is handled in Collapsable
		else if (token == "status") {
			lex.pushToken(token);
			break;
		}
	}
	InsetCollapsable::read(buf, lex);
}


int InsetFlex::plaintext(Buffer const & buf, odocstream & os,
			      OutputParams const & runparams) const
{
	return InsetText::plaintext(buf, os, runparams);
}


int InsetFlex::docbook(Buffer const & buf, odocstream & os,
			    OutputParams const & runparams) const
{
	ParagraphList::const_iterator beg = paragraphs().begin();
	ParagraphList::const_iterator par = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();

	if (!undefined())
		sgml::openTag(os, getLayout().latexname(),
			      par->getID(buf, runparams) + getLayout().latexparam());

	for (; par != end; ++par) {
		par->simpleDocBookOnePar(buf, os, runparams,
					 outerFont(distance(beg, par),
						   paragraphs()));
	}

	if (!undefined())
		sgml::closeTag(os, getLayout().latexname());

	return 0;
}


void InsetFlex::textString(Buffer const & buf, odocstream & os) const
{
	os << paragraphs().begin()->asString(buf, true);
}


void InsetFlex::validate(LaTeXFeatures & features) const
{
	if (!preamble_.empty())
		features.addPreambleSnippet(preamble_);
	features.require(packages_);
}

} // namespace lyx
