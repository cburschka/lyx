#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetlof.h"
#include "BufferView.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "commandtags.h"

string InsetLOF::getScreenLabel() const 
{
	return _("List of Figures");
}

void InsetLOF::Edit(BufferView * bv, int, int, unsigned int)
{
	bv->owner()->getLyXFunc()->Dispatch(LFUN_LOFVIEW);
}
