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

void InsetTOC::Edit(BufferView * bv, int, int)
{
	bv->owner()->getLyXFunc()->Dispatch(LFUN_TOCVIEW);
}

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
