/**
 * \file insetlabel.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "insetlabel.h"
#include "buffer.h"
#include "BufferView.h"
#include "funcrequest.h"
#include "gettext.h"
#include "lyxtext.h"

#include "support/lstrings.h"
#include "support/LOstream.h"
#include "support/lstrings.h" //frontStrip, strip

using namespace lyx::support;

using std::ostream;
using std::vector;
using std::pair;


InsetLabel::InsetLabel(InsetCommandParams const & p)
	: InsetCommand(p)
{}


// InsetLabel::InsetLabel(InsetCommandParams const & p, bool)
//	: InsetCommand(p, false)
// {}


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
	InsetOld::RESULT result = UNDISPATCHED;

	switch (cmd.action) {

	case LFUN_INSET_EDIT:
		InsetCommandMailer("label", *this).showDialog(cmd.view());
		result = DISPATCHED;
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
		result = DISPATCHED;
	}
	break;

	default:
		result = InsetCommand::localDispatch(cmd);
	}

	return result;
}


int InsetLabel::latex(Buffer const *, ostream & os,
		      LatexRunParams const &) const
{
	os << escape(getCommand());
	return 0;
}


int InsetLabel::ascii(Buffer const *, ostream & os, int) const
{
	os << '<' << getContents()  << '>';
	return 0;
}


int InsetLabel::linuxdoc(Buffer const *, ostream & os) const
{
	os << "<label id=\"" << getContents() << "\" >";
	return 0;
}


int InsetLabel::docbook(Buffer const *, ostream & os, bool) const
{
	os << "<anchor id=\"" << getContents() << "\">";
	return 0;
}
