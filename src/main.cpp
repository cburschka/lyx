/**
 * \file main.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "LyX.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/os.h"

#include <iostream>
#ifdef HAVE_IOS
#include <ios>
#endif

using namespace std;


int main(int argc, char * argv[])
{
#ifdef HAVE_IOS
	ios_base::sync_with_stdio(false);
#endif

	// To avoid ordering of global object problems with some
	// stdlibs we do the initialization here, but still as
	// early as possible.
	lyx::lyxerr.setStream(cerr);

	lyx::support::os::init(argc, argv);

	// initialize for internationalized version *EK*
	lyx::locale_init();

	lyx::LyX the_lyx_instance;

	return the_lyx_instance.exec(argc, argv);
}
