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


namespace lyx {

using support::bformat;

using std::endl;
using std::string;
using std::ostream;


InsetFloatList::InsetFloatList()
	: InsetCommand(InsetCommandParams("floatlist"), "toc")
{}


InsetFloatList::InsetFloatList(string const & type)
	: InsetCommand(InsetCommandParams("floatlist"), "toc")
{
	setCmdName(type);
}


docstring const InsetFloatList::getScreenLabel(Buffer const & buf) const
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
	InsetCommand::read(buf, lex);
	lyxerr[Debug::INSETS] << "FloatList::float_type: " << getCmdName() << endl;
	if (!buf.params().getLyXTextClass().floats().typeExist(getCmdName()))
		lex.printError("InsetFloatList: Unknown float type: `$$Token'");
}


int InsetFloatList::latex(Buffer const & buf, odocstream & os,
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
			// FIXME UNICODE
			os << "\\listof{" << from_ascii(getCmdName()) << "}{"
			   << buf.B_(cit->second.listName()) << "}\n";
		}
	} else {
		// FIXME UNICODE
		os << "%%\\listof{" << from_ascii(getCmdName()) << "}{"
		   << bformat(_("List of %1$s"), from_utf8(cit->second.name()))
		   << "}\n";
	}
	return 1;
}


int InsetFloatList::plaintext(Buffer const & buffer, odocstream & os,
                              OutputParams const &) const
{
	os << getScreenLabel(buffer) << "\n\n";

	toc::asciiTocList(getCmdName(), buffer, os);

	os << "\n";
	return 0;
}


void InsetFloatList::validate(LaTeXFeatures & features) const
{
	features.useFloat(getCmdName());
}


} // namespace lyx
