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

using std::ostream;
using std::vector;
using std::pair;


InsetLabel::InsetLabel(InsetCommandParams const & p, bool)
	: InsetCommand(p)
{}


InsetLabel::~InsetLabel()
{
	InsetCommandMailer mailer("label", *this);
	mailer.hideDialog();
}


vector<string> const InsetLabel::getLabelList() const
{
	return vector<string>(1, getContents());
}


void InsetLabel::edit(BufferView * bv, int, int, mouse_button::state)
{
	InsetCommandMailer mailer("label", *this);
	mailer.showDialog(bv);
}


dispatch_result InsetLabel::localDispatch(FuncRequest const & cmd)
{
	Inset::RESULT result = UNDISPATCHED;

	switch (cmd.action) {
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
		cmd.view()->updateInset(this, !clean);
		result = DISPATCHED;
	}
	break;

	default:
		result = InsetCommand::localDispatch(cmd);
	}

	return result;
}


void InsetLabel::edit(BufferView * bv, bool)
{
	edit(bv, 0, 0, mouse_button::none);
}


int InsetLabel::latex(Buffer const *, ostream & os,
		      bool /*fragile*/, bool /*fs*/) const
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
