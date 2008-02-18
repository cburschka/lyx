/**
 * \file MailInset.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "MailInset.h"

#include "BufferView.h"

#include "frontends/Application.h"

#include "support/debug.h"

#include <boost/assert.hpp>

using namespace std;

namespace lyx {


void MailInset::showDialog(BufferView * bv) const
{
	BOOST_ASSERT(bv);
	bv->showDialog(name(), inset2string(bv->buffer()), &inset());
}


void MailInset::updateDialog(BufferView * bv) const
{
	BOOST_ASSERT(bv);
	bv->updateDialog(name(), inset2string(bv->buffer()));
}


void MailInset::hideDialog() const
{
	if (theApp())
		theApp()->hideDialogs(name(), &inset());
}


void print_mailer_error(string const & class_name,
			string const & data, int arg_id, string const & arg)
{
	LYXERR0('\n' << class_name << "::string2params(" << data << ")\n"
	       << "Expected arg " << arg_id << " to be \"" << arg << "\"\n");
}


} // namespace lyx
