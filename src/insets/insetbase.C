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

