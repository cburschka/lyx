/**
 * \file ControlPreamble.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlPreamble.h"

#include "buffer.h"
#include "bufferparams.h"

using std::string;


ControlPreamble::ControlPreamble(Dialog & parent)
	: Dialog::Controller(parent)
{}


bool ControlPreamble::initialiseParams(std::string const &)
{
	params_ = kernel().buffer().params().preamble;
	return true;
}


void ControlPreamble::clearParams()
{
	params_.erase();
}


void ControlPreamble::dispatchParams()
{
	// This can stay because we're going to throw the class away
	// as soon as xforms 1.1 is released.
	// Ie, there's no need to define LFUN_BUFFERPREAMBLE_APPLY.
	Buffer & buffer = kernel().buffer();
	buffer.params().preamble = params();
	buffer.markDirty();
}


string const & ControlPreamble::params() const
{
	return params_;
}


void ControlPreamble::params(string const & newparams)
{
	params_ = newparams;
}
