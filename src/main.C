/**
 * \file main.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Lars Gullik Bjønnes
 * \author Jean Marc Lasgouttes
 */

#include <config.h>

#include "lyx_main.h"
#include "gettext.h"
#include "support/os.h"

#include <ios>

int main(int argc, char * argv[])
{
	std::ios::sync_with_stdio(false);

	os::init(&argc, &argv);

	// initialize for internationalized version *EK*
	locale_init();

	LyX lyx(argc, argv);
	return 0;
}
