#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetfloatlist.h"
#include "FloatList.h"
#include "LaTeXFeatures.h"
#include "lyxlex.h"
#include "frontends/Dialogs.h"
#include "frontends/LyXView.h"
#include "BufferView.h"
#include "buffer.h"
#include "toc.h"
#include "gettext.h"
#include "debug.h"


using std::ostream;
using std::endl;


InsetFloatList::InsetFloatList()
	: InsetCommand(InsetCommandParams())
{
}


InsetFloatList::InsetFloatList(string const & type)
	: InsetCommand(InsetCommandParams())
{
	setCmdName(type);
}


string const InsetFloatList::getScreenLabel(Buffer const * buf) const
{
	FloatList const & floats = buf->params.getLyXTextClass().floats();
	FloatList::const_iterator it = floats[getCmdName()];
	if (it != floats.end())
		return _(it->second.listName());
	else
		return _("ERROR: Nonexistent float type!");
}


Inset::Code InsetFloatList::lyxCode() const
{
	return Inset::FLOAT_LIST_CODE;
}


void InsetFloatList::write(Buffer const *, ostream & os) const
{
	os << "FloatList " << getCmdName() << "\n";
}


void InsetFloatList::read(Buffer const * buf, LyXLex & lex)
{
	FloatList const & floats = buf->params.getLyXTextClass().floats();
	string token;

	if (lex.eatLine()) {
		setCmdName(lex.getString());
		lyxerr[Debug::INSETS] << "FloatList::float_type: " << getCmdName() << endl;
		if (!floats.typeExist(getCmdName()))
			lex.printError("InsetFloatList: Unknown float type: `$$Token'");
	} else
		lex.printError("InsetFloatList: Parse error: `$$Token'");
	while (lex.isOK()) {
		lex.nextToken();
		token = lex.getString();
		if (token == "\\end_inset")
			break;
	}
	if (token != "\\end_inset") {
		lex.printError("Missing \\end_inset at this point. "
			       "Read: `$$Token'");
	}
}


void InsetFloatList::edit(BufferView * bv, int, int, mouse_button::state)
{
	bv->owner()->getDialogs().showTOC(this);
}


void InsetFloatList::edit(BufferView * bv, bool)
{
	edit(bv, 0, 0, mouse_button::none);
}


int InsetFloatList::latex(Buffer const * buf, ostream & os, bool, bool) const
{
	FloatList const & floats = buf->params.getLyXTextClass().floats();
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
			   << cit->second.listName() << "}\n";
		}
	} else {
		os << "%%\\listof{" << getCmdName() << "}{"
		   << _("List of ") << cit->second.name() << "}\n";
	}
	return 1;
}


int InsetFloatList::ascii(Buffer const * buffer, ostream & os, int) const
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
