/**
 * \file insetfloatlist.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetfloatlist.h"

#include "buffer.h"
#include "bufferparams.h"
#include "debug.h"
#include "dispatchresult.h"
#include "Floating.h"
#include "FloatList.h"
#include "funcrequest.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "lyxlex.h"
#include "metricsinfo.h"
#include "toc.h"

#include "support/lstrings.h"

using lyx::support::bformat;

using std::endl;
using std::string;
using std::ostream;


InsetFloatList::InsetFloatList()
	: InsetCommand(InsetCommandParams(), "toc")
{}


InsetFloatList::InsetFloatList(string const & type)
	: InsetCommand(InsetCommandParams(), "toc")
{
	setCmdName(type);
}


string const InsetFloatList::getScreenLabel(Buffer const & buf) const
{
	FloatList const & floats = buf.params().getLyXTextClass().floats();
	FloatList::const_iterator it = floats[getCmdName()];
	if (it != floats.end())
		return buf.B_(it->second.listName());
	else
		return _("ERROR: Nonexistent float type!");
}


InsetBase::Code InsetFloatList::lyxCode() const
{
	return InsetBase::FLOAT_LIST_CODE;
}


void InsetFloatList::write(Buffer const &, ostream & os) const
{
	os << "FloatList " << getCmdName() << "\n";
}


void InsetFloatList::read(Buffer const & buf, LyXLex & lex)
{
	FloatList const & floats = buf.params().getLyXTextClass().floats();
	string token;

	if (lex.eatLine()) {
		setCmdName(lex.getString());
		lyxerr[Debug::INSETS] << "FloatList::float_type: " << getCmdName() << endl;
		if (!floats.typeExist(getCmdName()))
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


int InsetFloatList::latex(Buffer const & buf, ostream & os,
			  OutputParams const &) const
{
	FloatList const & floats = buf.params().getLyXTextClass().floats();
	FloatList::const_iterator cit = floats[getCmdName()];

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
			os << "\\listof{" << getCmdName() << "}{"
			   << buf.B_(cit->second.listName()) << "}\n";
		}
	} else {
		os << "%%\\listof{" << getCmdName() << "}{"
		   << bformat(_("List of %1$s"), cit->second.name())
		   << "}\n";
	}
	return 1;
}


int InsetFloatList::plaintext(Buffer const & buffer, ostream & os, OutputParams const &) const
{
	os << getScreenLabel(buffer) << "\n\n";

	lyx::toc::asciiTocList(getCmdName(), buffer, os);

	os << "\n";
	return 0;
}


void InsetFloatList::validate(LaTeXFeatures & features) const
{
	features.useFloat(getCmdName());
}
