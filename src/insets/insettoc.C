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

extern BufferView * current_view;

void InsetTOC::Edit(int, int)
{
	current_view->owner()->getLyXFunc()->Dispatch(LFUN_TOCVIEW);
}

int InsetTOC::Linuxdoc(string & file)
{
	file += "<toc>";
	return 0;
}


int InsetTOC::DocBook(string & file)
{
	file += "<toc></toc>";
	return 0;
}
