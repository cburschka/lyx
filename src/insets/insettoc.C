#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insettoc.h"
#include "buffer.h"
#include "bufferlist.h"
#include "commandtags.h"
#include "lyxfunc.h"
#include "LyXView.h"
#include "BufferView.h"

void InsetTOC::Edit(BufferView * bv, int, int, unsigned int)
{
	bv->owner()->getLyXFunc()->Dispatch(LFUN_TOCVIEW);
}

#ifndef USE_OSTREAM_ONLY
int InsetTOC::Linuxdoc(string & file) const
{
	file += "<toc>";
	return 0;
}


int InsetTOC::DocBook(string & file) const
{
	file += "<toc></toc>";
	return 0;
}

#else

int InsetTOC::Linuxdoc(ostream & os) const
{
	os << "<toc>";
	return 0;
}


int InsetTOC::DocBook(ostream & os) const
{
	os << "<toc></toc>";
	return 0;
}
#endif
