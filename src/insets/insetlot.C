#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetlot.h"
#include "BufferView.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "commandtags.h"


string InsetLOT::getScreenLabel() const 
{
	return _("List of Tables");
}

void InsetLOT::Edit(BufferView * bv, int, int, unsigned int)
{
	bv->owner()->getLyXFunc()->Dispatch(LFUN_LOTVIEW);
}
