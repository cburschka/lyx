/**
 * \file mailinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "mailinset.h"

#include "BufferView.h"

#include "frontends/Dialogs.h"
#include "frontends/LyXView.h"

#include "support/LAssert.h"

using namespace lyx::support;


void MailInset::showDialog(BufferView * bv) const
{
	Assert(bv);
	bv->owner()->getDialogs().show(name(), inset2string(*bv->buffer()),
				       &inset());
}


void MailInset::updateDialog(BufferView * bv) const
{
	Assert(bv);
	if(bv->owner()->getDialogs().visible(name()))
		bv->owner()->getDialogs().update(name(),
						 inset2string(*bv->buffer()));
}


void MailInset::hideDialog() const
{
	Dialogs::hide(name(), &inset());
}
