#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "gettext.h"
#include "insettoc.h"
#include "commandtags.h"
#include "debug.h"
#include "lyxfunc.h"
#include "LyXView.h"
#include "BufferView.h"

using std::ostream;


string InsetTOC::getScreenLabel() const 
{
	return _("Table of Contents");
}


void InsetTOC::Edit(BufferView * bv, int, int, unsigned int)
{
	bv->owner()->getLyXFunc()->Dispatch(LFUN_TOCVIEW);
}

int InsetTOC::Linuxdoc(Buffer const *, ostream & os) const
{
	os << "<toc>";
	return 0;
}


int InsetTOC::DocBook(Buffer const *, ostream & os) const
{
	os << "<toc></toc>";
	return 0;
}
