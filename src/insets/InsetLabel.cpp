/**
 * \file InsetLabel.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetLabel.h"

#include "Buffer.h"
#include "BufferView.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "Text.h"
#include "sgml.h"

#include "support/lstrings.h"
#include "support/lyxalgo.h"

using namespace std;

namespace lyx {


InsetLabel::InsetLabel(InsetCommandParams const & p)
	: InsetCommand(p, "label")
{}


CommandInfo const * InsetLabel::findInfo(string const & /* cmdName */)
{
	static const char * const paramnames[] = {"name", ""};
	static const bool isoptional[] = {false};
	static const CommandInfo info = {1, paramnames, isoptional};
	return &info;
}


Inset * InsetLabel::clone() const
{
	return new InsetLabel(params());
}


void InsetLabel::getLabelList(Buffer const &, vector<docstring> & list) const
{
	list.push_back(getParam("name"));
}


docstring const InsetLabel::getScreenLabel(Buffer const &) const
{
	return getParam("name");
}


void InsetLabel::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p(LABEL_CODE);
		// FIXME UNICODE
		InsetCommandMailer::string2params("label", to_utf8(cmd.argument()), p);
		if (p.getCmdName().empty()) {
			cur.noUpdate();
			break;
		}
		if (p["name"] != params()["name"])
			cur.bv().buffer().changeRefsIfUnique(params()["name"],
					p["name"], REF_CODE);
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
	os << support::escape(getCommand());
	return 0;
}


int InsetLabel::plaintext(Buffer const &, odocstream & os,
			  OutputParams const &) const
{
	docstring const str = getParam("name");
	os << '<' << str << '>';
	return 2 + str.size();
}


int InsetLabel::docbook(Buffer const & buf, odocstream & os,
			OutputParams const & runparams) const
{
	os << "<!-- anchor id=\""
	   << sgml::cleanID(buf, runparams, getParam("name"))
	   << "\" -->";
	return 0;
}


} // namespace lyx
