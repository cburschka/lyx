#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "gettext.h"
#include "insettoc.h"
#include "BufferView.h"
#include "frontends/LyXView.h"
#include "frontends/Dialogs.h"
#include "debug.h"
#include "toc.h"

using std::vector;
using std::ostream;


string const InsetTOC::getScreenLabel(Buffer const *) const
{
	string const cmdname(getCmdName());

	if (cmdname == "tableofcontents")
		return _("Table of Contents");
	return _("Unknown toc list");
}


Inset::Code InsetTOC::lyxCode() const
{
	string const cmdname(getCmdName());
	if (cmdname == "tableofcontents")
		return Inset::TOC_CODE;
	return Inset::NO_CODE;
}


void InsetTOC::edit(BufferView * bv, int, int, mouse_button::state)
{
	bv->owner()->getDialogs().showTOC(this);
}


void InsetTOC::edit(BufferView * bv, bool)
{
	edit(bv, 0, 0, mouse_button::none);
}


int InsetTOC::ascii(Buffer const * buffer, ostream & os, int) const
{
	os << getScreenLabel(buffer) << "\n\n";

	toc::asciiTocList(toc::getType(getCmdName()), buffer, os);

	os << "\n";
	return 0;
}


int InsetTOC::linuxdoc(Buffer const *, ostream & os) const
{
	if (getCmdName() == "tableofcontents")
		os << "<toc>";
	return 0;
}


int InsetTOC::docbook(Buffer const *, ostream & os, bool) const
{
	if (getCmdName() == "tableofcontents")
		os << "<toc></toc>";
	return 0;
}
