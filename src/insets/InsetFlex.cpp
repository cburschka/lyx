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
#include "Lexer.h"
#include "Text.h"
#include "MetricsInfo.h"
#include "Paragraph.h"
#include "paragraph_funcs.h"
#include "sgml.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/convert.h"

#include <sstream>


namespace lyx {

using std::string;
using std::ostream;


InsetFlex::InsetFlex(BufferParams const & bp,
				InsetLayout const & il)
	: InsetCollapsable(bp, Collapsed, &il)
{
	name_ = il.name;
}


InsetFlex::InsetFlex(InsetFlex const & in)
	: InsetCollapsable(in), name_(in.name_)
{}


Inset * InsetFlex::clone() const
{
	return new InsetFlex(*this);
}


bool InsetFlex::undefined() const
{
	return layout_->labelstring == from_utf8("UNDEFINED");
}


docstring const InsetFlex::editMessage() const
{
	return _("Opened Flex Inset");
}


void InsetFlex::write(Buffer const & buf, ostream & os) const
{
	os << "Flex " << name_ << "\n";
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
		sgml::openTag(os, layout_->latexname,
			      par->getID(buf, runparams) + layout_->latexparam);

	for (; par != end; ++par) {
		par->simpleDocBookOnePar(buf, os, runparams,
					 outerFont(std::distance(beg, par),
						   paragraphs()));
	}

	if (!undefined())
		sgml::closeTag(os, layout_->latexname);

	return 0;
}


void InsetFlex::textString(Buffer const & buf, odocstream & os) const
{
	os << paragraphs().begin()->asString(buf, true);
}

} // namespace lyx
