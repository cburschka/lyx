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


dispatch_result InsetBase::dispatch(FuncRequest const &, idx_type &, pos_type &)
{
	return UNDISPATCHED;
}


dispatch_result InsetBase::localDispatch(FuncRequest const & cmd)
{
	idx_type idx = 0;
	pos_type pos = 0;
	return dispatch(cmd, idx, pos);
}
