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

#include "buffer.h"
#include "BufferView.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "InsetList.h"
#include "lyxtext.h"
#include "paragraph.h"
#include "pariterator.h"
#include "sgml.h"

#include "support/lstrings.h"
#include "support/lyxalgo.h"
#include "support/std_ostream.h"

using lyx::docstring;
using lyx::odocstream;
using lyx::support::escape;

using std::string;
using std::ostream;
using std::vector;


InsetLabel::InsetLabel(InsetCommandParams const & p)
	: InsetCommand(p, "label")
{}


std::auto_ptr<InsetBase> InsetLabel::doClone() const
{
	return std::auto_ptr<InsetBase>(new InsetLabel(params()));
}


void InsetLabel::getLabelList(Buffer const &, std::vector<docstring> & list) const
{
	// FIXME UNICODE
	list.push_back(lyx::from_utf8(getContents()));
}


docstring const InsetLabel::getScreenLabel(Buffer const &) const
{
	// FIXME UNICODE
	return lyx::from_utf8(getContents());
}


void InsetLabel::doDispatch(LCursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p("label");
		InsetCommandMailer::string2params("label", lyx::to_utf8(cmd.argument()), p);
		if (p.getCmdName().empty()) {
			cur.noUpdate();
			break;
		}
		if (p.getContents() != params().getContents())
			cur.bv().buffer()->changeRefsIfUnique(params().getContents(),
						       p.getContents(), InsetBase::REF_CODE);
		setParams(p);
		break;
	}

	default:
		InsetCommand::doDispatch(cur, cmd);
		break;
	}
}


int InsetLabel::latex(Buffer const &, odocstream & os,
		      OutputParams const &) const
{
	os << escape(getCommand());
	return 0;
}


int InsetLabel::plaintext(Buffer const &, odocstream & os,
		      OutputParams const &) const
{
	// FIXME UNICODE
	os << '<' << lyx::from_utf8(getContents()) << '>';
	return 0;
}


int InsetLabel::docbook(Buffer const & buf, ostream & os,
			OutputParams const & runparams) const
{
	os << "<!-- anchor id=\"" << sgml::cleanID(buf, runparams, getContents()) << "\" -->";
	return 0;
}
