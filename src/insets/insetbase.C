/**
 * \file insetbase.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetbase.h"
#include "debug.h"
#include "dispatchresult.h"


DispatchResult InsetBase::dispatch(LCursor & cur, FuncRequest const & cmd)
{
	return priv_dispatch(cur, cmd);
}


DispatchResult InsetBase::priv_dispatch(LCursor &, FuncRequest const & cmd)
{
	return DispatchResult(false);
}


void InsetBase::edit(LCursor &, bool)
{
	lyxerr << "InsetBase: edit left/right" << std::endl;
}


void InsetBase::edit(LCursor & cur, int, int)
{
	lyxerr << "InsetBase: edit xy" << std::endl;
	edit(cur, true);
}
