#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "UpdateInset.h"
#include "BufferView.h"
//#include "insets/lyxinset.h"

void UpdateInset::update(BufferView * bv) 
{
	while (!insetqueue.empty()) {
		Inset * inset = insetqueue.front();
		insetqueue.pop();
		bv->updateInset(inset, false); // "false" because no document change
	}
}


