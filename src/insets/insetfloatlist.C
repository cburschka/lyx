#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetfloatlist.h"
#include "FloatList.h"
#include "buffer.h"
#include "gettext.h"
#include "debug.h"

using std::endl;

string const InsetFloatList::getScreenLabel() const 
{
	string const guiName = floatList[float_type]->second.name();
	if (!guiName.empty()) {
		string const res = _("List of ") + guiName;
		return res;
	}
	return _("ERROR nonexistant float type!");
}


Inset::Code InsetFloatList::LyxCode() const
{
	return Inset::FLOAT_LIST_CODE;
}


void InsetFloatList::Write(Buffer const *, std::ostream & os) const
{
	os << "FloatList " << float_type << "\n";
}


void InsetFloatList::Read(Buffer const *, LyXLex & lex) 
{
	string token;

	if (lex.EatLine()) {
		float_type = lex.GetString();
		lyxerr << "FloatList::float_type: " << float_type << endl;
	} else
		lex.printError("InsetFloatList: Parse error: `$$Token'");
	while (lex.IsOK()) {
		lex.nextToken();
		token = lex.GetString();
		if (token == "\\end_inset")
			break;
	}
	if (token != "\\end_inset") {
		lex.printError("Missing \\end_inset at this point. "
			       "Read: `$$Token'");
	}
}


void InsetFloatList::Edit(BufferView *, int, int, unsigned int)
{
#ifdef WITH_WARNINGS
#warning Implement me please.
#endif
#if 0
	bv->owner()->getDialogs()->showFloatList(this);
#endif
}


int InsetFloatList::Latex(Buffer const *, std::ostream & os, bool, bool) const
{
	FloatList::const_iterator cit = floatList[float_type];

	
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
			os << "\\listof{" << float_type << "}{"
			   << _("List of ") << cit->second.name() << "}\n";
		}
	} else {
		os << "%%\\listof{" << float_type << "}{"
		   << _("List of ") << cit->second.name() << "}\n";
	}
	return 1;
}


int InsetFloatList::Ascii(Buffer const * buffer, std::ostream & os, int) const
{
	os << getScreenLabel() << "\n\n";

	Buffer::Lists const toc_list = buffer->getLists();
	Buffer::Lists::const_iterator cit =
		toc_list.find(float_type);
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
