// -*- C++ -*-
/**
 * \file mailinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "mailinset.h"
#include "inset.h"
#include "BufferView.h"
#include "frontends/LyXView.h"
#include "frontends/Dialogs.h"
#include "support/LAssert.h"
#include "Lsstream.h"


void MailInset::showDialog(BufferView * bv) const
{
	lyx::Assert(bv);
	bv->owner()->getDialogs().show(name(), inset2string(), &inset());
}


void MailInset::updateDialog(BufferView * bv) const
{
	lyx::Assert(bv);
	bv->owner()->getDialogs().update(name(), inset2string());
}


void MailInset::hideDialog() const
{
	Dialogs::hide()(name(), &inset());
}
