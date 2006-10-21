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


namespace lyx {

using support::escape;

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
	list.push_back(getParam("name"));
}


docstring const InsetLabel::getScreenLabel(Buffer const &) const
{
	return getParam("name");
}


void InsetLabel::doDispatch(LCursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p("label");
		InsetCommandMailer::string2params("label", to_utf8(cmd.argument()), p);
		if (p.getCmdName().empty()) {
			cur.noUpdate();
			break;
		}
		if (p["name"] != params()["name"])
        		// FIXME UNICODE
			cur.bv().buffer()->changeRefsIfUnique(to_utf8(params()["name"]),
						       to_utf8(p["name"]), InsetBase::REF_CODE);
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
	os << '<' << getParam("name") << '>';
	return 0;
}


int InsetLabel::docbook(Buffer const & buf, odocstream & os,
			OutputParams const & runparams) const
{
        // FIXME UNICODE
	os << "<!-- anchor id=\""
           << from_ascii(sgml::cleanID(buf, runparams, lyx::to_ascii(getParam("name"))))
           << "\" -->";
	return 0;
}


} // namespace lyx
