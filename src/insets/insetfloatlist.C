#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetfloatlist.h"
#include "FloatList.h"
#include "frontends/Dialogs.h"
#include "LyXView.h"
#include "BufferView.h"
#include "buffer.h"
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


string const InsetFloatList::getScreenLabel(Buffer const *) const
{
	string const guiName = floatList[getCmdName()]->second.name();
	if (!guiName.empty()) {
		string const res = guiName + _(" List");
		return res;
	}
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


void InsetFloatList::read(Buffer const *, LyXLex & lex)
{
	string token;

	if (lex.eatLine()) {
		setCmdName(lex.getString());
		lyxerr << "FloatList::float_type: " << getCmdName() << endl;
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


void InsetFloatList::edit(BufferView * bv, int, int, unsigned int)
{
	bv->owner()->getDialogs()->showTOC(this);
}


void InsetFloatList::edit(BufferView * bv, bool)
{
	edit(bv, 0, 0, 0);
}


int InsetFloatList::latex(Buffer const *, ostream & os, bool, bool) const
{
	FloatList::const_iterator cit = floatList[getCmdName()];

	if (cit != floatList.end()) {
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
			   << _("List of ") << cit->second.name() << "}\n";
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

	Buffer::Lists const toc_list = buffer->getLists();
	Buffer::Lists::const_iterator cit =
		toc_list.find(getCmdName());
	if (cit != toc_list.end()) {
		Buffer::SingleList::const_iterator ccit = cit->second.begin();
		Buffer::SingleList::const_iterator end = cit->second.end();
		for (; ccit != end; ++ccit)
			os << string(4 * ccit->depth, ' ')
			   << ccit->str << "\n";
	}

	os << "\n";
	return 0;
}
