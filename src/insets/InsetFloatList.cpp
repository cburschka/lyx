/**
 * \file InsetFloatList.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetFloatList.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "DispatchResult.h"
#include "Floating.h"
#include "FloatList.h"
#include "FuncRequest.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "TextClass.h"
#include "TocBackend.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include <ostream>

using namespace std;
using namespace lyx::support;

namespace lyx {


InsetFloatList::InsetFloatList()
	: InsetCommand(InsetCommandParams(FLOAT_LIST_CODE), "toc")
{}


InsetFloatList::InsetFloatList(string const & type)
	: InsetCommand(InsetCommandParams(FLOAT_LIST_CODE), "toc")
{
	setParam("type", from_ascii(type));
}


ParamInfo const & InsetFloatList::findInfo(string const & /* cmdName */)
{
	static ParamInfo param_info_;
	if (param_info_.empty()) {
		param_info_.add("type", ParamInfo::LATEX_REQUIRED);
	}
	return param_info_;
}


//HACK
bool InsetFloatList::isCompatibleCommand(string const & s)
{
	string str = s.substr(0, 6);
	return str == "listof";
}


docstring InsetFloatList::screenLabel() const
{
	FloatList const & floats = buffer().params().documentClass().floats();
	FloatList::const_iterator it = floats[to_ascii(getParam("type"))];
	if (it != floats.end())
		return buffer().B_(it->second.listName());
	else
		return _("ERROR: Nonexistent float type!");
}


void InsetFloatList::write(ostream & os) const
{
	os << "FloatList " << to_ascii(getParam("type")) << "\n";
}


void InsetFloatList::read(Lexer & lex)
{
	lex.setContext("InsetFloatList::read");
	FloatList const & floats = buffer().params().documentClass().floats();
	string token;

	if (lex.eatLine()) {
		setParam("type", lex.getDocString());
		LYXERR(Debug::INSETS, "FloatList::float_type: "
				      << to_ascii(getParam("type")));
		if (!floats.typeExist(to_ascii(getParam("type"))))
			lex.printError("Unknown float type");
	} else {
		lex.printError("Parse error");
	}

	while (lex.isOK()) {
		lex.next();
		token = lex.getString();
		if (token == "\\end_inset")
			break;
	}
	if (token != "\\end_inset") {
		lex.printError("Missing \\end_inset at this point.");
	}
}


int InsetFloatList::latex(odocstream & os, OutputParams const &) const
{
	FloatList const & floats = buffer().params().documentClass().floats();
	FloatList::const_iterator cit = floats[to_ascii(getParam("type"))];

	if (cit != floats.end()) {
		if (cit->second.builtin()) {
			// Only two different types allowed here:
			string const type = cit->second.type();
			if (type == "table") {
				os << "\\listoftables\n";
			} else if (type == "figure") {
				os << "\\listoffigures\n";
			} else {
				os << "%% unknown builtin float\n";
			}
		} else {
			os << "\\listof{" << getParam("type") << "}{"
			   << buffer().B_(cit->second.listName()) << "}\n";
		}
	} else {
		os << "%%\\listof{" << getParam("type") << "}{"
		   << bformat(_("List of %1$s"), from_utf8(cit->second.name()))
		   << "}\n";
	}
	return 1;
}


int InsetFloatList::plaintext(odocstream & os, OutputParams const &) const
{
	os << screenLabel() << "\n\n";

	buffer().tocBackend().writePlaintextTocList(to_ascii(getParam("type")), os);

	return PLAINTEXT_NEWLINE;
}


void InsetFloatList::validate(LaTeXFeatures & features) const
{
	features.useFloat(to_ascii(getParam("type")));
}


} // namespace lyx
