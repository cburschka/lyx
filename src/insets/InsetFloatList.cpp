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
#include "debug.h"
#include "DispatchResult.h"
#include "Floating.h"
#include "FloatList.h"
#include "FuncRequest.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "TocBackend.h"
#include "TextClass.h"

#include "support/lstrings.h"


namespace lyx {

using support::bformat;

using std::endl;
using std::string;
using std::ostream;


InsetFloatList::InsetFloatList()
	: InsetCommand(InsetCommandParams(FLOAT_LIST_CODE), "toc")
{}


InsetFloatList::InsetFloatList(string const & type)
	: InsetCommand(InsetCommandParams(FLOAT_LIST_CODE), "toc")
{
	setParam("type", from_ascii(type));
}


CommandInfo const * InsetFloatList::findInfo(std::string const & /* cmdName */)
{
	static const char * const paramnames[] = {"type", ""};
	static const bool isoptional[] = {false};
	static const CommandInfo info = {1, paramnames, isoptional};
	return &info;
}


//HACK
bool InsetFloatList::isCompatibleCommand(std::string const & s) {
	std::string str = s.substr(0, 6);
	return str == "listof";
}


docstring const InsetFloatList::getScreenLabel(Buffer const & buf) const
{
	FloatList const & floats = buf.params().getTextClass().floats();
	FloatList::const_iterator it = floats[to_ascii(getParam("type"))];
	if (it != floats.end())
		return buf.B_(it->second.listName());
	else
		return _("ERROR: Nonexistent float type!");
}


void InsetFloatList::write(Buffer const &, ostream & os) const
{
	os << "FloatList " << to_ascii(getParam("type")) << "\n";
}


void InsetFloatList::read(Buffer const & buf, Lexer & lex)
{
	FloatList const & floats = buf.params().getTextClass().floats();
	string token;

	if (lex.eatLine()) {
		setParam("type", lex.getDocString());
		LYXERR(Debug::INSETS) << "FloatList::float_type: "
				      << to_ascii(getParam("type")) << endl;
		if (!floats.typeExist(to_ascii(getParam("type"))))
			lex.printError("InsetFloatList: Unknown float type: `$$Token'");
	} else
		lex.printError("InsetFloatList: Parse error: `$$Token'");
	while (lex.isOK()) {
		lex.next();
		token = lex.getString();
		if (token == "\\end_inset")
			break;
	}
	if (token != "\\end_inset") {
		lex.printError("Missing \\end_inset at this point. "
			       "Read: `$$Token'");
	}
}


int InsetFloatList::latex(Buffer const & buf, odocstream & os,
			  OutputParams const &) const
{
	FloatList const & floats = buf.params().getTextClass().floats();
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
			   << buf.B_(cit->second.listName()) << "}\n";
		}
	} else {
		os << "%%\\listof{" << getParam("type") << "}{"
		   << bformat(_("List of %1$s"), from_utf8(cit->second.name()))
		   << "}\n";
	}
	return 1;
}


int InsetFloatList::plaintext(Buffer const & buffer, odocstream & os,
			      OutputParams const &) const
{
	os << getScreenLabel(buffer) << "\n\n";

	buffer.tocBackend().writePlaintextTocList(to_ascii(getParam("type")), os);

	return PLAINTEXT_NEWLINE;
}


void InsetFloatList::validate(LaTeXFeatures & features) const
{
	features.useFloat(to_ascii(getParam("type")));
}


} // namespace lyx
