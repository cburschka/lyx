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
#include "debug.h"
#include "frontends/LyXView.h"
#include "frontends/Dialogs.h"
#include "Lsstream.h"


void MailInset::showDialog() const
{
	BufferView * bv = inset().view();
	if (!bv) {
		lyxerr << "MailInset::showDialog:\n"
		       << "The BufferView has not been cached!"
		       << std::endl;
		return;
	}

	bv->owner()->getDialogs().show(name(), inset2string(), &inset());
}


void MailInset::updateDialog() const
{
	BufferView * bv = inset().view();
	if (!bv) {
		lyxerr << "MailInset::showDialog:\n"
		       << "The BufferView has not been cached!"
		       << std::endl;
		return;
	}

	bv->owner()->getDialogs().update(name(), inset2string());
}


void MailInset::hideDialog() const
{
	BufferView * bv = inset().view();
	if (!bv) {
		lyxerr << "MailInset::showDialog:\n"
		       << "The BufferView has not been cached!"
		       << std::endl;
		return;
	}

	InsetBase * cmp = bv->owner()->getDialogs().getOpenInset(name());
	if (cmp == &inset())
		bv->owner()->getDialogs().hide(name());
}
