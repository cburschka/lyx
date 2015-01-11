/**
 * \file ConsoleApplication.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/ConsoleApplication.h"

#include "support/ConsoleApplicationPrivate.h"


namespace lyx {

namespace support {


ConsoleApplication::~ConsoleApplication()
{
	delete d;
}


ConsoleApplication::ConsoleApplication(std::string const & app,
		int & argc, char ** argv)
	: d(new ConsoleApplicationPrivate(this, app, argc, argv))
{
}


int ConsoleApplication::exec()
{
	return d->execute();
}


void ConsoleApplication::exit(int status)
{
	d->exit(status);
}


} // namespace support
} // namespace lyx

#include "moc_ConsoleApplicationPrivate.cpp"
