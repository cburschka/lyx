/**
 * \file Systemcall.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Peter Kümmel
 *
 * Interface cleaned up by
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/Systemcall.h"
#include "support/SystemcallPrivate.h"
#include "support/ProgressInterface.h"

namespace lyx {
namespace support {

static ProgressInterface* progress_impl = 0;


void Systemcall::registerProgressInterface(ProgressInterface* p)
{
	progress_impl = p;
}


ProgressInterface* Systemcall::progress()
{
	return progress_impl;
}


int Systemcall::startscript(Starttype how, std::string const & what)
{
	// TODO Reuse of instance?
	SystemcallPrivate* process = new SystemcallPrivate;
	if (how == Wait)
		return process->start(what, true);
	return process->start(what, false);
}


} // namespace support
} // namespace lyx
