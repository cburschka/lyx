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
#include "debug.h"

#include "frontends/Dialogs.h"
#include "frontends/LyXView.h"

using std::string;


void MailInset::showDialog(BufferView * bv) const
{
	BOOST_ASSERT(bv);
	bv->owner()->getDialogs().show(name(), inset2string(*bv->buffer()),
				       &inset());
}


void MailInset::updateDialog(BufferView * bv) const
{
	BOOST_ASSERT(bv);
	if (bv->owner()->getDialogs().visible(name()))
		bv->owner()->getDialogs().update(name(),
						 inset2string(*bv->buffer()));
}


void MailInset::hideDialog() const
{
	Dialogs::hide(name(), &inset());
}


void print_mailer_error(string const & class_name,
			string const & data, int arg_id, string const & arg)
{
	lyxerr << '\n' << class_name << "::string2params(" << data << ")\n"
	       << "Expected arg " << arg_id << " to be \"" << arg << "\"\n"
	       << std::endl;
}
