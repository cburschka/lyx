/**
 * \file insetlabel.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetlabel.h"

#include "BufferView.h"
#include "funcrequest.h"

#include "support/lstrings.h"

using lyx::support::escape;

using std::ostream;


InsetLabel::InsetLabel(InsetCommandParams const & p)
	: InsetCommand(p)
{}


InsetLabel::~InsetLabel()
{
	InsetCommandMailer("label", *this).hideDialog();
}


void InsetLabel::getLabelList(std::vector<string> & list) const
{
	list.push_back(getContents());
}


dispatch_result InsetLabel::localDispatch(FuncRequest const & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_EDIT:
		InsetCommandMailer("label", *this).showDialog(cmd.view());
		return DISPATCHED;
		break;

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p;
		InsetCommandMailer::string2params(cmd.argument, p);
		if (p.getCmdName().empty())
			return UNDISPATCHED;

		bool clean = true;
		if (view() && p.getContents() != params().getContents()) {
			clean = view()->ChangeRefsIfUnique(params().getContents(),
							   p.getContents());
		}

		setParams(p);
		cmd.view()->updateInset(this);
		return DISPATCHED;
	}

	default:
		return InsetCommand::localDispatch(cmd);
	}
}


int InsetLabel::latex(Buffer const &, ostream & os,
		      LatexRunParams const &) const
{
	os << escape(getCommand());
	return 0;
}


int InsetLabel::ascii(Buffer const &, ostream & os, int) const
{
	os << '<' << getContents()  << '>';
	return 0;
}


int InsetLabel::linuxdoc(Buffer const &, ostream & os) const
{
	os << "<label id=\"" << getContents() << "\" >";
	return 0;
}


int InsetLabel::docbook(Buffer const &, ostream & os, bool) const
{
	os << "<anchor id=\"" << getContents() << "\">";
	return 0;
}
