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
	else if (cmdname == "listof{algorithm}{List of Algorithms}")
		return _("List of Algorithms");
	else if (cmdname == "listoffigures")
		return _("List of Figures");
	else
		return _("List of Tables");
}


Inset::Code InsetTOC::LyxCode() const
{
	string const cmdname(getCmdName());
	if (cmdname == "tableofcontents")
		return Inset::TOC_CODE;
	else if (cmdname == "listof{algorithm}{List of Algorithms}")
		return Inset::LOA_CODE;
	else if (cmdname == "listoffigures")
		return Inset::LOF_CODE; 
	else
		return Inset::LOT_CODE;
}


void InsetTOC::Edit(BufferView * bv, int, int, unsigned int)
{
	bv->owner()->getDialogs()->showTOC(this);
}


int InsetTOC::Ascii(Buffer const * buffer, std::ostream & os, int) const
{
	os << getScreenLabel() << "\n\n";

#if 0
	Buffer::TocType type;
	string cmdname = getCmdName();
	if (cmdname == "tableofcontents")
		type = Buffer::TOC_TOC;
	else if (cmdname == "listof{algorithm}{List of Algorithms}")
		type = Buffer::TOC_LOA;
	else if (cmdname == "listoffigures")
		type = Buffer::TOC_LOF; 
	else
		type = Buffer::TOC_LOT;

	vector<vector<Buffer::TocItem> > const toc_list =
                buffer->getTocList();
	vector<Buffer::TocItem> const & toc = toc_list[type];
	for (vector<Buffer::TocItem>::const_iterator it = toc.begin();
	     it != toc.end(); ++it)
		os << string(4 * it->depth, ' ') << it->str << endl;
#else
#ifdef WITH_WARNINGS
#warning Fix Me! (Lgb)
#endif
	string type;
	string const cmdname = getCmdName();
	if (cmdname == "tableofcontents")
		type = "TOC";
	else if (cmdname == "listof{algorithm}{List of Algorithms}")
		type = "LOA";
	else if (cmdname == "listoffigures")
		type = "LOF";
	else 
		type = "LOT";

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
#endif
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
