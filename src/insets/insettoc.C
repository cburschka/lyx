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
