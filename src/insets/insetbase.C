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


dispatch_result
InsetBase::dispatch(FuncRequest const & f, idx_type & i, pos_type & p)
{
	return priv_dispatch(f, i, p);
}

dispatch_result
InsetBase::dispatch(FuncRequest const & f)
{
	idx_type i = 0;
	pos_type p = 0;
	return priv_dispatch(f, i, p);
}


dispatch_result
InsetBase::priv_dispatch(FuncRequest const &, idx_type &, pos_type &)
{
	return UNDISPATCHED;
}
