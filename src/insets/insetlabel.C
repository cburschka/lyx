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
#include "dispatchresult.h"
#include "funcrequest.h"

#include "frontends/LyXView.h"

#include "support/lstrings.h"

#include "support/std_ostream.h"

using lyx::support::escape;

using std::string;
using std::ostream;


InsetLabel::InsetLabel(InsetCommandParams const & p)
	: InsetCommand(p, "label")
{}


std::auto_ptr<InsetBase> InsetLabel::clone() const
{
	return std::auto_ptr<InsetBase>(new InsetLabel(params()));
}


void InsetLabel::getLabelList(Buffer const &, std::vector<string> & list) const
{
	list.push_back(getContents());
}


string const InsetLabel::getScreenLabel(Buffer const &) const
{
	return getContents();
}


DispatchResult
InsetLabel::priv_dispatch(LCursor & cur, FuncRequest const & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p;
		InsetCommandMailer::string2params("label", cmd.argument, p);
		if (p.getCmdName().empty())
			return DispatchResult(false);
		bool clean = true;
		if (p.getContents() != params().getContents())
			clean = cur.bv().ChangeRefsIfUnique(params().getContents(),
						       p.getContents());
		setParams(p);
		cur.bv().update();
		return DispatchResult(true, true);
	}

	default:
		return InsetCommand::priv_dispatch(cur, cmd);
	}
}


int InsetLabel::latex(Buffer const &, ostream & os,
		      OutputParams const &) const
{
	os << escape(getCommand());
	return 0;
}


int InsetLabel::plaintext(Buffer const &, ostream & os,
		      OutputParams const &) const
{
	os << '<' << getContents()  << '>';
	return 0;
}


int InsetLabel::linuxdoc(Buffer const &, ostream & os,
			 OutputParams const &) const
{
	os << "<label id=\"" << getContents() << "\" >";
	return 0;
}


int InsetLabel::docbook(Buffer const &, ostream & os,
			OutputParams const &) const
{
	os << "<anchor id=\"" << getContents() << "\">";
	return 0;
}
