#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "gettext.h"
#include "insettoc.h"
#include "BufferView.h"
#include "LyXView.h"
#include "frontends/Dialogs.h"
#include "debug.h"
#include "buffer.h"

using std::vector;

string const InsetTOC::getScreenLabel() const 
{
	string const cmdname(getCmdName());
	
	if (cmdname == "tableofcontents")
		return _("Table of Contents");
	return _("Unknown toc list");
}


Inset::Code InsetTOC::LyxCode() const
{
	string const cmdname(getCmdName());
	if (cmdname == "tableofcontents")
		return Inset::TOC_CODE;
	return Inset::NO_CODE;
}


void InsetTOC::Edit(BufferView * bv, int, int, unsigned int)
{
	bv->owner()->getDialogs()->showTOC(this);
}


int InsetTOC::Ascii(Buffer const * buffer, std::ostream & os, int) const
{
	os << getScreenLabel() << "\n\n";

	string type;
	string const cmdname = getCmdName();
	if (cmdname == "tableofcontents")
		type = "TOC";
	Buffer::Lists const toc_list = buffer->getLists();
	Buffer::Lists::const_iterator cit =
		toc_list.find(type);
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


int InsetTOC::Linuxdoc(Buffer const *, std::ostream & os) const
{
	if (getCmdName() == "tableofcontents")
		os << "<toc>";
	return 0;
}


int InsetTOC::DocBook(Buffer const *, std::ostream & os) const
{
	if (getCmdName() == "tableofcontents")
		os << "<toc></toc>";
	return 0;
}
