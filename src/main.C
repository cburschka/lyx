/**
 * \file main.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "debug.h"
#include "lyx_main.h"
#include "gettext.h"
#include "support/os.h"

#ifdef HAVE_IOS
#include <ios>
#endif


namespace os = lyx::support::os;


int main(int argc, char * argv[])
{
#ifdef HAVE_IOS
	std::ios_base::sync_with_stdio(false);
#endif
	// To avoid ordering of global object problems with some
	// stdlibs we do the initialization here, but still as
	// early as possible.
	lyxerr.rdbuf(std::cerr.rdbuf());

	os::init(&argc, &argv);

	// initialize for internationalized version *EK*
	locale_init();

	LyX::exec(argc, argv);
	return 0;
}
