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
	: InsetCommand(p)
{}


InsetLabel::~InsetLabel()
{
	InsetCommandMailer("label", *this).hideDialog();
}


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
InsetLabel::priv_dispatch(FuncRequest const & cmd,
			  idx_type & idx, pos_type & pos)
{
	BOOST_ASSERT(cmd.view());
	BufferView * const bv = cmd.view();

	switch (cmd.action) {

	case LFUN_INSET_EDIT:
		InsetCommandMailer("label", *this).showDialog(bv);
		return DispatchResult(true);
		break;

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p;
		InsetCommandMailer::string2params(cmd.argument, p);
		if (p.getCmdName().empty())
			return DispatchResult(false);

		bool clean = true;
		if (bv && p.getContents() != params().getContents()) {
			clean = bv->ChangeRefsIfUnique(params().getContents(),
						       p.getContents());
		}

		setParams(p);
		bv->updateInset(this);
		return DispatchResult(true);
	}

	default:
		return InsetCommand::priv_dispatch(cmd, idx, pos);
	}
}


int InsetLabel::latex(Buffer const &, ostream & os,
		      LatexRunParams const &) const
{
	os << escape(getCommand());
	return 0;
}


int InsetLabel::ascii(Buffer const &, ostream & os,
		      LatexRunParams const &) const
{
	os << '<' << getContents()  << '>';
	return 0;
}


int InsetLabel::linuxdoc(Buffer const &, ostream & os,
			 LatexRunParams const &) const
{
	os << "<label id=\"" << getContents() << "\" >";
	return 0;
}


int InsetLabel::docbook(Buffer const &, ostream & os,
			LatexRunParams const &) const
{
	os << "<anchor id=\"" << getContents() << "\">";
	return 0;
}
